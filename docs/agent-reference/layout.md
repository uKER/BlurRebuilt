## Layout

`C:\AIProjects\Blur2PC\` is a container: a bare git repository (`.git`) and one worktree per branch, one branch per phase (D16, since 2026-09-25). Every worktree has the same layout, `code/` at its root. Before D16 (2026-09-24 to 2026-09-25, D10) the phases were folders of one working tree stacked at build time, and before D10 the Blur1PC contents were at the repo root; older ledger entries and analysis notes use those paths.

| Worktree | Branch | What |
|---|---|---|
| `Blur1PCSource/` | `master` | The leaked Blur 1 PC source snapshot (~2010-06-29), byte for byte as at `C:\AIProjects\Blur\code`. Never edited. `git diff master blur1pc` is every port change. |
| `Blur1PC/` | `blur1pc` | **Phase 1**: Blur 1 from retail PC data. The port of `master`: the source tree, the CMake build, the launchers (`play.bat`, `Blur.lnk`), `docs/`, this file, `tools/`, and its `_work/` (build output, the user's exe, and the scratch every phase shares: `vendor/`, `ghidra/`, `ghidra_proj/`, `retail_lua*/`). |
| `Blur1Multi/` | `blur1multi` | A higher phase; see its own AGENTS file. |
| `BlurX/` | `blurx` | A higher phase; see its own AGENTS file. |
| `Blur1PCDX11/` | `blur1pcdx11` | A higher phase; see its own AGENTS file. |

Inside a worktree:

| Path | What |
|---|---|
| `code/` | The source tree. `amax/`: the game (`game/`: entry `Main.cpp`, `GameApp.cpp`, `GameSystems.cpp`; `library/`: Ui, RaceBook, Network, Kanzen cameras; `tools/`: asset converters). `mercury/library/`: the engine (`common/`: system, xt2 gamedata, btl containers; `util/`: logicalFs, resource, input; `graphics/`, `physics/`, `gamesys/`, `ai/`, `audio/`, `ui/`). `mercurygame/`: older tech-test shell, superseded by `amax/`. `tools/`: build farm, bizpak, jam sources, exporters; ours: `jam2cmake/` (build generators), `ghidra_retail/`, and the phases' verification tools. `vendor/`: demonware, lua, zlib, libtom*, cxxtest, tclap; our shims (dxerr, bink-stub, efx-util, crt-compat, crash-handler). |
| `cmake/` | `blur_common.cmake` (the build description), `overrides.cmake` (hand tweaks), `targets.cmake` (generated, never edited). |
| `CMakeLists.txt` | The same file on every branch except its marked phase lines. |
| `docs/` | Blur 1 PC's docs (analysis, decisions, plan, board, ledger), about Blur 1 on PC only. Each higher phase keeps its own in `docs/<phase>/` on its branch (`docs/blur1multi/`, `docs/blurx/`, `docs/blur1pcdx11/`), which exists on that branch and the ones above it only. |
| `tools/` | `build.sh` (regenerate and build the phase this worktree holds). |
| `_work/` | Scratch (gitignored, so never inherited): `build/` (targets JSON, generated units, VS tree), `run/` (the user's exe, automation harness). Third-party SDKs are read from `Blur1PC/_work/vendor` by every phase (`blur_common.cmake`). |

External inputs (never modified, never copied into the repo):

- Retail Blur 1 PC: `C:\Program Files (x86)\Activision\Blur(TM)` (native little-endian reference for every format the source reads).
