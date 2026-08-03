import subprocess
import sys
import unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

SCANNED_SUFFIXES = {
    ".cpp", ".h", ".hpp", ".mm", ".qml", ".json", ".yml", ".yaml",
    ".md", ".py", ".txt", ".iss", ".desktop", ".in",
}

SKIPPED_DIRS = {
    ".git", "build", "third_party", "node_modules", "__pycache__", ".claude",
}

ALLOWED = set("–—‘’“”…©®™")


def is_emoji(character):
    codepoint = ord(character)
    if character in ALLOWED or codepoint < 0x2000:
        return False
    if unicodedata.category(character) in {"So", "Sk"}:
        return True
    return any(
        start <= codepoint <= end
        for start, end in (
            (0x1F000, 0x1FAFF),
            (0x2600, 0x27BF),
            (0xFE0F, 0xFE0F),
            (0x1F1E6, 0x1F1FF),
        )
    )


def scan_text(label, text, failures):
    for number, line in enumerate(text.splitlines(), start=1):
        for character in line:
            if is_emoji(character):
                name = unicodedata.name(character, "unnamed")
                failures.append(f"{label}:{number}: U+{ord(character):04X} {name}")


def scan_files(failures):
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in SCANNED_SUFFIXES:
            continue
        if any(part in SKIPPED_DIRS for part in path.relative_to(ROOT).parts):
            continue
        try:
            scan_text(path.relative_to(ROOT).as_posix(), path.read_text(encoding="utf-8"), failures)
        except UnicodeDecodeError:
            failures.append(f"{path.relative_to(ROOT).as_posix()}: not valid UTF-8")


def scan_commits(failures):
    try:
        log = subprocess.run(
            ["git", "log", "-40", "--no-merges", "--pretty=format:%h %s"],
            cwd=ROOT, capture_output=True, text=True, encoding="utf-8", check=True,
        ).stdout
    except (subprocess.CalledProcessError, FileNotFoundError):
        return
    scan_text("commit", log, failures)


def main():
    failures = []
    scan_files(failures)
    scan_commits(failures)

    if failures:
        print("Emoji found. This project does not use them anywhere.")
        for failure in failures:
            print(f"  {failure}")
        return 1

    print("No emoji found.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
