# Agent context and verification

Keep the information needed to decide and verify a change; retrieve other material when it becomes relevant.

## Task state

From the phase worktree, use:

```powershell
python tools/agent_context.py context
python tools/agent_context.py context --task T176
```

This prints the selected task rows, a compact active-ownership summary, and the two newest ledger entries plus the latest entry mentioning each selected task. It names the original files and board line numbers. The default output cap is 8,000 characters. If truncated, read the specific missing section before acting; the cap is not permission to ignore constraints or blockers. Set a larger explicit cap only when the relevant material requires it:

```powershell
python tools/agent_context.py --max-chars 12000 context --task T176 --recent 3
```

After startup, reread task state when ownership, blockers, or recent work may have changed. Query the task row or new ledger entries rather than repeating a complete board, ledger, AGENTS.md, or architecture read. Continue to claim tasks and append verification records under the phase protocol in workflow.md.

## Searches and tools

- Use `rg --files` to locate files and scoped `rg -n` searches to locate symbols. Read the necessary ranges before editing; expand only when dependencies or evidence require it.
- In `functions.exec`, print `result.output` and necessary status fields, not the whole result object. For parallel results, include a short source label and inspect every result. Keep images as image blocks, never JSON or base64 text.
- Set shell output budgets around 2,000-4,000 tokens. Batching calls does not justify dumping every full file; keep the aggregate returned text small. Explicitly raise a budget when needed to avoid losing essential evidence.
- Keep complete build and runtime logs in the phase's ignored `_work/` directory. Return the command's exit code, error summary, useful failure excerpts, and log path. A log excerpt helper does not establish build success and its exit code must not replace the build exit code.
- Use the process/session wait mechanism for running commands; avoid repeated empty polling or clock checks. Keep waits short enough to provide meaningful progress updates.
- Run the checks required by the change. Once they pass, repeat or broaden them only for a new change, a failure, or an unresolved concern. Preserve milestone and delivery checks required by the project.

## Log excerpts

```powershell
python tools/agent_context.py log _work/build/amax.log
python tools/agent_context.py log _work/run/example.log --pattern 'shader|fatal' --matches 6
```

The helper returns matching lines with neighboring context and a short tail, all under the same character cap. Read other ranges if the displayed failures need more context. Continue to judge localized MSBuild results using `tools/jam2cmake/errsum.py`, not English text matching alone.

## Measure results

```powershell
python tools/agent_context.py usage --latest 3
python tools/agent_context.py usage --session C:\Users\uKER\.codex\sessions\2026\09\29\rollout-example.jsonl
```

The helper reports recorded first, last, and peak request input tokens; cumulative input, cached input, and output tokens; and compaction counts. Rollout file bytes, character counts, and image base64 sizes are not token counts. Repeated token-count events are snapshots, not distinct billable requests. Cumulative input repeatedly counts carried context; cached input still occupies context.

For comparable completed tasks, record usage snapshots before and after the task, then subtract cumulative totals from the same session. Compare the same kind of task, model, and reasoning effort, including verification and outcome. Do not equate a lower token count with a better result if the required checks or task completion were lost.

Local Codex configuration sets `tool_output_token_limit = 4000` and `model_auto_compact_token_limit = 140000`. These are initial guardrails; measure before tuning further. Configuration and instruction changes take effect when sessions reload them; existing conversations retain earlier material until it is compacted or a fresh session is started.

## Skills and supporting documents

Read only the skill and reference material needed for the current workflow. A skill entry point should route to supporting documents rather than eagerly load every workflow. Installed system/plugin skills and harness instructions are maintained by their owners; do not overwrite them to shorten a project prompt. This project's build, architecture, layout, and workflow references are separate so agents can retrieve the relevant section without injecting the entire reference on every request.
