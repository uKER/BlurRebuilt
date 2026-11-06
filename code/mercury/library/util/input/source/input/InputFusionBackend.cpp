// [Blur2PC] T272: preserve InputFusion's existing SDL controller implementation.
// Its worker thread owns the HID handles, so retain the module until process exit.
#include "stdafx.h"
#include <input/InputFusionBackend.h>
#include <stdio.h>
#include <wchar.h>

namespace Input
{
namespace
{
typedef DWORD (WINAPI* ReadStateFunction)(DWORD, XINPUT_STATE*);
typedef DWORD (WINAPI* WriteRumbleFunction)(DWORD, const XINPUT_VIBRATION*);
HMODULE backendModule = NULL;
ReadStateFunction readState = NULL;
WriteRumbleFunction writeRumble = NULL;

bool ExecutableSibling(const wchar_t* name, wchar_t (&path)[MAX_PATH])
{
    const DWORD length = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;
    wchar_t* separator = wcsrchr(path, L'\\');
    if (!separator) return false;
    separator[1] = L'\0';
    return wcscat_s(path, name) == 0;
}

void RecordBackend(const char* message, DWORD error)
{
    wchar_t path[MAX_PATH];
    if (!ExecutableSibling(L"input-backend.log", path)) return;
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    char line[256];
    const int length = sprintf_s(line, "pid=%lu %s error=%lu\r\n",
        GetCurrentProcessId(), message, error);
    DWORD written = 0;
    if (length > 0) WriteFile(file, line, static_cast<DWORD>(length), &written, NULL);
    CloseHandle(file);
}

BOOL InitialiseBackend()
{
#if !defined(BLUR_WITH_INPUTFUSION)
    RecordBackend("system XInput selected (InputFusion disabled at build time)", ERROR_SUCCESS);
    return TRUE;
#else
    char selection[32] = {};
    const DWORD selectionLength = GetEnvironmentVariableA("BLUR_INPUT_BACKEND", selection, sizeof(selection));
    if (selectionLength < sizeof(selection) && _stricmp(selection, "system") == 0)
    {
        RecordBackend("system XInput explicitly selected", ERROR_SUCCESS);
        return TRUE;
    }

    wchar_t path[MAX_PATH];
    if (!ExecutableSibling(L"BlurInputFusion.dll", path))
    {
        RecordBackend("cannot construct InputFusion path; using system XInput", ERROR_BAD_PATHNAME);
        return TRUE;
    }
    backendModule = LoadLibraryExW(path, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!backendModule)
    {
        RecordBackend("InputFusion load failed; using system XInput", GetLastError());
        return TRUE;
    }
    readState = reinterpret_cast<ReadStateFunction>(GetProcAddress(backendModule, "XInputGetState"));
    writeRumble = reinterpret_cast<WriteRumbleFunction>(GetProcAddress(backendModule, "XInputSetState"));
    if (!readState || !writeRumble)
    {
        readState = NULL;
        writeRumble = NULL;
        // Do not unload a DLL whose SDL worker may already be running.
        RecordBackend("InputFusion exports missing; using system XInput", ERROR_PROC_NOT_FOUND);
        return TRUE;
    }
    RecordBackend("InputFusion 0.4.1 selected from executable folder", ERROR_SUCCESS);
    return TRUE;
#endif
}

void EnsureBackend()
{
    // The legacy engine headers target older Windows APIs; C++14's local-static
    // guard provides thread-safe one-time initialization without changing them.
    static const BOOL initialized = InitialiseBackend();
    (void)initialized;
}
}

DWORD ReadPadState(DWORD userIndex, XINPUT_STATE* state)
{
    EnsureBackend();
    return readState ? readState(userIndex, state) : XInputGetState(userIndex, state);
}

DWORD WritePadRumble(DWORD userIndex, XINPUT_VIBRATION* vibration)
{
    EnsureBackend();
    return writeRumble ? writeRumble(userIndex, vibration) : XInputSetState(userIndex, vibration);
}
}
