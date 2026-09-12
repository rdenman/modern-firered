#include "global.h"
#include "mf_rules.h"

void MfRules_ResetToEmpty(struct ModernRules *rules)
{
    if (rules == NULL)
        return;

    memset(rules, 0, sizeof(*rules));
    rules->version = MF_RULES_VERSION;
    rules->monotype = 31; // ME TX_CHALLENGE_TYPE_OFF
}

void MfRules_Pack(const struct ModernRules *rules, u8 *out)
{
    if (rules == NULL || out == NULL)
        return;

    memcpy(out, rules, sizeof(struct ModernRules));
}

void MfRules_Unpack(const u8 *in, struct ModernRules *rules)
{
    if (in == NULL || rules == NULL)
        return;

    memcpy(rules, in, sizeof(struct ModernRules));
}

struct ModernRules *MfRules_GetSaveRules(void)
{
    return &gSaveBlock3Ptr->mfRules;
}
