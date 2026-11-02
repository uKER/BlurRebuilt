// [Blur2PC] T259. x86 host for the user's unmodified AMAX client.
// Retail detour entry locations/prototypes recovered from AMAX DllMain and Blur 1.2.
// These are host-side adapter entry points, not a copy or patch of the retail exe.
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>
#include <string.h>
#include "BridgeProtocol.h"
#pragma comment(lib, "ws2_32.lib")
using namespace AmaxClientProtocol;

static bool s_loginAccepted;
static bool __fastcall LoginContinuation(void*, void*, const char*, const char*)
{
    // The DLL calls this only after its authentication succeeds. The x64 game
    // subsequently performs real DemonWare authentication through the DLL sockets.
    s_loginAccepted = true;
    return true;
}
static int __cdecl NativeCreate(bool blocking)
{
    SOCKET handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (handle == INVALID_SOCKET) return -1;
    u_long mode = blocking ? 0 : 1;
    if (ioctlsocket(handle, FIONBIO, &mode)) { closesocket(handle); return -1; }
    return (int)handle;
}
static bool __cdecl NativeClose(int handle) { return closesocket((SOCKET)handle) == 0; }
static const uintptr_t s_sites[] = {0xce9910, 0xd8c630, 0xd8c680, 0xd8c7a0, 0xd8c8d0, 0xd8ca00};

static void Entry(uintptr_t address, uintptr_t callback)
{
    unsigned char entry[] = {0x90,0x90,0x90,0x90,0x90,0xb8,0,0,0,0,0xff,0xe0};
    memcpy(entry + 6, &callback, sizeof(callback));
    memcpy((void*)address, entry, sizeof(entry));
}
static int LoadClientDll(const wchar_t* path)
{
    if ((uintptr_t)GetModuleHandleW(NULL) != 0x400000) return HostLayout;
    const uintptr_t regions[] = {0xce0000, 0xd80000, 0xe50000};
    for (unsigned i = 0; i < 3; ++i)
        if (VirtualAlloc((void*)regions[i], 65536, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)
            != (void*)regions[i]) return HostLayout;
    const unsigned char unused[] = {0x8b,0xff,0x55,0x8b,0xec,0x33,0xc0,0x5d,0xc3,0x90,0x90,0x90};
    for (unsigned i = 0; i < 6; ++i) memcpy((void*)s_sites[i], unused, sizeof(unused));
    Entry(s_sites[0], (uintptr_t)&LoginContinuation);
    Entry(s_sites[1], (uintptr_t)&NativeCreate);
    Entry(s_sites[5], (uintptr_t)&NativeClose);
    *(FARPROC*)0xe5362c = GetProcAddress(GetModuleHandleW(L"ws2_32.dll"), "gethostbyname");
    unsigned char before[6][12];
    for (unsigned i = 0; i < 6; ++i) memcpy(before[i], (void*)s_sites[i], sizeof(before[i]));
    DWORD previous;
    for (unsigned i = 0; i < 2; ++i) {
        if (!VirtualProtect((void*)regions[i], 65536, PAGE_EXECUTE_READ, &previous)) return HostLayout;
        FlushInstructionCache(GetCurrentProcess(), (void*)regions[i], 65536);
    }
    HMODULE client = LoadLibraryExW(path, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!client) return LoadClient;
    for (unsigned i = 0; i < 6; ++i)
        if (!memcmp(before[i], (void*)s_sites[i], sizeof(before[i]))) return HooksMissing;
    FARPROC dns = GetProcAddress(client, "fake_gethostbyname");
    if (!dns || *(FARPROC*)0xe5362c != dns) return HooksMissing;
    return Accepted;
}
static bool Transfer(HANDLE pipe, void* data, DWORD bytes, bool write)
{
    unsigned char* cursor = (unsigned char*)data;
    while (bytes) {
        DWORD done = 0;
        BOOL ok = write ? WriteFile(pipe, cursor, bytes, &done, NULL) : ReadFile(pipe, cursor, bytes, &done, NULL);
        if (!ok || !done) return false;
        bytes -= done; cursor += done;
    }
    return true;
}
int wmain(int argc, wchar_t** argv)
{
    if (argc != 3) return 2; // pipe name, absolute installed DLL path; no credentials in arguments.
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    HANDLE pipe = CreateFileW(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE) return 3;
    Login login = {};
    if (!Transfer(pipe, &login, sizeof(login), false)) return 4;
    int result = BadProtocol;
    if (login.magic == Magic && login.username[0] && login.password[0]
        && !login.username[MaxCredential] && !login.password[MaxCredential]) {
        WSADATA ws;
        result = WSAStartup(MAKEWORD(2,2), &ws) ? HostLayout : LoadClientDll(argv[2]);
        if (result == Accepted) {
            typedef bool (__thiscall *Authenticate)(void*, const char*, const char*);
            int opaqueHost = 0;
            if (!((Authenticate)s_sites[0])(&opaqueHost, login.username, login.password) || !s_loginAccepted)
                result = LoginRejected;
        }
    }
    SecureZeroMemory(&login, sizeof(login));
    if (!Transfer(pipe, &result, sizeof(result), true) || result != Accepted) ExitProcess(5);
    // The official client currently has one stream at a time (auth, then lobby).
    // Reject overlap instead of letting a second create replace its live stream.
    int active = -1;
    Command command;
    unsigned char data[MaxTransfer];
    struct RetailAddress { uint32_t words[4]; }; // retail connect ABI; DLL ignores destination.
    while (Transfer(pipe, &command, sizeof(command), false)) {
        if (command.size > MaxTransfer) break;
        result = -10;
        if (command.operation == Send && !Transfer(pipe, data, command.size, false)) break;
        if (command.operation == Create) {
            if (active == -1) active = ((int(__cdecl*)(bool))s_sites[1])(command.size != 0);
            else break;
            result = active;
        } else if (command.handle == active && active != -1) {
            switch (command.operation) {
            case Connect: { RetailAddress address = {};
                result = ((int(__cdecl*)(int,RetailAddress,unsigned short))s_sites[2])(active,address,3074); break; }
            case Send: result = ((int(__cdecl*)(int,const void*,unsigned))s_sites[3])(active,data,command.size); break;
            case Receive: result = ((int(__cdecl*)(int,void*,unsigned))s_sites[4])(active,data,command.size); break;
            case Close: result = ((bool(__cdecl*)(int))s_sites[5])(active) ? 1 : 0; active = -1; break;
            default: ExitProcess(6);
            }
        }
        if (!Transfer(pipe, &result, sizeof(result), true)) break;
        if (command.operation == Receive && result > 0) {
            if ((unsigned)result > command.size || !Transfer(pipe, data, result, true)) break;
        }
    }
    // Process lifetime owns the DLL's background authentication/session threads.
    ExitProcess(0);
}
