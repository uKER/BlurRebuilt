# Automation scripts

Unattended test runs for every phase. The game reads a script named by the environment variable
`BLUR2PC_AUTOMATION`; `<Phase>/_work/run/run_script.ps1 -Script <file>` sets it, starts the game
minimised, waits for it to exit and prints `automation.log`. The script format is documented at the
top of `code/mercury/library/common/system/include/system/debug/Blur2PCAutomation.h`.

Commands: `tap`, `down`, `up` (keys), `shot` (back-buffer PNG), `record <prefix> <frames>` (the
next consecutive presented frames at half size, encoded by ffmpeg into `<prefix>.mp4`, with each
frame's time in `<prefix>_times.txt`; for motion, flicker and pacing checks. ffmpeg comes from PATH
or `BLUR2PC_FFMPEG`, the frame rate from `BLUR2PC_FRAMELIMIT`, else 60. The game keeps its pace while
recording, and a `quit` waits until the file is finished), `quit`,
`lua <code>` (runs in the UI Lua state) and `wait <timeout> <Lua expression>`. A `wait` blocks the lines below it until the
expression is true; their times count from that moment. A wait that times out saves `timeout.png`
and exits with code 3, so a broken run fails fast and says where it stopped.

`automation.log` lists every screen change in every UI context, every key, wait, Lua line and
screenshot, with seconds since the window opened.

The key names include `NUMPADPLUS`, `NUMPADENTER` and `NUMPAD0` for split-screen menu checks.

Lua helpers available to scripts while automation is active:

| Helper | Returns |
|---|---|
| `Blur2PC.RaceState()` | race logic state: `None`, `Loading`, `Intro`, `Starting` (countdown), `Racing`, `Finished`, `Outro`, `Quitting`, `Restarting`, `GoingToNextEvent` |
| `Blur2PC.Tap(key)` | presses and releases a key (names as for `tap`); false while an earlier tap is held. For menus that take different paths run to run: poll a Lua function from a `wait` that taps by what is on screen (`career.lua`) |
| `Blur2PC.Log(text)` | writes a line to `automation.log` |
| `Blur2PC.Run(file)` | runs a Lua file (absolute, or relative to the script) |
| `UIScreen.GetCurrentScreen([context])` | the game's own: current screen, lower case, e.g. `intro\startscreen.lua` |

Write Lua strings that contain backslashes as long strings (`[[intro\startscreen.lua]]`): in a
quoted Lua string `\s` is not a backslash.

## Files

- `quickrace.lua`: `Blur2PC_QuickRace(tier, event[, car])` starts a stock career event from the
  start screen with the calls the career menus make (listed in the file), skipping the menus.
  `Blur2PC_QuickRaceReady()` is true once the profile has loaded.
- `quick_race.txt`: boot, start screen, career tier 1 event 1, vista skip, throttle from the green
  light, three driving screenshots, quit.
- `career.lua`: `Blur2PC_GoCareer()`, polled by a `wait`, presses RETURN through the start screen,
  "Previously on Blur" (context 6, which sometimes waits for a key and sometimes closes on its own)
  and the single player carousel until the Career tier screen (context 5) is up.
- `career_record.txt`: boot, start screen, Career through the real menus (`career.lua`), then records
  240 frames of the tier screen.

Never capture bursts of PNGs (repeated `shot` lines): each one is written on the render thread and
freezes the game, which the user sees on the taskbar with its sound looping (AGENTS.md rule 12). Use
`record` for anything that moves; extract stills from the video with ffmpeg when needed.

Frame-rate-sensitive runs: the player's `settings.xml` may have vsync on, which paces a scripted run
to the display (about 30 fps while the display sleeps). Point `BLUR2PC_SETTINGS` at a copy with
`vsync="0"` and set `BLUR2PC_FRAMELIMIT` (for example 144) to test a given render rate; launch
through `Blur1PC/tools/game_slot.ps1` so other sessions' runs queue.

Saves: like retail, the game keeps its saves in `%APPDATA%\Bizarre Creations\Blur\SAVE\BLURSAVE` (and
`PHOTO`, `VINYL`). `BLUR2PC_SAVE_ROOT` replaces the `%APPDATA%\Bizarre Creations\Blur` folder; a run
script sets it to a private folder so scripted runs never load or overwrite the player's career. To test
with a given save, copy it to `<folder>\SAVE\BLURSAVE` and point `BLUR2PC_SAVE_ROOT` at `<folder>`.

The full menu run (`race_long.txt` in `Blur1PC/_work/run/scripts`) still exercises the front end;
it is the T061 regression run.
