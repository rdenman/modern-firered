# 0093 — Rival starter is the Oak ball; balancing never leaves its evo stage

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** S53
- **ME reference:** —
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

ADR 0056 remaps Oak's three balls through S51 (STATIC, Balancing applies) and keeps Gary's *ball* via type advantage. His actual fights still went through S53 trainer remap of vanilla Charmander / Squirtle / Bulbasaur. That second roll could disagree with the lab text. ADR 0057 also lets themed Balancing fall through to "any stage of that type," so a level-5 lab rival could become Haunter even with Balancing on.

## Decision

1. **Balancing** — while `similar` is on, `MfSpeciesMapEx` / `MfSpeciesMapExForType` only pick the same evo-stage bucket as the source. HM and theme filters may widen BST or drop HM, but they do not promote Gastly→Haunter on a basic-stage source. If no typed same-stage dest exists, themed remap identity-maps the source (mixed party) rather than upgrading stage.
2. **Rival starter slot** — when `randomStarter` is on, FRLG rival / Champion parties (`TRAINER_RIVAL_*` through Champion rematch) do not trainer-remap the Kanto starter line. That slot is the Oak rival species, walked along its live evo table to the authored slot's stage (lab Charmander → Oak pick; Cerulean Charmeleon → that pick's first evo). Other party mons still remap.

## Alternatives considered

- **Trainer remap only, keep stage** — rejected; the lab text and the fight would still be different species.
- **Persist rival species in a new var** — rejected; Oak remap is already deterministic from `VAR_STARTER_MON` + S16.

## Consequences

- ADR 0057's "any stage of that type" last step is narrowed: same stage first; type without stage-up is not used under Balancing.
- Changing which trainer ids pin the starter is a mapping break for rival fights only.
