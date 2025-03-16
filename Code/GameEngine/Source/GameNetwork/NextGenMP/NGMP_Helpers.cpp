#include "GameNetwork/NextGenMP/NGMP_include.h"

void NetworkLog(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, 1024, fmt, args);
	buffer[1024 - 1] = 0;
	va_end(args);

	DevConsole.AddLog(buffer);

	OutputDebugString(buffer);
	OutputDebugString("\n");
}