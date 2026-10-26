// stacksample.cpp - [Blur2PC] poor man's sampling profiler for the 32-bit game.
// Usage: stacksample <pid> [seconds=5] [interval_ms=5] [depth=12]
// Suspends every thread of the process at a fixed interval, walks each stack with the
// target's PDB (dbghelp), and prints the most frequent leaf functions and full stacks.
// Build (x86): cl /nologo /O2 /EHsc stacksample.cpp dbghelp.lib
#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "dbghelp.lib")

static std::string Sym(HANDLE proc, DWORD64 addr)
{
	char buf[sizeof(SYMBOL_INFO) + 256];
	SYMBOL_INFO* s = (SYMBOL_INFO*)buf;
	s->SizeOfStruct = sizeof(SYMBOL_INFO);
	s->MaxNameLen = 255;
	DWORD64 disp = 0;
	if (SymFromAddr(proc, addr, &disp, s))
		return s->Name;
	char tmp[32];
	sprintf_s(tmp, "0x%08llx", addr);
	return tmp;
}

int main(int argc, char** argv)
{
	if (argc < 2) { printf("usage: stacksample <pid> [seconds] [interval_ms] [depth]\n"); return 1; }
	DWORD pid = atoi(argv[1]);
	int seconds = argc > 2 ? atoi(argv[2]) : 5;
	int interval = argc > 3 ? atoi(argv[3]) : 5;
	int depth = argc > 4 ? atoi(argv[4]) : 12;
	bool includeIdle = argc > 5 && strcmp(argv[5], "all") == 0;

	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!proc) { printf("OpenProcess failed %lu\n", GetLastError()); return 1; }
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	if (!SymInitialize(proc, NULL, TRUE)) { printf("SymInitialize failed\n"); return 1; }

	std::map<std::string, int> leaf, stacks, anyframe;
	std::map<DWORD, int> threadSamples;
	int total = 0;
	DWORD end = GetTickCount() + seconds * 1000;
	while (GetTickCount() < end)
	{
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 te; te.dwSize = sizeof(te);
		for (BOOL ok = Thread32First(snap, &te); ok; ok = Thread32Next(snap, &te))
		{
			if (te.th32OwnerProcessID != pid) continue;
			HANDLE th = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			if (!th) continue;
			if (SuspendThread(th) != (DWORD)-1)
			{
				CONTEXT ctx; ctx.ContextFlags = CONTEXT_FULL;
				if (GetThreadContext(th, &ctx))
				{
					STACKFRAME64 sf; memset(&sf, 0, sizeof(sf));
					sf.AddrPC.Offset = ctx.Eip; sf.AddrPC.Mode = AddrModeFlat;
					sf.AddrFrame.Offset = ctx.Ebp; sf.AddrFrame.Mode = AddrModeFlat;
					sf.AddrStack.Offset = ctx.Esp; sf.AddrStack.Mode = AddrModeFlat;
					std::string st, first;
					for (int i = 0; i < depth; ++i)
					{
						if (!StackWalk64(IMAGE_FILE_MACHINE_I386, proc, th, &sf, &ctx, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) break;
						if (!sf.AddrPC.Offset) break;
						std::string n = Sym(proc, sf.AddrPC.Offset);
						if (i == 0) first = n;
						anyframe[n]++;
						st += (i ? " < " : "") + n;
					}
					// skip idle waits so the report shows work
					bool idle = first.find("NtWaitFor") != std::string::npos || first.find("ZwWaitFor") != std::string::npos ||
						first.find("NtDelayExecution") != std::string::npos || first.find("NtRemoveIoCompletion") != std::string::npos ||
						first.find("ZwDelayExecution") != std::string::npos || first.find("NtWaitForAlertByThreadId") != std::string::npos ||
						first.find("ZwRemoveIoCompletion") != std::string::npos || first.find("NtUserMsgWaitForMultipleObjectsEx") != std::string::npos;
					if (!idle || includeIdle)
					{
						leaf[first]++;
						stacks[st]++;
						threadSamples[te.th32ThreadID]++;
						total++;
					}
				}
				ResumeThread(th);
			}
			CloseHandle(th);
		}
		CloseHandle(snap);
		Sleep(interval);
	}

	auto top = [](std::map<std::string, int>& m, int n, const char* title)
	{
		std::vector<std::pair<int, std::string>> v;
		for (auto& kv : m) v.push_back(std::make_pair(kv.second, kv.first));
		std::sort(v.rbegin(), v.rend());
		printf("=== %s\n", title);
		for (int i = 0; i < (int)v.size() && i < n; ++i) printf("%6d  %s\n", v[i].first, v[i].second.c_str());
	};
	printf("busy samples: %d\n", total);
	for (auto& kv : threadSamples) printf("thread %lu: %d\n", kv.first, kv.second);
	top(leaf, 25, "busy leaf functions");
	top(stacks, 400, "busy stacks");
	return 0;
}
