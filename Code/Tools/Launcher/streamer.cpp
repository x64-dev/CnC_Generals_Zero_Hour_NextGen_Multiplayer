#include "streamer.h"
#ifdef _WIN32
#include <windows.h>
#endif

Streamer::Streamer()
    : std::streambuf(), Output_Device(nullptr), is_unbuffered_(false), buffer_(nullptr)
{
    // By default, we use buffered output (0 = buffered, 1 = unbuffered)
    unbuffered(0);
}

Streamer::~Streamer()
{
    sync();
    if (buffer_) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

int Streamer::setOutputDevice(OutputDevice* device)
{
    Output_Device = device;
    return 0;
}

// Write up to n characters from s into the buffer
std::streamsize Streamer::xsputn(const char* buf, std::streamsize size)
{
    if (size <= 0)
        return 0;

    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(buf);
    for (std::streamsize i = 0; i < size; i++, ptr++) {
        if (*ptr == '\n') {
            if (overflow(*ptr) == EOF)
                return i;
        }
        else if (sputc(*ptr) == EOF)
            return i;
    }
    return size;
}

// Flush the buffer and make room if needed.
int Streamer::overflow(int c)
{
    if (c == EOF)
        return sync();

    // If no put area is set, try to allocate one.
    if ((pbase() == nullptr) && (doallocate() == 0))
        return EOF;

    // If the put area is full and we cannot flush it, fail.
    if ((pptr() >= epptr()) && (sync() == EOF))
        return EOF;
    else {
        sputc(c);
        // If unbuffered mode with newline or if put area is now full, flush.
        if (((unbuffered() && (c == '\n')) || (pptr() >= epptr())) && sync() == EOF)
            return EOF;
        return c;
    }
}

// This is a write-only stream, so underflow should never be reached.
int Streamer::underflow(void)
{
    return EOF;
}

// Allocate the internal buffer if not already done.
int Streamer::doallocate()
{
    if (buffer_ == nullptr) {
        buffer_ = new char[2 * STREAMER_BUFSIZ];  // to be deleted in destructor
        memset(buffer_, 0, 2 * STREAMER_BUFSIZ);

        // Set the get area to empty (not used in a write-only stream)
        setg(buffer_, buffer_, buffer_);
        // Use the second half of the allocated block for output.
        setp(buffer_ + STREAMER_BUFSIZ, buffer_ + 2 * STREAMER_BUFSIZ);
        return 1;
    }
    else
        return 0;
}

// Flush the output buffer to the output device.
int Streamer::sync()
{
    if (pptr() <= pbase())
        return 0;

    int wlen = static_cast<int>(pptr() - pbase());

    if (Output_Device) {
        Output_Device->print(pbase(), wlen);
    }

    // Reset the put pointer: if unbuffered, empty the buffer completely.
    if (unbuffered())
        setp(pbase(), pbase());
    else
        setp(pbase(), pbase() + STREAMER_BUFSIZ);
    return 0;
}
