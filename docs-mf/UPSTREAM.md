# Upstream merge workflow

How to pull from [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) (`RHH/master`) without losing Modern FireRed work.

Our product docs and ADRs live under **`docs-mf/`**, not upstream’s `docs/` (their mdbook). Do not put Modern FireRed guides in `docs/`.

## Remotes

Expected remotes (names matter for the commands below):

| Remote | URL |
| ------ | --- |
| `origin` | this fork (`rdenman/modern-firered` or your fork) |
| `RHH` | `https://github.com/rh-hideout/pokeemerald-expansion.git` |

Check:

```bash
git remote -v
```

If `RHH` is missing:

```bash
git remote add RHH https://github.com/rh-hideout/pokeemerald-expansion.git
```

Do **not** add Modern Emerald (`resetes12/pokeemerald`) as a remote or merge from it. ME is a read-only spec — fetch individual files with `curl` when needed (see `AGENTS.md`).

## Fetch and merge

Prefer a normal merge (not rebase) so upstream history stays visible and conflict resolution is reviewable.

```bash
git fetch RHH master
git merge RHH/master
```

If the merge is a no-op (`Already up to date.`), you are current; still run the [post-merge checklist](#post-merge-checklist) after any local change that might interact with upstream headers or the Makefile.

### Conflict strategy by area

| Area | Prefer | Then |
| ---- | ------ | ---- |
| `include/config/*.h` | **Theirs** (RHH) for the shared file body | Re-apply our compile-time flips and any `MF_` include site. Our knobs are additive; do not keep an old upstream default just because we edited nearby lines. |
| `.github/workflows/build.yml` | **Ours** (slim FireRed + `make check` gate — ADR 0002) | Re-apply any new shared install/cache steps from RHH that we still need. |
| `Makefile` | **Theirs** for shared rules | Keep our `mf_%.o` warning policy and never change the default target to FireRed. |
| `src/mf_*.c`, `include/mf_*.h`, `include/config/modern_firered.h`, `docs-mf/` | **Ours** | These should not exist upstream; if they collide, something is wrong. |
| One-line call sites into `mf_` helpers inside upstream `.c` files | Resolve carefully | Keep the `mf_` call; take upstream’s surrounding logic. |

After resolving config conflicts, diff against `RHH/master` for each flipped macro and confirm our intended value is still present (Phase 1+ stories document which flips we own).

## Post-merge checklist

Run from a clean tree after the merge commit exists:

```bash
make clean
make firered -j$(sysctl -n hw.ncpu)
make check
```

Confirm:

1. `pokefirered.gba` is produced and boots in mGBA.
2. `make check` is green (Emerald TESTELF — expansion’s test runner does not target FireRed).
3. No new warnings in `src/mf_*.c` / objects built by the `mf_%.o` rule.
4. If save structs changed upstream, re-read S12 / S64 notes before shipping — never silently reshuffle our rules blob.

## Naming (preview — owned by S04)

- Configs: `MF_*` in `include/config/modern_firered.h`
- Sources: `src/mf_*.c`, `include/mf_*.h`
- Decision records: `docs-mf/decisions/` (see that folder’s README for the ADR template)

## Trial merge log

| Date | `RHH/master` tip | Result |
| ---- | ---------------- | ------ |
| 2026-09-11 | `d74f64dc9f` (merge-base; 0 commits behind) | Already up to date. Post-merge checklist run green — see S03. |
