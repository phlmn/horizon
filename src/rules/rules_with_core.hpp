#pragma once
#include "rules.hpp"

namespace horizon {
RulesCheckResult rules_check(Rules *rules, RuleID id, class Core *c, class RulesCheckCache &cache,
                             check_status_cb_t status_cb);
void rules_apply(Rules *rules, RuleID id, class Core *c);
} // namespace horizon
