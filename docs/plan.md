# Plan

Milestone order and the workstreams inside each. Tasks are tracked on `BOARD.md`; this file explains why they exist and how they fit. Update it when a milestone's shape changes.

## M0 — Workspace (done 2026-09-23)
AGENTS.md, docs, board, ledger, gitignore, git init.

## M1 — The source builds and runs Blur 1 on PC
Nothing else can be tested until this exists. Retail Blur 1 PC data is the test input.
- W1.1 Build description: resolve D08; produce a build that compiles `amax` WIN32 Speed and Debug.
- W1.2 Stubs: Bink movie player (no-op or skip), Massive ads, Demonware/online (compile-time off or stub), OpenAL (real).
- W1.3 Run against `C:\Program Files (x86)\Activision\Blur(TM)` data: front end appears, Showroom loads, one race runs. Fix crashes caused by the 2012 retail data being newer than the June 2010 source, if any.
- W1.4 Debug affordances: `FileMappingDebug`, logfs, a command-line data-root override (the source has none).

## M7 — Time independence
Convert the frame-counted consumers listed in `analysis.md` §2 to elapsed time; keep the fixed 60 Hz step; verify at 30 / 60 / 144 fps with a debug frame limiter.

## Phase layout (D16, 2026-09-25; D10 before that)
Each phase is a git branch in its own worktree under `C:\AIProjects\Blur2PC\`: `Blur1PCSource/` (`master`, the untouched leak), `Blur1PC/` (`blur1pc`, phase 1), `Blur1Multi/` (`blur1multi`, phase 2), `BlurX/` (`blurx`, phase 3), each branched from the previous one. Lower-phase commits reach the upper phases by `git merge`, nothing flows down. M1 and M7 work is committed on `blur1pc`. Docs, board and ledger are edited in `Blur1PC/` only.

## Standing goal (D09)
Blur 1 on retail PC data remains a supported configuration after every milestone; re-run the T005 smoke test (boot, start screen, Showroom, one race) before closing any milestone.

## Out of scope
Multiplayer, Demonware, Live/PSN, DLC, achievements, avatar awards.
