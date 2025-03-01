#ifndef STREAMER_HEADER
#define STREAMER_HEADER

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <cstring>    // use <cstring> instead of <string.h>
#include "odevice.h"

#ifndef STREAMER_BUFSIZ
// This limits the number of characters that can be sent to a single 'print'
// call. If your debug message is bigger than this, it will get split over
// multiple 'print' calls. That's usually not a problem.
#define STREAMER_BUFSIZ 2048
#endif

using namespace std;

// Provide a streambuf interface for a class that can 'print'
class Streamer : public std::streambuf
{
public:
    Streamer();
    virtual ~Streamer();

    int setOutputDevice(OutputDevice* output_device);

protected:
    // Overridden virtual methods from std::streambuf:
    virtual std::streamsize xsputn(const char* s, std::streamsize n); // write n characters
    virtual int overflow(int c = EOF);   // flush buffer and make more room
    virtual int underflow(void);         // not used (write-only stream)
    virtual int sync();

    int doallocate();  // allocate a buffer if needed

    OutputDevice* Output_Device;

private:
    // Members to hold our buffer and buffering mode.
    bool is_unbuffered_;
    char* buffer_;

    // Inline functions to get/set the unbuffered flag.
    int unbuffered() const { return is_unbuffered_ ? 1 : 0; }
    void unbuffered(int flag) { is_unbuffered_ = (flag != 0); }
};

#endif // STREAMER_HEADER
