#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/CRC.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/NextGenMP/Packets/NextGenTransport.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//--------------------------------------------------------------------------
// Packet-level encryption is an XOR operation, for speed reasons.  To get
// the max throughput, we only XOR whole 4-byte words, so the last bytes
// can be non-XOR'd.

// This assumes the buf is a multiple of 4 bytes.  Extra is not encrypted.
static inline void encryptBuf( unsigned char *buf, Int len )
{
	UnsignedInt mask = 0x0000Fade;

	UnsignedInt *uintPtr = (UnsignedInt *) (buf);

	for (int i=0 ; i<len/4 ; i++) {
		*uintPtr = (*uintPtr) ^ mask;
		*uintPtr = htonl(*uintPtr);
		uintPtr++;
		mask += 0x00000321; // just for fun
	}
}

// This assumes the buf is a multiple of 4 bytes.  Extra is not encrypted.
static inline void decryptBuf( unsigned char *buf, Int len )
{
	UnsignedInt mask = 0x0000Fade;

	UnsignedInt *uintPtr = (UnsignedInt *) (buf);

	for (int i=0 ; i<len/4 ; i++) {
		*uintPtr = htonl(*uintPtr);
		*uintPtr = (*uintPtr) ^ mask;
		uintPtr++;
		mask += 0x00000321; // just for fun
	}
}

NextGenTransport::NextGenTransport()
{

}

NextGenTransport::~NextGenTransport()
{
	reset();
}

Bool NextGenTransport::init(AsciiString ip, UnsignedShort port)
{
	return true;
}

Bool NextGenTransport::init(UnsignedInt ip, UnsignedShort port)
{
	return true;
}

void NextGenTransport::reset(void)
{

}

Bool NextGenTransport::update(void)
{
	return true;
}

Bool NextGenTransport::doRecv(void)
{
	return true;
}

Bool NextGenTransport::doSend(void)
{
	return true;
}

Bool NextGenTransport::queueSend(UnsignedInt addr, UnsignedShort port, const UnsignedByte* buf, Int len /*, NetMessageFlags flags, Int id */)
{
	return true;
}
