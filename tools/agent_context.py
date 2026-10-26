"""Print bounded task context, log excerpts, or Codex session usage (stdlib only)."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys


PHASE_DOCS = {
    "Blur1PC": "docs", "Blur1Multi": "docs/blur1multi",
    "BlurX": "docs/blurx", "Blur1PCDX11": "docs/blur1pcdx11",
}


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig", errors="replace")


def bounded(text: str, limit: int) -> str:
    if len(text) <= limit:
        return text
    notice = "\n[Excerpt truncated; read the named file at the relevant section.]\n"
    if limit < len(notice):
        return notice[:limit]
    return text[:limit - len(notice)] + notice


def task_context(args: argparse.Namespace) -> str:
    root = args.root.resolve() if args.root else Path(__file__).resolve().parent.parent
    if root.name not in PHASE_DOCS:
        raise ValueError("--root must name a Blur phase worktree")
    docs = root / PHASE_DOCS[root.name]
    board = docs / "BOARD.md"
    ledger = docs / "LEDGER.md"
    tasks = set(args.task or [])
    header = f"Phase: {root.name}\nBoard: {board}\nLedger: {ledger}"
    requested = []
    owners = []
    for number, line in enumerate(read_text(board).splitlines(), 1):
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) < 7 or not re.fullmatch(r"T\d+", cells[0]):
            continue
        if cells[0] in tasks:
            requested.append(f"{number}: {line}")
        elif cells[4].lower().startswith(("open", "in progress", "blocked", "review")):
            owners.append(f"{number}: {cells[0]} | {cells[3]} | {cells[4][:65]} | {cells[5][:85]}")
    missing = tasks - set(re.findall(r"\bT\d+\b", read_text(board)))
    if missing:
        raise ValueError("Tasks absent from this phase's board: " + ", ".join(sorted(missing)))
    text = read_text(ledger)
    headings = list(re.finditer(r"^## \d{4}-\d{2}-\d{2}[^\n]*", text, re.MULTILINE))
    entries = [text[m.start():headings[i + 1].start() if i + 1 < len(headings) else len(text)]
               for i, m in enumerate(headings)]
    selected = set(range(max(0, len(entries) - args.recent), len(entries)))
    for task in tasks:
        matches = [i for i, entry in enumerate(entries) if re.search(r"\b" + re.escape(task) + r"\b", entry)]
        if matches:
            selected.add(matches[-1])
    # Reserve space for every category; a large board must not crowd out the ledger.
    labels = ["Requested tasks:", "Recent ledger entries (newest first):", "Active ownership / open tasks:"]
    sections = ["\n".join(requested) or "No task selected; choose --task Tnnn.",
                "\n".join(entries[i] for i in sorted(selected, reverse=True)), "\n".join(owners)]
    available = max(0, args.max_chars - len(header) - sum(map(len, labels)) - 12)
    budgets = [int(available * .22), int(available * .48)]
    budgets.append(available - sum(budgets))
    # Redistribute unused space before truncating any section.
    spare = sum(max(0, budget - len(section)) for budget, section in zip(budgets, sections))
    budgets = [min(budget, len(section)) for budget, section in zip(budgets, sections)]
    for i in (0, 1, 2):
        added = min(spare, len(sections[i]) - budgets[i])
        budgets[i] += added
        spare -= added
    output = [header] + [label + "\n" + bounded(section, budget)
                         for label, section, budget in zip(labels, sections, budgets)]
    return "\n\n".join(output)


def log_excerpt(args: argparse.Namespace) -> str:
    path = args.path.resolve()
    lines = read_text(path).splitlines()
    pattern = re.compile(args.pattern, re.IGNORECASE)
    indices = set()
    hits = [i for i, line in enumerate(lines) if pattern.search(line)]
    for i in hits[:args.matches]:
        indices.update(range(max(0, i - args.context), min(len(lines), i + args.context + 1)))
    indices.update(range(max(0, len(lines) - args.tail), len(lines)))
    output = [f"Log: {path}\nMatches: {len(hits)}; showing up to {args.matches}, plus tail."]
    previous = -2
    for i in sorted(indices):
        if i != previous + 1:
            output.append("...")
        output.append(f"{i + 1}: {lines[i]}")
        previous = i
    return "\n".join(output)


def usage(args: argparse.Namespace) -> str:
    paths = [args.session] if args.session else sorted(
        args.sessions_dir.rglob("*.jsonl"), key=lambda path: path.stat().st_mtime, reverse=True
    )[:args.latest]
    results = []
    for path in paths:
        records = []
        compactions = 0
        with path.open(encoding="utf-8") as stream:
            for line in stream:
                try:
                    event = json.loads(line)
                except json.JSONDecodeError:
                    continue
                payload = event.get("payload", {})
                compactions += event.get("type") == "compacted"
                if event.get("type") == "event_msg" and payload.get("type") == "token_count" and payload.get("info"):
                    records.append(payload["info"])
        if not records:
            results.append({"session": path.name, "usage": "No token usage recorded"})
            continue
        last = records[-1]
        total = last["total_token_usage"]
        inputs = [record["last_token_usage"]["input_tokens"] for record in records]
        # Duplicate usage events can occur; these are snapshots, not billable request counts.
        results.append({
            "session": path.name,
            "first_request_input_tokens": inputs[0],
            "last_request_input_tokens": inputs[-1],
            "peak_request_input_tokens": max(inputs),
            "cumulative_input_tokens": total["input_tokens"],
            "cumulative_cached_input_tokens": total.get("cached_input_tokens", 0),
            "cumulative_output_tokens": total["output_tokens"],
            "compactions": compactions,
        })
    return json.dumps(results, indent=2)


def positive(value: str) -> int:
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError("must be positive")
    return number


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--max-chars", type=positive, default=8000, help="Total output character cap (default: 8000)")
    commands = parser.add_subparsers(dest="command", required=True)
    context = commands.add_parser("context", help="Active board rows and recent ledger entries")
    context.add_argument("--root", type=Path)
    context.add_argument("--task", action="append", help="Task ID; repeat to select more than one")
    context.add_argument("--recent", type=positive, default=2)
    context.set_defaults(run=task_context)
    log = commands.add_parser("log", help="Error excerpts and a short tail from a saved log")
    log.add_argument("path", type=Path)
    log.add_argument("--pattern", default=r"\berror\b|fatal|failed|exception|traceback")
    log.add_argument("--matches", type=positive, default=8)
    log.add_argument("--context", type=positive, default=2)
    log.add_argument("--tail", type=positive, default=12)
    log.set_defaults(run=log_excerpt)
    metrics = commands.add_parser("usage", help="Measured request and cumulative tokens from local Codex rollouts")
    metrics.add_argument("--session", type=Path)
    metrics.add_argument("--sessions-dir", type=Path, default=Path.home() / ".codex" / "sessions")
    metrics.add_argument("--latest", type=positive, default=3)
    metrics.set_defaults(run=usage)
    args = parser.parse_args()
    sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")
    try:
        print(bounded(args.run(args), args.max_chars))
    except (OSError, ValueError, re.error, KeyError) as error:
        print(f"agent_context: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
