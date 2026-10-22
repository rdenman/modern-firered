#!/usr/bin/env python3
"""Title-case FireRed-visible all-caps labels (ADR 0098).

Re-run after an RHH merge if script/map name casing regresses:
    python3 tools/mf/title_case_text.py
    python3 tools/mf/title_case_text.py --test
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

SPECIAL = [
    ("POKéNAV", "PokéNav"),
    ("POKéCENTER", "Pokécenter"),
    ("POKéMANIAC", "Pokémaniac"),
    ("POKéFAN", "Pokéfan"),
    ("POKéDEX", "Pokédex"),
    ("POKéMON", "Pokémon"),
    ("POKé BALLS", "Poké Balls"),
    ("POKé BALL", "Poké Ball"),
    ("POKé FLUTE", "Poké Flute"),
    ("S.S. ANNE", "S.S. Anne"),
    ("S.S.ANNE", "S.S. Anne"),
    ("S.S. TICKET", "S.S. Ticket"),
    ("SR. AND JR.", "Sr. and Jr."),
    ("LT. SURGE", "Lt. Surge"),
    ("PROF. OAK", "Prof. Oak"),
    ("PROF. BIRCH", "Prof. Birch"),
    ("PROF. OAKS", "Prof. Oak's"),
    ("DIGLETT'S", "Diglett's"),
]

# Keep these tokens unchanged (case-sensitive match on the all-caps form).
ACRONYMS = {
    "HP",
    "PP",
    "TM",
    "HM",
    "PC",
    "TV",
    "ID",
    "OK",
    "VS",
    "EV",
    "IV",
    "GB",
    "SP",
    "LR",
    "KO",
    "EXP",
    "DNA",
    "BST",
    "NOA",
    "N64",
    "GBA",
    "RHH",
    "OHKO",
    "II",
    "III",
    "VI",
    "VII",
    "VIII",
    "IX",
}

SMALL = {"of", "and", "the", "or", "a", "an", "to", "in", "at"}

# Whole-word-ish all-caps run: letters, digits, '.', apostrophe.
WORD_RE = re.compile(r"[A-Z][A-Z0-9.']*")
BRACE_RE = re.compile(r"\{[^{}]*\}")
# Don't match __("…") raw encodings (naming keyboard insert tables).
C_STRING_RE = re.compile(
    r'((?:COMPOUND_STRING|(?<!_)_)\(")((?:\\.|[^"\\])*)("\))'
)
ASM_STRING_RE = re.compile(r'^(\s*\.string\s+")((?:\\.|[^"\\])*)(".*)$')
# Continuation lines in scripts: leading whitespace, quote, text, end quote
CONT_STRING_RE = re.compile(r'^(\s+")((?:\\.|[^"\\])*)(".*)$')


def apply_specials(text: str) -> str:
    for src, dst in SPECIAL:
        text = text.replace(src, dst)
    return text


def title_word(raw: str, is_first: bool) -> str:
    if not raw:
        return raw
    lead = 0
    while lead < len(raw) and not raw[lead].isalnum():
        lead += 1
    tail = len(raw)
    while tail > lead and not raw[tail - 1].isalnum() and raw[tail - 1] not in "'":
        tail -= 1
    prefix, core, suffix = raw[:lead], raw[lead:tail], raw[tail:]
    if not core:
        return raw
    upper = core.upper()
    core_nodot = core.rstrip(".")
    if core_nodot.upper() in ACRONYMS:
        return prefix + core_nodot.upper() + core[len(core_nodot) :] + suffix
    # TM01 / HM05 — acronym + digits is one WORD_RE token, not "TM".
    m = re.match(r"^([A-Z]+)(\d+)$", upper)
    if m and m.group(1) in ACRONYMS:
        return prefix + m.group(1) + m.group(2) + suffix
    # Mt. / Co. / Lt. / Prof. — not S.S.
    if core.endswith(".") and len(core) <= 5 and "." not in core[:-1]:
        body = core[:-1]
        titled = body[0] + body[1:].lower()
        return prefix + titled + "." + suffix
    # Don't recase an ALL-CAPS prefix of a mixed token (IDNo.)
    # handled by WORD_RE not eating lowercase; keep as-is if leftover lower follows.
    titled = core[0] + core[1:].lower()
    if not is_first and titled.lower() in SMALL:
        titled = titled.lower()
    return prefix + titled + suffix


def recase_plain(text: str) -> str:
    text = apply_specials(text)
    out = []
    i = 0
    is_first = True
    while i < len(text):
        if text[i] == "{":
            end = text.find("}", i)
            if end == -1:
                out.append(text[i:])
                break
            out.append(text[i : end + 1])
            i = end + 1
            continue
        # Sentence breaks inside the string
        if text.startswith("\\n", i) or text.startswith("\\p", i) or text.startswith("\\l", i):
            out.append(text[i : i + 2])
            i += 2
            is_first = True
            continue
        m = WORD_RE.match(text, i)
        if m:
            word = m.group(0)
            # Don't recase 1-letter (A button is often {A_BUTTON})
            if len(re.sub(r"[^A-Z]", "", word)) >= 2:
                out.append(title_word(word, is_first))
            else:
                out.append(word)
            i = m.end()
            is_first = False
            continue
        ch = text[i]
        out.append(ch)
        if ch in ".!?":
            is_first = True
        i += 1
    text = "".join(out)
    # WORD_RE treats dotted abbrevs / IDNo. as all-caps runs on re-runs.
    text = text.replace("S.s.", "S.S.")
    text = text.replace("Idno.", "IDNo.")
    # Already-title-cased TM01/HM05 ("Tm01") will not rematch WORD_RE.
    text = re.sub(r"\bTm(\d)", r"TM\1", text)
    text = re.sub(r"\bHm(\d)", r"HM\1", text)
    return text


def recase_quoted_chunks(text: str, pattern: re.Pattern[str]) -> tuple[str, int]:
    n = 0

    def repl(m: re.Match[str]) -> str:
        nonlocal n
        inner = m.group(2)
        # Alphabet lookup tables, not player-facing labels.
        if "ABCDEFGHIJKLMNOPQRSTUVWXYZ" in inner:
            return m.group(0)
        new = recase_plain(inner)
        if new != inner:
            n += 1
        return m.group(1) + new + m.group(3)

    return pattern.sub(repl, text), n


def should_skip_asm_line(line: str) -> bool:
    stripped = line.lstrip()
    return stripped.startswith("@") or stripped.startswith("/*")


def recase_inc(path: Path) -> int:
    lines = path.read_text().splitlines(keepends=True)
    changed = 0
    in_string_block = False
    out_lines = []
    for line in lines:
        if should_skip_asm_line(line):
            out_lines.append(line)
            continue
        m = ASM_STRING_RE.match(line.rstrip("\n"))
        if m:
            inner = recase_plain(m.group(2))
            if inner != m.group(2):
                changed += 1
            nl = "\n" if line.endswith("\n") else ""
            out_lines.append(m.group(1) + inner + m.group(3) + nl)
            in_string_block = not m.group(2).endswith("$") and (
                m.group(3).endswith("$") is False
            )
            # $ is inside the quotes usually
            in_string_block = "$" not in m.group(2)
            continue
        m = CONT_STRING_RE.match(line.rstrip("\n"))
        if m and (in_string_block or True):
            # Only treat as string continuation if it looks like script text
            if m.group(1).startswith("\t") or m.group(1).startswith(" "):
                inner = recase_plain(m.group(2))
                if inner != m.group(2):
                    changed += 1
                nl = "\n" if line.endswith("\n") else ""
                out_lines.append(m.group(1) + inner + m.group(3) + nl)
                in_string_block = "$" not in m.group(2)
                continue
        out_lines.append(line)
        if not line.strip().startswith('"'):
            in_string_block = False
    new = "".join(out_lines)
    if new != path.read_text():
        path.write_text(new)
    return changed


def recase_c_strings(path: Path) -> int:
    text = path.read_text()
    new, n = recase_quoted_chunks(text, C_STRING_RE)
    if new != text:
        path.write_text(new)
    return n


def recase_mapsecs(path: Path) -> int:
    data = json.loads(path.read_text())
    n = 0
    for sec in data.get("map_sections", []):
        name = sec.get("name")
        if not name:
            continue
        new = recase_plain(name)
        if new != name:
            sec["name"] = new
            n += 1
    if n:
        path.write_text(json.dumps(data, indent=2) + "\n")
    return n


def recase_party_names(path: Path) -> int:
    n = 0
    lines = path.read_text().splitlines(keepends=True)
    out = []
    for line in lines:
        if line.startswith("Name: "):
            raw = line[len("Name: ") :].rstrip("\n")
            if raw in ("PH", "Ph"):
                out.append("Name: PH" + ("\n" if line.endswith("\n") else ""))
                continue
            new = recase_plain(raw)
            if new != raw:
                n += 1
            nl = "\n" if line.endswith("\n") else ""
            out.append("Name: " + new + nl)
        else:
            out.append(line)
    if n:
        path.write_text("".join(out))
    return n


def self_test() -> None:
    cases = {
        "TM01": "TM01",
        "HM05": "HM05",
        "Tm26": "TM26",
        "Hm01": "HM01",
        "{PLAYER} received Tm26": "{PLAYER} received TM26",
        "Tm32{CLEAR_TO 72}1,500 Coins": "TM32{CLEAR_TO 72}1,500 Coins",
        "Oak: Hm…": "Oak: Hm…",
        "Hmm…": "Hmm…",
        "TMs & HMs": "TMs & HMs",
        "HP": "HP",
    }
    failed = []
    for src, want in cases.items():
        got = recase_plain(src)
        if got != want:
            failed.append(f"{src!r}: got {got!r}, want {want!r}")
    if failed:
        raise SystemExit("title_case_text self-test failed:\n" + "\n".join(failed))


def main() -> int:
    if "--test" in sys.argv:
        self_test()
        print("ok")
        return 0

    report = []

    mapsec = ROOT / "src/data/region_map/region_map_sections.json"
    n = recase_mapsecs(mapsec)
    report.append(f"{mapsec.relative_to(ROOT)}: {n} names")

    party = ROOT / "src/data/trainers_frlg.party"
    n = recase_party_names(party)
    report.append(f"{party.relative_to(ROOT)}: {n} trainer names")

    c_files = [
        ROOT / "src/battle_main.c",
        ROOT / "src/strings.c",
        ROOT / "src/credits_frlg.c",
        ROOT / "src/fame_checker.c",
        ROOT / "src/battle_message.c",
        ROOT / "src/main_menu.c",
        ROOT / "src/start_menu.c",
        ROOT / "src/option_menu.c",
        ROOT / "src/new_game.c",
        ROOT / "src/pokedex.c",
        ROOT / "src/pokedex_plus_hgss.c",
        ROOT / "src/pokemon_summary_screen.c",
        ROOT / "src/naming_screen.c",
        ROOT / "src/hall_of_fame_frlg.c",
        ROOT / "src/trainer_card.c",
        ROOT / "src/map_name_popup.c",
        ROOT / "src/region_map.c",
        ROOT / "src/item_menu.c",
        ROOT / "src/party_menu.c",
        ROOT / "src/player_pc.c",
        ROOT / "src/pokemon_storage_system.c",
        ROOT / "src/mf_options_menu.c",
        ROOT / "src/mf_rules_menu.c",
        ROOT / "src/oak_speech.c",
        ROOT / "src/battle_controller_oak_old_man.c",
        ROOT / "src/fishing.c",
        ROOT / "src/item_use.c",
        ROOT / "src/field_screen_effect.c",
        ROOT / "src/landmark.c",
        ROOT / "src/mystery_event_msg.c",
    ]
    for extra in (ROOT / "src/data/text").glob("*.h"):
        c_files.append(extra)
    for extra in (ROOT / "src/data").glob("party_menu.h"):
        c_files.append(extra)
    for extra in (ROOT / "src/data").glob("script_menu.h"):
        c_files.append(extra)

    for path in c_files:
        if not path.exists():
            continue
        n = recase_c_strings(path)
        if n:
            report.append(f"{path.relative_to(ROOT)}: {n} C strings")

    inc_files = []
    for path in (ROOT / "data/maps").rglob("scripts.inc"):
        if any(part.endswith("_Frlg") for part in path.parts):
            inc_files.append(path)
    inc_files.extend((ROOT / "data/text").glob("*.inc"))
    inc_files.extend((ROOT / "data/scripts").glob("*.inc"))

    inc_changed = 0
    inc_files_n = 0
    for path in sorted(set(inc_files)):
        n = recase_inc(path)
        if n:
            inc_changed += n
            inc_files_n += 1
    report.append(f"script/text .inc: {inc_changed} strings in {inc_files_n} files")

    print("\n".join(report))
    return 0


if __name__ == "__main__":
    sys.exit(main())
