#pragma once
#include "core.hpp"
#include <forward_list>

namespace horizon {

class ToolSetNotConnected : public ToolBase {
public:
    ToolSetNotConnected(Core *c, ToolID tid);
    ToolResponse begin(const ToolArgs &args) override;
    ToolResponse update(const ToolArgs &args) override;
    bool can_begin() override;
    bool is_specific() override
    {
        return false;
    }

private:
    enum class Mode { SET, CLEAR, TOGGLE };
    Mode mode = Mode::SET;
    void update_tip();
};
} // namespace horizon
