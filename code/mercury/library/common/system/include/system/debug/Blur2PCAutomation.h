// [Blur2PC] Scripted automation for unattended test runs.
//
// Active only when the environment variable BLUR2PC_AUTOMATION names a script file. The script
// injects keyboard input into the game's own keyboard device, runs Lua in the UI's Lua state,
// waits on game state and requests back-buffer screenshots, so test runs need neither window focus
// nor desktop capture and do not disturb whoever is using the machine. While a script is active the
// game also ignores focus loss (no auto-pause).
//
// Script format: one command per line. Times are seconds since the start of the line's segment:
// the first segment starts at the first keyboard update (roughly: when the window opens), and each
// `wait` line starts a new segment for the lines below it, at the moment its condition came true.
// Lines are sorted by time within their segment; a `wait` starts after every line above it in its
// segment has run.
//     <time> tap  <KEY> [hold_ms]     press and release (default hold 150 ms)
//     <time> down <KEY>               press and keep held
//     <time> up   <KEY>               release
//     <time> shot <file.png>          save the next presented frame (path relative to the script)
//     <time> record <prefix> <frames> encode the next <frames> presented frames, at half size, into
//                                     <prefix>.mp4 (ffmpeg on PATH, or BLUR2PC_FFMPEG; frame rate
//                                     BLUR2PC_FRAMELIMIT, else 60) and write each frame's time to
//                                     <prefix>_times.txt. Frames are blitted on the GPU, read back two
//                                     frames later and piped to ffmpeg from a worker thread, so the game
//                                     keeps its pace. A `quit` waits for the file to be finished.
//     <time> quit                     exit the process (exit code 0)
//     <time> lua  <code>              run <code> in the UI Lua state (rest of the line, '#' is Lua)
//     <time> wait <timeout> <expr>    block until the Lua expression <expr> is true (tested once per
//                                     UI frame; rest of the line, '#' is Lua). On timeout: log it,
//                                     save timeout.png beside the script and exit with code 3.
// On other lines '#' starts a comment. A line that starts with '#' is a comment.
// KEY: RETURN ESCAPE SPACE TAB UP DOWN LEFT RIGHT RSHIFT LSHIFT RCONTROL LCONTROL PGUP PGDN INSERT DELETE, NUMPADPLUS NUMPADENTER NUMPAD0, A-Z, 0-9, F1-F12.
//
// Lua helpers the UI manager adds while a script is active (amax/library/Ui/UiManager.cpp):
//     Blur2PC.RaceState()   "None" "Loading" "Intro" "Starting" "Racing" "Finished" "Outro" ...
//                           (race logic layer state; "None" when there is no race)
//     Blur2PC.Tap(key)      press and release KEY (names as above) from Lua, e.g. from a `wait` condition
//                           that drives menus by the screen it sees; false while an earlier tap is held
//     Blur2PC.Log(text)     write a line to automation.log
//     Blur2PC.Run(file)     run a Lua file (absolute, or relative to the script); returns what the file returns
// plus everything the game's UI scripts see (UIScreen.GetCurrentScreen(), UIGlobals, Amax.*, ...).
// `lua` and `wait` code runs as the main UI context's screen code does: context 0 selected and that
// screen's GUI and SCUI bound. Write strings with backslashes as [[intro\startscreen.lua]].
// Screen changes in every UI context are logged to automation.log.
//
// Header-only on purpose: the input, display, UI and framework libraries all include it, and inline
// functions with function-local statics share one instance across translation units.
#ifndef SYSTEM_DEBUG_BLUR2PC_AUTOMATION_H
#define SYSTEM_DEBUG_BLUR2PC_AUTOMATION_H

#if defined(PLATFORM_PC)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

namespace Blur2PCAutomation
{
	enum E_Kind { E_Down, E_Up, E_Shot, E_Record, E_Quit, E_Lua, E_Wait };

	enum { k_MaxEvents = 512, k_MaxText = 768 };

	struct Event
	{
		int segment;
		double time;
		double timeout;
		E_Kind kind;
		int dik;
		char text[k_MaxText];	// screenshot path, Lua code or wait expression
	};

	struct State
	{
		bool loaded;
		bool active;
		Event events[k_MaxEvents];
		int count;
		int next;
		bool keys[256];
		char pendingShot[MAX_PATH];
		char pendingRecord[MAX_PATH];
		int pendingRecordFrames;
		char baseDir[MAX_PATH];
		LARGE_INTEGER start;
		LARGE_INTEGER freq;
		bool started;
		FILE* log;
		CRITICAL_SECTION lock;	// Update() runs in the input update, the Lua side in the UI update
		double segmentStart;
		// wait in progress
		bool waiting;
		volatile bool waitMet;
		double waitStart;
		const Event* waitEvent;
		// Lua code handed to the UI update
		const Event* pendingLua;
		// exit after a failed wait, once its screenshot had a chance to be written
		double failQuitAt;
		// parsing: latest time used so far in the current segment
		double parseSegmentLast;
		// key tap requested from Lua (Blur2PC.Tap): DIK code (0 = none), pressed yet, release time
		int luaTapDik;
		bool luaTapPressed;
		double luaTapUp;
		// a recording is being captured or encoded; a `quit` waits for it
		volatile LONG recordingBusy;
		bool quitPending;
	};

	inline State& Get()
	{
		static State s_state;
		return s_state;
	}

	inline int KeyFromName(const char* name)
	{
		struct Pair { const char* n; int dik; };
		static const Pair table[] = {
			{"RETURN",0x1C},{"ENTER",0x1C},{"ESCAPE",0x01},{"ESC",0x01},{"SPACE",0x39},{"TAB",0x0F},
			{"NUMPADPLUS",0x4E},{"NUMPADENTER",0x9C},{"NUMPAD0",0x52},
			{"UP",0xC8},{"DOWN",0xD0},{"LEFT",0xCB},{"RIGHT",0xCD},{"RSHIFT",0x36},{"LSHIFT",0x2A},{"RCONTROL",0x9D},{"LCONTROL",0x1D},{"PGUP",0xC9},{"PGDN",0xD1},{"INSERT",0xD2},{"DELETE",0xD3},
			{"Q",0x10},{"W",0x11},{"E",0x12},{"R",0x13},{"T",0x14},{"Y",0x15},{"U",0x16},{"I",0x17},{"O",0x18},{"P",0x19},
			{"A",0x1E},{"S",0x1F},{"D",0x20},{"F",0x21},{"G",0x22},{"H",0x23},{"J",0x24},{"K",0x25},{"L",0x26},
			{"Z",0x2C},{"X",0x2D},{"C",0x2E},{"V",0x2F},{"B",0x30},{"N",0x31},{"M",0x32},
			{"1",0x02},{"2",0x03},{"3",0x04},{"4",0x05},{"5",0x06},{"6",0x07},{"7",0x08},{"8",0x09},{"9",0x0A},{"0",0x0B},
			{"F1",0x3B},{"F2",0x3C},{"F3",0x3D},{"F4",0x3E},{"F5",0x3F},{"F6",0x40},{"F7",0x41},{"F8",0x42},
			{"F9",0x43},{"F10",0x44},{"F11",0x57},{"F12",0x58},
		};
		for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
			if (_stricmp(table[i].n, name) == 0)
				return table[i].dik;
		return -1;
	}

	//! Seconds since the script clock started (0 before the first keyboard update).
	inline double Now()
	{
		State& s = Get();
		if (!s.started)
			return 0.0;
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (double)(now.QuadPart - s.start.QuadPart) / (double)s.freq.QuadPart;
	}

	//! Writes "<time> <message>" to automation.log.
	inline void LogF(const char* fmt, ...)
	{
		State& s = Get();
		if (!s.log)
			return;
		char line[1024];
		va_list args;
		va_start(args, fmt);
		_vsnprintf_s(line, sizeof(line), _TRUNCATE, fmt, args);
		va_end(args);
		fprintf(s.log, "%8.2f %s\n", Now(), line);
		fflush(s.log);
	}

	inline Event* AddEvent(int segment, double time, E_Kind kind, int dik)
	{
		State& s = Get();
		if (time > s.parseSegmentLast)
			s.parseSegmentLast = time;
		if (s.count >= k_MaxEvents)
			return NULL;
		Event& e = s.events[s.count++];
		e.segment = segment; e.time = time; e.timeout = 0.0; e.kind = kind; e.dik = dik; e.text[0] = 0;
		return &e;
	}

	inline bool EventBefore(const Event& a, const Event& b)
	{
		return a.segment < b.segment || (a.segment == b.segment && a.time < b.time);
	}

	inline void SortEvents()
	{
		// stable insertion sort by (segment, time); scripts are short
		State& s = Get();
		for (int j = 1; j < s.count; ++j)
			for (int i = j; i > 0 && EventBefore(s.events[i], s.events[i - 1]); --i)
			{
				Event tmp = s.events[i]; s.events[i] = s.events[i - 1]; s.events[i - 1] = tmp;
			}
	}

	inline char* SkipSpaces(char* p) { while (*p == ' ' || *p == '\t') ++p; return p; }

	inline void TrimEnd(char* p)
	{
		size_t n = strlen(p);
		while (n > 0 && (p[n - 1] == '\n' || p[n - 1] == '\r' || p[n - 1] == ' ' || p[n - 1] == '\t'))
			p[--n] = 0;
	}

	inline void Load()
	{
		State& s = Get();
		if (s.loaded)
			return;
		s.loaded = true;
		const char* path = getenv("BLUR2PC_AUTOMATION");
		if (!path || !*path)
			return;
		FILE* f = fopen(path, "r");
		if (!f)
			return;
		InitializeCriticalSection(&s.lock);
		strncpy_s(s.baseDir, path, _TRUNCATE);
		char* slash = strrchr(s.baseDir, '\\');
		if (!slash) slash = strrchr(s.baseDir, '/');
		if (slash) slash[1] = 0; else s.baseDir[0] = 0;
		char logPath[MAX_PATH];
		_snprintf_s(logPath, MAX_PATH, _TRUNCATE, "%sautomation.log", s.baseDir);
		s.log = fopen(logPath, "w");

		int segment = 0;
		char line[2048];
		while (fgets(line, sizeof(line), f))
		{
			TrimEnd(line);
			char* p = SkipSpaces(line);
			if (*p == '#' || *p == 0)
				continue;
			char* end = NULL;
			const double t = strtod(p, &end);
			if (end == p)
				continue;
			p = SkipSpaces(end);
			char cmd[32] = {0};
			int n = 0;
			while (*p && *p != ' ' && *p != '\t' && n < 31) cmd[n++] = *p++;
			p = SkipSpaces(p);

			// lua and wait take the rest of the line verbatim
			if (_stricmp(cmd, "lua") == 0)
			{
				if (Event* e = AddEvent(segment, t, E_Lua, 0))
					strncpy_s(e->text, p, _TRUNCATE);
				continue;
			}
			if (_stricmp(cmd, "wait") == 0)
			{
				const double timeout = strtod(p, &end);
				if (end == p || timeout <= 0.0) { LogF("wait without a timeout: %s", line); continue; }
				// a wait starts after every line above it in its segment
				if (Event* e = AddEvent(segment, t > s.parseSegmentLast ? t : s.parseSegmentLast, E_Wait, 0))
				{
					e->timeout = timeout;
					strncpy_s(e->text, SkipSpaces(end), _TRUNCATE);
				}
				++segment;
				s.parseSegmentLast = 0.0;
				continue;
			}

			char* hash = strchr(p, '#');
			if (hash) *hash = 0;
			char arg[400] = {0}; int hold = 150;
			const int args = sscanf_s(p, "%399s %d", arg, (unsigned)sizeof(arg), &hold);
			if (_stricmp(cmd, "quit") == 0) { AddEvent(segment, t, E_Quit, 0); continue; }
			if (args < 1)
				continue;
			if (_stricmp(cmd, "record") == 0)
			{
				if (Event* e = AddEvent(segment, t, E_Record, args >= 2 ? hold : 60))
					_snprintf_s(e->text, k_MaxText, _TRUNCATE, "%s%s", s.baseDir, arg);
				continue;
			}
			if (_stricmp(cmd, "shot") == 0)
			{
				if (Event* e = AddEvent(segment, t, E_Shot, 0))
					_snprintf_s(e->text, k_MaxText, _TRUNCATE, "%s%s", s.baseDir, arg);
				continue;
			}
			const int dik = KeyFromName(arg);
			if (dik < 0) { LogF("unknown key %s", arg); continue; }
			if (_stricmp(cmd, "tap") == 0) { AddEvent(segment, t, E_Down, dik); AddEvent(segment, t + (args >= 2 ? hold : 150) / 1000.0, E_Up, dik); }
			else if (_stricmp(cmd, "down") == 0) AddEvent(segment, t, E_Down, dik);
			else if (_stricmp(cmd, "up") == 0) AddEvent(segment, t, E_Up, dik);
			else LogF("unknown command %s", cmd);
		}
		fclose(f);
		SortEvents();
		s.active = true;
		LogF("script loaded: %s", path);
	}

	inline bool IsActive()
	{
		Load();
		return Get().active;
	}

	inline const char* KeyName(int dik)
	{
		static char name[8];
		_snprintf_s(name, sizeof(name), _TRUNCATE, "0x%02X", dik & 0xFF);
		return name;
	}

	//! Called once per keyboard update; applies due events. Returns false when inactive.
	inline bool Update()
	{
		if (!IsActive())
			return false;
		State& s = Get();
		if (!s.started)
		{
			QueryPerformanceFrequency(&s.freq);
			QueryPerformanceCounter(&s.start);
			s.started = true;
		}
		const double t = Now();
		EnterCriticalSection(&s.lock);
		if (s.failQuitAt > 0.0)
		{
			if (t >= s.failQuitAt)
			{
				LogF("quit (exit code 3)");
				if (s.log) fclose(s.log);
				ExitProcess(3);
			}
			LeaveCriticalSection(&s.lock);
			return true;
		}
		if (s.quitPending)
		{
			if (!s.recordingBusy)
			{
				LogF("quit");
				if (s.log) fclose(s.log);
				ExitProcess(0);
			}
			LeaveCriticalSection(&s.lock);
			return true;
		}
		if (s.waiting)
		{
			if (s.waitMet)
			{
				s.waiting = false;
				s.segmentStart = t;
				LogF("wait done after %.2f s: %s", t - s.waitStart, s.waitEvent->text);
			}
			else if (t - s.waitStart > s.waitEvent->timeout)
			{
				s.waiting = false;
				LogF("wait TIMED OUT after %.0f s: %s", s.waitEvent->timeout, s.waitEvent->text);
				_snprintf_s(s.pendingShot, MAX_PATH, _TRUNCATE, "%stimeout.png", s.baseDir);
				s.failQuitAt = t + 2.0;
				LeaveCriticalSection(&s.lock);
				return true;
			}
		}
		// a tap requested from Lua: press on this update, release 150 ms later (at least one update apart)
		if (s.luaTapDik)
		{
			if (!s.luaTapPressed)
			{
				s.keys[s.luaTapDik & 0xFF] = true;
				s.luaTapPressed = true;
				s.luaTapUp = t + 0.15;
				LogF("key down %s (lua)", KeyName(s.luaTapDik));
			}
			else if (t >= s.luaTapUp)
			{
				s.keys[s.luaTapDik & 0xFF] = false;
				LogF("key up %s (lua)", KeyName(s.luaTapDik));
				s.luaTapDik = 0;
			}
		}
		bool pressedNow[256] = { false };
		while (!s.waiting && s.next < s.count && s.events[s.next].time <= t - s.segmentStart)
		{
			const Event& e = s.events[s.next];
			// a release in the same update as its press would be invisible to the game (seen on
			// frame hitches); leave it and everything after it for the next update
			if (e.kind == E_Up && pressedNow[e.dik & 0xFF])
				break;
			// one Lua chunk per UI frame, in script order
			if (e.kind == E_Lua && s.pendingLua)
				break;
			if (e.kind == E_Down)
				pressedNow[e.dik & 0xFF] = true;
			++s.next;
			switch (e.kind)
			{
			case E_Down: s.keys[e.dik & 0xFF] = true;  LogF("key down %s", KeyName(e.dik)); break;
			case E_Up:   s.keys[e.dik & 0xFF] = false; LogF("key up %s", KeyName(e.dik)); break;
			case E_Shot: strncpy_s(s.pendingShot, e.text, _TRUNCATE); break;
			case E_Record: strncpy_s(s.pendingRecord, e.text, _TRUNCATE); s.pendingRecordFrames = e.dik; break;
			case E_Lua:  s.pendingLua = &e; break;
			case E_Wait:
				s.waiting = true; s.waitMet = false; s.waitStart = t; s.waitEvent = &e;
				LogF("wait (up to %.0f s): %s", e.timeout, e.text);
				break;
			case E_Quit:
				if (s.recordingBusy) { s.quitPending = true; LogF("quit waits for the recording"); break; }
				LogF("quit"); if (s.log) fclose(s.log); ExitProcess(0);
			}
		}
		LeaveCriticalSection(&s.lock);
		return true;
	}

	//! Scripted state of a DirectInput key (DIK_* code).
	inline bool IsKeyDown(int dik)
	{
		return Get().active && dik >= 0 && dik < 256 && Get().keys[dik];
	}

	//! Lua side (Blur2PC.Tap): queue a press and release of a key; false while an earlier tap is still held.
	inline bool QueueTap(int dik)
	{
		State& s = Get();
		if (!s.active || dik <= 0 || dik > 255)
			return false;
		EnterCriticalSection(&s.lock);
		const bool queued = s.luaTapDik == 0;
		if (queued)
		{
			s.luaTapDik = dik;
			s.luaTapPressed = false;
		}
		LeaveCriticalSection(&s.lock);
		return queued;
	}

	//! Returns the requested screenshot path once, or NULL.
	inline const char* TakeShotRequest()
	{
		State& s = Get();
		if (!s.active || !s.pendingShot[0])
			return NULL;
		static char path[MAX_PATH];
		EnterCriticalSection(&s.lock);
		strncpy_s(path, s.pendingShot, _TRUNCATE);
		s.pendingShot[0] = 0;
		LeaveCriticalSection(&s.lock);
		LogF("shot %s", path);
		return path;
	}

	//! The recorder has finished writing its file: a waiting `quit` may exit.
	inline void RecordingDone()
	{
		Get().recordingBusy = 0;
	}

	//! Returns the requested recording's path prefix once (frame count in *pFrames), or NULL.
	inline const char* TakeRecordRequest(int* pFrames)
	{
		State& s = Get();
		if (!s.active || !s.pendingRecord[0])
			return NULL;
		static char path[MAX_PATH];
		EnterCriticalSection(&s.lock);
		strncpy_s(path, s.pendingRecord, _TRUNCATE);
		*pFrames = s.pendingRecordFrames;
		s.pendingRecord[0] = 0;
		s.recordingBusy = 1;
		LeaveCriticalSection(&s.lock);
		LogF("record %s, %d frames", path, *pFrames);
		return path;
	}

	//! UI side: returns the Lua code of a due `lua` line once, or NULL.
	inline const char* TakeLuaRequest()
	{
		State& s = Get();
		if (!s.active || !s.pendingLua)
			return NULL;
		EnterCriticalSection(&s.lock);
		const char* code = s.pendingLua ? s.pendingLua->text : NULL;
		s.pendingLua = NULL;
		LeaveCriticalSection(&s.lock);
		return code;
	}

	//! UI side: the expression of the wait in progress, or NULL. Call WaitMet() when it is true.
	inline const char* WaitCondition()
	{
		State& s = Get();
		return (s.active && s.waiting && !s.waitMet) ? s.waitEvent->text : NULL;
	}

	//! Directory of the script, with a trailing separator.
	inline const char* ScriptDir()
	{
		return Get().baseDir;
	}

	inline void WaitMet()
	{
		Get().waitMet = true;
	}
}

#endif // PLATFORM_PC
#endif // SYSTEM_DEBUG_BLUR2PC_AUTOMATION_H
