/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define IG_DEBUG_STACKTRACE
#if defined(_DEBUG) || defined(_INTERNAL) || defined(IG_DEBUG_STACKTRACE)

#pragma pack(push, 8)

#pragma comment(linker, "/defaultlib:Dbghelp.lib")

#include "Common/StackDump.h"
#include "Common/Debug.h"


//*****************************************************************************
//	Prototypes
//*****************************************************************************
BOOL InitSymbolInfo(void);
void UninitSymbolInfo(void);
#ifdef _WIN64
void MakeStackTrace(DWORD64 myeip, DWORD64 myesp, DWORD64 myebp, int skipFrames, void (*callback)(const char*));
void GetFunctionDetails(void* pointer, char* name, char* filename, unsigned int* linenumber, unsigned int* address);
void WriteStackLine(void* address, void (*callback)(const char*));
#else
void MakeStackTrace(DWORD myeip,DWORD myesp,DWORD myebp, int skipFrames, void (*callback)(const char*));
void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address);
void WriteStackLine(void*address, void (*callback)(const char*));
#endif

//*****************************************************************************
//	Mis-named globals :-)
//*****************************************************************************
static CONTEXT gsContext;
static Bool gsInit=FALSE;

#ifdef _WIN64
BOOL(__stdcall* gsSymGetLineFromAddr)(
		IN  HANDLE                  hProcess,
		IN  DWORD64                 dwAddr,
		OUT PDWORD64                pdwDisplacement,
		OUT PIMAGEHLP_LINE64        Line
		);
#else
BOOL (__stdcall *gsSymGetLineFromAddr)(
		IN  HANDLE                  hProcess,
		IN  DWORD                   dwAddr,
		OUT PDWORD                  pdwDisplacement,
		OUT PIMAGEHLP_LINE          Line
			);
#endif

//*****************************************************************************
//*****************************************************************************
void StackDumpDefaultHandler(const char*line)
{
	DEBUG_LOG((line));
}


//*****************************************************************************
//*****************************************************************************
void StackDump(void (*callback)(const char*))
{
	if (callback == NULL) 
	{
		callback = StackDumpDefaultHandler;
	}

	InitSymbolInfo();

#ifdef _WIN64
	DWORD64 myeip, myesp, myebp;
	// Use CaptureStackBackTrace for x64.  Inline assembly isn't allowed.
	// We'll get the context from within MakeStackTrace.
	myeip = 0;
	myesp = 0;
	myebp = 0;
#else
	DWORD myeip,myesp,myebp;

_asm
{
MYEIP1:
 mov eax, MYEIP1
 mov dword ptr [myeip] , eax
 mov eax, esp
 mov dword ptr [myesp] , eax
 mov eax, ebp
 mov dword ptr [myebp] , eax
}
#endif


	MakeStackTrace(myeip,myesp,myebp, 2, callback);
}


//*****************************************************************************
//*****************************************************************************
#ifdef _WIN64
void StackDumpFromContext(DWORD64 eip, DWORD64 esp, DWORD64 ebp, void (*callback)(const char*))
#else
void StackDumpFromContext(DWORD eip, DWORD esp, DWORD ebp, void (*callback)(const char*))
#endif
{
	if (callback == NULL) 
	{
		callback = StackDumpDefaultHandler;
	}

	InitSymbolInfo();

	MakeStackTrace(eip,esp,ebp, 0,  callback);
}


//*****************************************************************************
//*****************************************************************************
BOOL InitSymbolInfo()
{
	if (gsInit == TRUE) 
		return TRUE;

	gsInit = TRUE;

	atexit(UninitSymbolInfo);

	// See if we have the line from address function
	// We use GetProcAddress to stop link failures at dll loadup
	HINSTANCE hInstDebugHlp = GetModuleHandle("dbghelp.dll");

#ifdef _WIN64
	gsSymGetLineFromAddr = (BOOL(__stdcall*)(IN HANDLE, IN DWORD64, OUT PDWORD64, OUT PIMAGEHLP_LINE64))
		GetProcAddress(hInstDebugHlp, "SymGetLineFromAddr64");
#else
	gsSymGetLineFromAddr = (BOOL(__stdcall*)(IN HANDLE, IN DWORD, OUT PDWORD, OUT PIMAGEHLP_LINE))
		GetProcAddress(hInstDebugHlp, "SymGetLineFromAddr");
#endif

	char pathname[_MAX_PATH+1];
	char drive[10];
	char directory[_MAX_PATH+1];
	HANDLE process;


	::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST);

	process = GetCurrentProcess();

	//Get the apps name
	::GetModuleFileName(NULL, pathname, _MAX_PATH);

	// turn it into a search path
	_splitpath(pathname, drive, directory, NULL, NULL);
	sprintf(pathname, "%s:\\%s", drive, directory);

	// append the current directory to build a search path for SymInit
	::lstrcat(pathname, ";.;");

	if(::SymInitialize(process, pathname, FALSE))
	{
		// regenerate the name of the app
		::GetModuleFileName(NULL, pathname, _MAX_PATH);
#ifdef _WIN64
		if (::SymLoadModule64(process, NULL, pathname, NULL, 0, 0)) // Use SymLoadModule64
#else
		if (::SymLoadModule(process, NULL, pathname, NULL, 0, 0))
#endif
		{
				//Load any other relevant modules (ie dlls) here
				return TRUE;
		}
		::SymCleanup(process);
	}

	return(FALSE);
}


//*****************************************************************************
//*****************************************************************************
void UninitSymbolInfo(void)
{
	if (gsInit == FALSE)
	{
		return;
	}

	gsInit = FALSE;

	::SymCleanup(GetCurrentProcess());
}



//*****************************************************************************
//*****************************************************************************
#ifdef _WIN64
void MakeStackTrace(DWORD64 myeip, DWORD64 myesp, DWORD64 myebp, int skipFrames, void (*callback)(const char*))
{
	STACKFRAME64      stack_frame;
	BOOL            b_ret = TRUE;

	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();

	memset(&gsContext, 0, sizeof(CONTEXT));
	gsContext.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&gsContext);

	memset(&stack_frame, 0, sizeof(STACKFRAME64));
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = gsContext.Rip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = gsContext.Rsp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = gsContext.Rbp;
	{
		callback("Call Stack\n**********\n");

		// Skip some ?
		unsigned int skip = skipFrames;
		DWORD machine_type = IMAGE_FILE_MACHINE_AMD64;
		while (b_ret && skip)
		{
			b_ret = StackWalk64(machine_type,
				process,
				thread,
				&stack_frame,
				&gsContext,
				NULL,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				NULL);
			skip--;
		}

		skip = 30;
		while (b_ret && skip)
		{

			b_ret = StackWalk64(machine_type,
				process,
				thread,
				&stack_frame,
				&gsContext,
				NULL,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				NULL);



			if (b_ret) WriteStackLine((void*)stack_frame.AddrPC.Offset, callback);
			skip--;
		}
	}
}
#else
void MakeStackTrace(DWORD myeip,DWORD myesp,DWORD myebp, int skipFrames, void (*callback)(const char*))
{
STACKFRAME      stack_frame;
BOOL            b_ret = TRUE;

HANDLE thread = GetCurrentThread();
HANDLE process = GetCurrentProcess();

memset(&gsContext, 0, sizeof(CONTEXT));
gsContext.ContextFlags = CONTEXT_FULL;

memset(&stack_frame, 0, sizeof(STACKFRAME));
stack_frame.AddrPC.Mode = AddrModeFlat;
stack_frame.AddrPC.Offset = myeip;
stack_frame.AddrStack.Mode = AddrModeFlat;
stack_frame.AddrStack.Offset = myesp;
stack_frame.AddrFrame.Mode = AddrModeFlat;
stack_frame.AddrFrame.Offset = myebp;
{
/*
    if(GetThreadContext(thread, &gsContext))
    {
        memset(&stack_frame, 0, sizeof(STACKFRAME));
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrPC.Offset = gsContext.Eip;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = gsContext.Esp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = gsContext.Ebp;
*/

		//{
			callback("Call Stack\n**********\n");

			// Skip some ?
			unsigned int skip = skipFrames;
			while (b_ret&&skip)
			{
					b_ret = StackWalk(      IMAGE_FILE_MACHINE_I386,
											process,
											thread,
											&stack_frame,
											NULL, //&gsContext,
											NULL,
											SymFunctionTableAccess,
											SymGetModuleBase,
											NULL);
					skip--;
			}

			skip = 30;
			while(b_ret&&skip)
			{

					b_ret = StackWalk(      IMAGE_FILE_MACHINE_I386,
											process,
											thread,
											&stack_frame,
											NULL, //&gsContext,
											NULL,
											SymFunctionTableAccess,
											SymGetModuleBase,
											NULL);
					

					
					if (b_ret) WriteStackLine((void *) stack_frame.AddrPC.Offset, callback);
					skip--;
			}
	}
}
#endif


//*****************************************************************************
//*****************************************************************************
void GetFunctionDetails(void *pointer, char*name, char*filename, unsigned int* linenumber, unsigned int* address)
{
	InitSymbolInfo();
	if (name)
	{
		strcpy(name, "<Unknown>");
	}
	if (filename)
	{
		strcpy(filename, "<Unknown>");
	}
	if (linenumber)
	{
		*linenumber = 0xFFFFFFFF;
	}
	if (address)
	{
		*address = 0xFFFFFFFF;
	}

#ifdef _WIN64
	DWORD64 displacement = 0;
#else
	ULONG displacement = 0;
#endif

    HANDLE process = ::GetCurrentProcess();

#ifdef _WIN64
	char symbol_buffer[512 + sizeof(IMAGEHLP_SYMBOL64)];
	memset(symbol_buffer, 0, sizeof(symbol_buffer));

	PIMAGEHLP_SYMBOL64 psymbol = (PIMAGEHLP_SYMBOL64)symbol_buffer;
	psymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	psymbol->MaxNameLength = 512;

	if (SymGetSymFromAddr64(process, (DWORD64)pointer, &displacement, psymbol))
#else
    char symbol_buffer[512 + sizeof(IMAGEHLP_SYMBOL)];
    memset(symbol_buffer, 0, sizeof(symbol_buffer));

    PIMAGEHLP_SYMBOL psymbol = (PIMAGEHLP_SYMBOL)symbol_buffer;
    psymbol->SizeOfStruct = sizeof(symbol_buffer);
    psymbol->MaxNameLength = 512;

    if (SymGetSymFromAddr(process, (DWORD) pointer, &displacement, psymbol))
#endif
    {
		if (name)
		{
			strcpy(name, psymbol->Name);
			strcat(name, "();");
		}

		// Get line now
		if (gsSymGetLineFromAddr)
		{
			// Unsupported for win95/98 at least with my current dbghelp.dll

#ifdef _WIN64
			IMAGEHLP_LINE64 line; // Use IMAGEHLP_LINE64
			memset(&line, 0, sizeof(line));
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);


			if (gsSymGetLineFromAddr(process, (DWORD64)pointer, &displacement, &line)) // Use DWORD64
#else
			IMAGEHLP_LINE line;
			memset(&line,0,sizeof(line));
			line.SizeOfStruct = sizeof(line);

		
			if (gsSymGetLineFromAddr(process, (DWORD) pointer, &displacement, &line))
#endif
			{
				if (filename)
				{
					strcpy(filename, line.FileName);
				}
				if (linenumber)
				{
					*linenumber = (unsigned int)line.LineNumber;
				}
				if (address)
				{
					*address = (unsigned int)line.Address;
				}
			} 					
		}
    }
}


//*****************************************************************************
// Gets last x addresses from the stack
//*****************************************************************************
#ifdef _WIN64
void FillStackAddresses(void** addresses, unsigned int count, unsigned int skip)
{
	InitSymbolInfo();

	STACKFRAME64	stack_frame;


	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();

	memset(&gsContext, 0, sizeof(CONTEXT));
	gsContext.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&gsContext);

	memset(&stack_frame, 0, sizeof(STACKFRAME64));
	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = gsContext.Rip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = gsContext.Rsp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = gsContext.Rbp;

	{
		Bool stillgoing = TRUE;
		//	unsigned int cd = count;
		DWORD machine_type = IMAGE_FILE_MACHINE_AMD64;
		// Skip some?
		while (stillgoing && skip)
		{
			stillgoing = StackWalk64(machine_type,
				process,
				thread,
				&stack_frame,
				&gsContext,
				NULL,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				NULL) != 0;
			skip--;
		}

		while (stillgoing && count)
		{
			stillgoing = StackWalk64(machine_type,
				process,
				thread,
				&stack_frame,
				&gsContext,
				NULL,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				NULL) != 0;
			if (stillgoing)
			{
				*addresses = (void*)stack_frame.AddrPC.Offset;
				addresses++;
				count--;
			}
		}

		// Fill remainder
		while (count)
		{
			*addresses = NULL;
			addresses++;
			count--;
		}

	}
}
#else
void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip)
{
	InitSymbolInfo();

	STACKFRAME	stack_frame;

	
	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();

    memset(&gsContext, 0, sizeof(CONTEXT));
    gsContext.ContextFlags = CONTEXT_FULL;

	DWORD myeip,myesp,myebp;
_asm
{
MYEIP2:
 mov eax, MYEIP2
 mov dword ptr [myeip] , eax
 mov eax, esp
 mov dword ptr [myesp] , eax
 mov eax, ebp
 mov dword ptr [myebp] , eax
 xor eax,eax
}
memset(&stack_frame, 0, sizeof(STACKFRAME));
stack_frame.AddrPC.Mode = AddrModeFlat;
stack_frame.AddrPC.Offset = myeip;
stack_frame.AddrStack.Mode = AddrModeFlat;
stack_frame.AddrStack.Offset = myesp;
stack_frame.AddrFrame.Mode = AddrModeFlat;
stack_frame.AddrFrame.Offset = myebp;

{
/*
    if(GetThreadContext(thread, &gsContext))
    {
        memset(&stack_frame, 0, sizeof(STACKFRAME));
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrPC.Offset = gsContext.Eip;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = gsContext.Esp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = gsContext.Ebp;
*/

		Bool stillgoing = TRUE;
//	unsigned int cd = count;

		// Skip some?
		while (stillgoing&&skip)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL,	//&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			skip--;
		}

		while(stillgoing&&count)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL, //&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			if (stillgoing)
			{
				*addresses  = (void*)stack_frame.AddrPC.Offset;				
				addresses++;
				count--;
			}
		}

		// Fill remainder
		while (count)
		{
			*addresses = NULL;
			addresses++;
			count--;
		}

	}
/*
	else
	{
		memset(addresses,NULL,count*sizeof(void*));
	}
*/	
}
#endif



//*****************************************************************************
// Do full stack dump using an address array
//*****************************************************************************
void StackDumpFromAddresses(void**addresses, unsigned int count, void (*callback)(const char *))
{
	if (callback == NULL) 
	{
		callback = StackDumpDefaultHandler;
	}

	InitSymbolInfo();

	while ((count--) && (*addresses!=NULL))
	{
		WriteStackLine(*addresses,callback);	
		addresses++;
	}	
}


AsciiString g_LastErrorDump;
//*****************************************************************************
//*****************************************************************************
void WriteStackLine(void*address, void (*callback)(const char*))
{
	static char line[MAX_PATH];
	static char function_name[512];
	static char filename[MAX_PATH];
	unsigned int linenumber;
	unsigned int addr;

	GetFunctionDetails(address, function_name, filename, &linenumber, &addr);
#ifdef _WIN64
	sprintf(line, "  %s(%d) : %s 0x%016llx", filename, linenumber, function_name, (unsigned long long)address);
#else
	sprintf(line, "  %s(%d) : %s 0x%08p", filename, linenumber, function_name, address);
#endif
		if (g_LastErrorDump.isNotEmpty()) {
			g_LastErrorDump.concat(line);
			g_LastErrorDump.concat("\n");
		}
	callback(line);
	callback("\n");
}

//*****************************************************************************
//*****************************************************************************
void DumpExceptionInfo( unsigned int u, EXCEPTION_POINTERS* e_info )
{
   DEBUG_LOG(( "\n********** EXCEPTION DUMP ****************\n" ));
	/*
	** List of possible exceptions
	*/
	 g_LastErrorDump.clear(); 

	static const unsigned int _codes[] = {
		EXCEPTION_ACCESS_VIOLATION,
		EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
		EXCEPTION_BREAKPOINT,
		EXCEPTION_DATATYPE_MISALIGNMENT,
		EXCEPTION_FLT_DENORMAL_OPERAND,
		EXCEPTION_FLT_DIVIDE_BY_ZERO,
		EXCEPTION_FLT_INEXACT_RESULT,
		EXCEPTION_FLT_INVALID_OPERATION,
		EXCEPTION_FLT_OVERFLOW,
		EXCEPTION_FLT_STACK_CHECK,
		EXCEPTION_FLT_UNDERFLOW,
		EXCEPTION_ILLEGAL_INSTRUCTION,
		EXCEPTION_IN_PAGE_ERROR,
		EXCEPTION_INT_DIVIDE_BY_ZERO,
		EXCEPTION_INT_OVERFLOW,
		EXCEPTION_INVALID_DISPOSITION,
		EXCEPTION_NONCONTINUABLE_EXCEPTION,
		EXCEPTION_PRIV_INSTRUCTION,
		EXCEPTION_SINGLE_STEP,
		EXCEPTION_STACK_OVERFLOW,
		0xffffffff
	};

	/*
	** Information about each exception type.
	*/
	static char const * _code_txt[] = {
		"Error code: EXCEPTION_ACCESS_VIOLATION\nDescription: The thread tried to read from or write to a virtual address for which it does not have the appropriate access.",
		"Error code: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\nDescription: The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.",
		"Error code: EXCEPTION_BREAKPOINT\nDescription: A breakpoint was encountered.",
		"Error code: EXCEPTION_DATATYPE_MISALIGNMENT\nDescription: The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.",
		"Error code: EXCEPTION_FLT_DENORMAL_OPERAND\nDescription: One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.",
		"Error code: EXCEPTION_FLT_DIVIDE_BY_ZERO\nDescription: The thread tried to divide a floating-point value by a floating-point divisor of zero.",
		"Error code: EXCEPTION_FLT_INEXACT_RESULT\nDescription: The result of a floating-point operation cannot be represented exactly as a decimal fraction.",
		"Error code: EXCEPTION_FLT_INVALID_OPERATION\nDescription: Some strange unknown floating point operation was attempted.",
		"Error code: EXCEPTION_FLT_OVERFLOW\nDescription: The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_FLT_STACK_CHECK\nDescription: The stack overflowed or underflowed as the result of a floating-point operation.",
		"Error code: EXCEPTION_FLT_UNDERFLOW\nDescription:	The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.",
		"Error code: EXCEPTION_ILLEGAL_INSTRUCTION\nDescription:	The thread tried to execute an invalid instruction.",
		"Error code: EXCEPTION_IN_PAGE_ERROR\nDescription:	The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.",
		"Error code: EXCEPTION_INT_DIVIDE_BY_ZERO\nDescription: The thread tried to divide an integer value by an integer divisor of zero.",
		"Error code: EXCEPTION_INT_OVERFLOW\nDescription: The result of an integer operation caused a carry out of the most significant bit of the result.",
		"Error code: EXCEPTION_INVALID_DISPOSITION\nDescription: An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.",
		"Error code: EXCEPTION_NONCONTINUABLE_EXCEPTION\nDescription: The thread tried to continue execution after a noncontinuable exception occurred.",
		"Error code: EXCEPTION_PRIV_INSTRUCTION\nDescription: The thread tried to execute an instruction whose operation is not allowed in the current machine mode.",
		"Error code: EXCEPTION_SINGLE_STEP\nDescription: A trace trap or other single-instruction mechanism signaled that one instruction has been executed.",
		"Error code: EXCEPTION_STACK_OVERFLOW\nDescription: The thread used up its stack.",
		"Error code: ?????\nDescription: Unknown exception."
	};

	DEBUG_LOG( ("Dump exception info\n") );
	CONTEXT *context = e_info->ContextRecord;
	/*
	** The following are set for access violation only
	*/
	int access_read_write=-1;
	unsigned long access_address = 0;
	AsciiString msg; 

// DOUBLE_DEBUG does a DEBUG_LOG, and concats to g_LastErrorDump.  jba.
#define DOUBLE_DEBUG(x) { msg.format x; g_LastErrorDump.concat(msg); DEBUG_LOG( x ); }

	if ( e_info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	{
		DOUBLE_DEBUG (("Exception is access violation\n"));
		access_read_write = e_info->ExceptionRecord->ExceptionInformation[0];  // 0=read, 1=write
		access_address = e_info->ExceptionRecord->ExceptionInformation[1];
	}
	else
	{
		DOUBLE_DEBUG (("Exception code is %x\n", e_info->ExceptionRecord->ExceptionCode));
	}
#ifdef _WIN64
	ULONG_PTR* winMainAddr = (ULONG_PTR*)WinMain;
#else
	Int* winMainAddr = (Int*)WinMain;
#endif
	DOUBLE_DEBUG(("WinMain at %x\n", winMainAddr));
	/*
	** Match the exception type with the error string and print it out
	*/
	int i = 0;
	for ( i=0 ; _codes[i] != 0xffffffff ; i++ )
	{
		if ( _codes[i] == e_info->ExceptionRecord->ExceptionCode )
		{
			DEBUG_LOG ( ("Found exception description\n") );
			break;
		}
	}
	DOUBLE_DEBUG( ("%s\n", _code_txt[i]));
	/** For access violations, print out the violation address and if it was read or write.
	*/
	if ( e_info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION )
	{
		if ( access_read_write )
		{
			DOUBLE_DEBUG( ("Access address:%08X was written to.\n", access_address));
		}
		else
		{
			DOUBLE_DEBUG( ("Access address:%08X was read from.\n", access_address));
		}
	}

	DOUBLE_DEBUG (("\nStack Dump:\n"));
#ifdef _WIN64
    StackDumpFromContext(context->Rip, context->Rsp, context->Rbp, NULL);
#else
    StackDumpFromContext(context->Eip, context->Esp, context->Ebp, NULL);
#endif

	DOUBLE_DEBUG (("\nDetails:\n"));

	DOUBLE_DEBUG (("Register dump...\n"));

	/*
	** Dump the registers.
	*/
#ifdef _WIN64
    DOUBLE_DEBUG(("Rip:%016llX\tRsp:%016llX\tRbp:%016llX\n", context->Rip, context->Rsp, context->Rbp));
    DOUBLE_DEBUG(("Rax:%016llX\tRbx:%016llX\tRcx:%016llX\n", context->Rax, context->Rbx, context->Rcx));
    DOUBLE_DEBUG(("Rdx:%016llX\tRsi:%016llX\tRdi:%016llX\n", context->Rdx, context->Rsi, context->Rdi));
    DOUBLE_DEBUG(("R8 :%016llX\tR9 :%016llX\tR10:%016llX\n", context->R8, context->R9, context->R10));
    DOUBLE_DEBUG(("R11:%016llX\tR12:%016llX\tR13:%016llX\n", context->R11, context->R12, context->R13));
    DOUBLE_DEBUG(("R14:%016llX\tR15:%016llX\tEFlags:%08X\n", context->R14, context->R15, context->EFlags));
#else
    DOUBLE_DEBUG(("Eip:%08X\tEsp:%08X\tEbp:%08X\n", context->Eip, context->Esp, context->Ebp));
    DOUBLE_DEBUG(("Eax:%08X\tEbx:%08X\tEcx:%08X\n", context->Eax, context->Ebx, context->Ecx));
    DOUBLE_DEBUG(("Edx:%08X\tEsi:%08X\tEdi:%08X\n", context->Edx, context->Esi, context->Edi));
    DOUBLE_DEBUG(("EFlags:%08X\n", context->EFlags));
#endif

	/*
	** Dump the bytes at EIP. This will make it easier to match the crash address with later versions of the game.
	*/
	char scrap[512];
	DOUBLE_DEBUG ( ("EIP bytes dump...\n"));
#ifdef _WIN64
	wsprintf(scrap, "\nBytes at CS:RIP (%p)  : ", (void*)context->Rip);
	unsigned char* ip_ptr = (unsigned char*)(context->Rip);
#else
	wsprintf(scrap, "\nBytes at CS:EIP (%08X)  : ", context->Eip);
	unsigned char* ip_ptr = (unsigned char*)(context->Eip);
#endif

	char bytestr[32];

	for (int c = 0 ; c < 32 ; c++)
	{
		if (IsBadReadPtr(ip_ptr, 1))
		{
			lstrcat (scrap, "?? ");
		}
		else
		{
			sprintf (bytestr, "%02X ", *ip_ptr);
			strcat (scrap, bytestr);
		}
		ip_ptr++;
	}

	strcat (scrap, "\n");
	DOUBLE_DEBUG ( ( (scrap)));
  DEBUG_LOG(( "********** END EXCEPTION DUMP ****************\n\n" ));
}																									 


#pragma pack(pop)

#endif

