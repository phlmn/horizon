#include "rule_match_editor.hpp"
#include "rules/rule_match.hpp"
#include "widgets/net_button.hpp"
#include "block.hpp"
#include "core/core_board.hpp"

namespace horizon {
	RuleMatchEditor::RuleMatchEditor(RuleMatch *ma, class Core *c): Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4), match(ma), core(c) {
		combo_mode = Gtk::manage(new Gtk::ComboBoxText());
		combo_mode->append(std::to_string(static_cast<int>(RuleMatch::Mode::ALL)), "All");
		combo_mode->append(std::to_string(static_cast<int>(RuleMatch::Mode::NET)), "Net");
		combo_mode->append(std::to_string(static_cast<int>(RuleMatch::Mode::NET_CLASS)), "Net class");
		combo_mode->append(std::to_string(static_cast<int>(RuleMatch::Mode::NET_NAME_REGEX)), "Net name (regex)");
		combo_mode->set_active_id(std::to_string(static_cast<int>(match->mode)));
		combo_mode->signal_changed().connect([this] {
			match->mode = static_cast<RuleMatch::Mode>(std::stoi(combo_mode->get_active_id()));
			sel_stack->set_visible_child(combo_mode->get_active_id());
			s_signal_updated.emit();
		});

		pack_start(*combo_mode, true, true, 0);
		combo_mode->show();

		sel_stack = Gtk::manage(new Gtk::Stack());
		sel_stack->set_homogeneous(true);
		Block *block = nullptr;
		if(auto co = dynamic_cast<CoreBoard*>(core)) {
			block = co->get_board()->block;
		}
		assert(block);
		net_button = Gtk::manage(new NetButton(block));
		net_button->set_net(match->net);
		net_button->signal_changed().connect([this](const UUID &uu) {
			match->net = uu;
		});
		sel_stack->add(*net_button, std::to_string(static_cast<int>(RuleMatch::Mode::NET)));

		auto *dummy_label = Gtk::manage(new Gtk::Label("matches all nets"));
		sel_stack->add(*dummy_label, std::to_string(static_cast<int>(RuleMatch::Mode::ALL)));

		pack_start(*sel_stack, true, true, 0);
		sel_stack->show_all();

		sel_stack->set_visible_child(std::to_string(static_cast<int>(match->mode)));
	}
}