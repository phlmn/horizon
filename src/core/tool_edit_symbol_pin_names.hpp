#pragma once
#include "block/component.hpp"
#include "core.hpp"
#include <forward_list>
#include "tool_helper_get_symbol.hpp"

namespace horizon {

class ToolEditSymbolPinNames : public ToolHelperGetSymbol {
public:
    ToolEditSymbolPinNames(Core *c, ToolID tid);
    ToolResponse begin(const ToolArgs &args) override;
    ToolResponse update(const ToolArgs &args) override;
    bool can_begin() override;
    bool is_specific() override
    {
        return true;
    }
};
} // namespace horizon
