# 0122 — Item randomizer grants each TM at most once

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-12
- **Story:** — (bug fix; amends ADR 0098 / 0101 for TM sources)
- **ME reference:** —
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0098 hashes each chest into its pool, and ADR 0101 sends TM sources into TM01–50. With 28 TM balls plus about 13 gym and NPC TM gifts drawing independently from 50 TMs, collisions were near-certain. Players found the same TM twice, and with **TMs** on that meant the same move twice. The **TMs** dest map (ADR 0105) is already unique across TM01–50, tutors, and Cape Brink.

## Decision

1. **TM sources use a seed-global permutation** of the TM pool, not a per-site hash. Source TM*n* maps to `perm[n]`, so the grant site no longer matters.
2. Vanilla FRLG grants every TM01–50 at most once (ground, gym, NPC, roof drinks), so all TM grants are unique. Combined with ADR 0105, each TM move and tutor move is also unique.
3. Non-TM items keep per-site hashing (ADR 0098).
4. Marts and Game Corner TM stock stay vanilla (ADR 0088). A bought TM can still match a found one.

## Consequences

- Same seed: TM pickups change versus older builds. Already-collected items are unaffected.
- Gift dialogue (ADR 0102) stays consistent, because both the remap and the name buffer use the same mapping.
