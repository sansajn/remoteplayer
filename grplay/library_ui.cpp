#include "rplib/log.hpp"
#include "library_ui.hpp"

library_ui::library_ui()
	: _container{Gtk::ORIENTATION_VERTICAL}
	, _filtered_media_list_view{1}
{}

void library_ui::init()
{
	_search.set_placeholder_text("<Enter search terms there>");

	_scroll.add(_media_list_view);
	_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

	_filtered_scroll.add(_filtered_media_list_view);
	_filtered_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

	_filtered_media_list_view.set_column_title(0, "Filtered Media:");

	_library_control_bar.pack_start(_playlist_add_button, Gtk::PackOptions::PACK_SHRINK);
	_library_control_bar.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);
	_playlist_add_button.set_image_from_icon_name("list-add");

	_bed_time.set_label("bed time");
	_bed_time_clicked = _bed_time.signal_clicked().connect(sigc::mem_fun(*this, &library_ui::on_bed_time_clicked));

	_control_bar.pack_start(_library_control_bar, Gtk::PackOptions::PACK_SHRINK);
	_control_bar.pack_end(_bed_time, Gtk::PackOptions::PACK_SHRINK);

	_container.pack_start(_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_container.pack_start(_search, Gtk::PackOptions::PACK_SHRINK);
	_container.pack_start(_control_bar, Gtk::PackOptions::PACK_SHRINK);
}

void library_ui::bed_time(bool state)
{
	_bed_time_clicked.block();
	_bed_time.set_active(state);
	_bed_time_clicked.unblock();
}

bool library_ui::bed_time() const
{
	return _bed_time.get_active();
}

void library_ui::on_bed_time_clicked()
{
	LOG(trace) << "library_ui::on_bed_time_clicked()";

	bool bed_time = _bed_time.get_active();
	for (auto * l : listeners())
		l->on_library_bed_time(bed_time);
}
