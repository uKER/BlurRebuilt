// crash_handler.cpp — [Blur2PC] unhandled-exception reporter for the port.
// The original build had no crash handling on PC; while the port is being
// brought up, every crash writes crash.txt (exception, faulting module+offset,
// symbolised stack with file/line from the PDB) and crash.dmp next to the exe.
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "dbghelp.lib")

namespace
{
	char g_exeDir[MAX_PATH];

	void ExeDir()
	{
		GetModuleFileNameA(NULL, g_exeDir, MAX_PATH);
		char* p = strrchr(g_exeDir, '\\');
		if (p) p[1] = 0;
	}

	LONG WINAPI OnUnhandled(EXCEPTION_POINTERS* ep)
	{
		char path[MAX_PATH];
		_snprintf_s(path, MAX_PATH, _TRUNCATE, "%scrash.txt", g_exeDir);
		FILE* f = fopen(path, "w");
		if (!f)
			return EXCEPTION_CONTINUE_SEARCH;

		const EXCEPTION_RECORD* er = ep->ExceptionRecord;
		fprintf(f, "exception 0x%08lX at 0x%p", er->ExceptionCode, er->ExceptionAddress);
		if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2)
			fprintf(f, " (%s address 0x%p)", er->ExceptionInformation[0] ? "writing" : "reading", (void*)er->ExceptionInformation[1]);
		fprintf(f, "\n");

		HMODULE mod = NULL;
		if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)er->ExceptionAddress, &mod))
		{
			char name[MAX_PATH];
			GetModuleFileNameA(mod, name, MAX_PATH);
			fprintf(f, "module %s + 0x%08lX\n", name, (unsigned long)((char*)er->ExceptionAddress - (char*)mod));
		}

		HANDLE proc = GetCurrentProcess();
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		SymInitialize(proc, g_exeDir, TRUE);

		CONTEXT ctx = *ep->ContextRecord;
		STACKFRAME64 sf;
		memset(&sf, 0, sizeof(sf));
#if defined(_WIN64) // [Blur2PC][x64]
		const DWORD machine = IMAGE_FILE_MACHINE_AMD64;
		sf.AddrPC.Offset = ctx.Rip;    sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = ctx.Rbp; sf.AddrFrame.Mode = AddrModeFlat;
		sf.AddrStack.Offset = ctx.Rsp; sf.AddrStack.Mode = AddrModeFlat;
#else
		const DWORD machine = IMAGE_FILE_MACHINE_I386;
		sf.AddrPC.Offset = ctx.Eip;    sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = ctx.Ebp; sf.AddrFrame.Mode = AddrModeFlat;
		sf.AddrStack.Offset = ctx.Esp; sf.AddrStack.Mode = AddrModeFlat;
#endif

		fprintf(f, "stack:\n");
		for (int i = 0; i < 64; ++i)
		{
			if (!StackWalk64(machine, proc, GetCurrentThread(), &sf, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
				break;
			if (sf.AddrPC.Offset == 0)
				break;
			char buf[sizeof(SYMBOL_INFO) + 512];
			SYMBOL_INFO* sym = (SYMBOL_INFO*)buf;
			sym->SizeOfStruct = sizeof(SYMBOL_INFO);
			sym->MaxNameLen = 512;
			DWORD64 disp = 0;
			const char* name = SymFromAddr(proc, sf.AddrPC.Offset, &disp, sym) ? sym->Name : "?";
			IMAGEHLP_LINE64 line;
			line.SizeOfStruct = sizeof(line);
			DWORD ldisp = 0;
			if (SymGetLineFromAddr64(proc, sf.AddrPC.Offset, &ldisp, &line))
				fprintf(f, "  %02d 0x%08llX %s + 0x%llX  [%s:%lu]\n", i, sf.AddrPC.Offset, name, disp, line.FileName, line.LineNumber);
			else
				fprintf(f, "  %02d 0x%08llX %s + 0x%llX\n", i, sf.AddrPC.Offset, name, disp);
		}
		fclose(f);

		_snprintf_s(path, MAX_PATH, _TRUNCATE, "%scrash.dmp", g_exeDir);
		HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION mei = { GetCurrentThreadId(), ep, FALSE };
			MiniDumpWriteDump(proc, GetCurrentProcessId(), h, (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory), &mei, NULL, NULL);
			CloseHandle(h);
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	struct Installer
	{
		Installer()
		{
			ExeDir();
			SetUnhandledExceptionFilter(OnUnhandled);
		}
	} g_installer;
}
