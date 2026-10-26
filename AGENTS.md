# Agent instructions

## Phase and ownership

Work in the phase that owns the change. `Blur1PCSource/master` is the untouched source and must never be edited. The phase worktrees are:

| Worktree / branch | Task documents |
|---|---|
| Blur1PC / blur1pc | docs/ |
| Blur1Multi / blur1multi | docs/blur1multi/ |
| BlurX / blurx | docs/blurx/ |
| Blur1PCDX11 / blur1pcdx11 | docs/blur1pcdx11/ |

At startup run `python tools/agent_context.py context` from this worktree. Read the selected task row and the recent ledger entries; use `context --task Tnnn` once the task is identified. Read the stage definition in [workflow](docs/agent-reference/workflow.md) before stage work. Claim a task before editing: Owner, Status, phase, and expected files. One task per agent; parallel agents use disjoint tasks/files. Facts and decisions belong to the owning phase's analysis.md and decisions.md. Append a ledger entry and update the board when finishing or stopping mid-task; record verification, blockers, and the exact next step.

A fault belongs to the lowest phase whose own executable reproduces it on its own data; record Found in / Belongs to. Fix lower-phase faults there, never in a higher branch. Any unavoidable higher-phase stopgap lives in a higher-phase file, is marked, and has a removal task. Base unity/header dependencies are base faults. Shared AGENTS.md and tools are authored in Blur1PC and inherited above. Phase docs describe only that phase's additions.

Commit only when the user asks; stage only your changes, by hunk when needed. After a lower-phase commit, merge it through every higher phase before calling it done; never rebase shared branches. Never push unless asked. Preserve other sessions' changes. Never silently reverse an accepted decision; record direction changes as proposed until the user confirms. Deleting an upper branch must leave lower phases unchanged. See [workflow](docs/agent-reference/workflow.md) for the full protocol and stage definitions.

## Correctness and build

Replicate the actual retail mechanism faithfully for reported gameplay/engine issues: compare pristine source or decompile the relevant retail binary with Ghidra. Give recovered unnamed entities readable names and tags. Do not invent guessed values, workarounds, or approximations as fixes. If the mechanism cannot be recovered, say what is unverified, record it in the owning phase's analysis.md, and label any substitute a stopgap. PC serializers are the format authority; record unknowns as unknowns.

Every implementation must run correctly with an unlocked frame rate, in every phase (Blur1PC, Blur1Multi, BlurX). The simulation keeps its fixed-step cadence while rendering runs at any rate, so code that assumes one rendered frame per game tick is a fault. This has already required changes across audio, physics, shaders/effects, particles, animation and cameras. New or ported code (including Blur 2 code recovered from Kanzen) must advance by elapsed time or fixed simulation ticks with interpolation, never by rendered-frame count. This is a design rule: nothing may be implemented in a way that only works at 60 Hz. It is not a test requirement; a change does not need separate runs at 60 Hz and unlocked unless it alters frame-timed behaviour or a frame-rate fault is being investigated.

Keep new engine code beside what it extends; new project tools under root tools/<name>/. Keep code/tools/ for tools shipped with the leaked source. Do not restructure the source tree. Put build and scratch output under this phase's ignored _work/. Base source edits carry `// [Blur2PC]` comments. Generate source lists from the jamfiles; never hand-edit cmake/targets.cmake. Regenerate and commit it after sources change, including after merges. Regenerate .cp/.cv/.sp headers in each worktree and built-in shader blobs after shader edits. Build tweaks go in cmake/overrides.cmake with a reason; shared flags/SDKs/shims in cmake/blur_common.cmake.

Use `tools/build.sh` from the phase worktree for the standard Release build. Before building or changing generators, read the relevant section of [build reference](docs/agent-reference/build.md), which preserves the exact commands and toolchain details. Judge localized MSBuild results by errsum.py's error count. Deliver Amax.exe and Amax.pdb to this phase's _work/run/ from _work/build/vs/Release; wait for a running delivered exe to exit before copying. The user's launcher uses only that location. Do not substitute a renamed/private output or another phase's build. A private automated-run copy is allowed but does not count as delivery. Say ready only after the standard executable contains the change; otherwise report pending delivery.

## Runtime and communication

Never kill processes you do not own. Queue every automated game run through tools/game_slot.ps1; do not poll for a free slot or ask other sessions to pause. Launch the game minimized, never foreground or hidden; use the phase's _work/run/run_script.ps1. Prefer debug-menu races (`Ui.NewLaunchMode = DebugRb`, Right Ctrl then Enter); read the debug-menu setup and limits in the relevant phase analysis. Never capture PNG bursts; record compressed video or take one still when sufficient.

Continue until the task is complete, user input is required, or an unrecoverable condition prevents progress. Do not stop merely to report progress. Post-commit summaries omit other agents' pending uncommitted work.

Whenever applicable, when a task is considered done, end up the handover by providing a link to the verification screenshot used as the approval.

## Context and verification

Use scoped rg searches and relevant file ranges. Return tool output and necessary status fields rather than whole result objects; keep shell results around 2,000-4,000 tokens and batch output bounded. Save full logs under _work/, report exit status and a useful error excerpt, and read further ranges if needed. Use process/session waits rather than repeated empty polls. Run required checks; repeat them only after changes, failures, or unresolved concerns.

Retrieve task state selectively, including active ownership and blockers; do not repeatedly load whole boards, ledgers, or this file. Truncated excerpts require retrieving the missing relevant section before acting. `python tools/agent_context.py log <path>` returns bounded log excerpts; `usage --latest 3` reports recorded token usage. Read [context guidance](docs/agent-reference/context.md) when using these helpers or measuring efficiency. Keep model and task outcomes comparable when measuring tokens per completed task.

Load supporting sections only when relevant: [layout](docs/agent-reference/layout.md) for paths/history, [architecture](docs/agent-reference/architecture.md) for engine mechanisms, [build](docs/agent-reference/build.md) for generation/toolchain, [workflow](docs/agent-reference/workflow.md) for stage/coordination details. Read relevant installed skills progressively; do not preload every reference or overwrite owner-maintained system/plugin prompts.
