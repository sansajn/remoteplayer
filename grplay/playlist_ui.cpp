#include "rplib/log.hpp"
#include "playlist_ui.hpp"

playlist_ui::playlist_ui()
	: _container{Gtk::Orientation::ORIENTATION_VERTICAL}
	, _ply_view{2}
{}

void playlist_ui::init()
{
	_container.pack_start(_ply_scroll, Gtk::PackOptions::PACK_SHRINK);
	_container.pack_start(_control_bar, Gtk::PackOptions::PACK_SHRINK);

	// item list
	_ply_scroll.set_size_request(-1, 150);
	_ply_scroll.add(_ply_view);
	_ply_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

	_ply_view.set_column_title(0, "Track");
	_ply_view.set_column_title(1, "Title");
	_ply_view.get_column(0)->set_alignment(Gtk::Align::ALIGN_CENTER);
	_ply_view.signal_button_press_event().connect_notify(sigc::mem_fun(*this, &playlist_ui::on_playlist_button_press));
	_ply_view.signal_row_activated().connect(sigc::mem_fun(*this, &playlist_ui::on_item_clicked));

	auto item = Gtk::manage(new Gtk::MenuItem("_Play", true));
	item->signal_activate().connect(sigc::mem_fun(*this, &playlist_ui::on_playlist_popup_play));
	_ply_menu.append(*item);

	item = Gtk::manage(new Gtk::MenuItem("_Remove", true));
	item->signal_activate().connect(sigc::mem_fun(*this, &playlist_ui::on_playlist_popup_remove));
	_ply_menu.append(*item);
//	_ply_menu.accelerate(*this);
	_ply_menu.show_all();

	// controls
	_control_bar.pack_start(_left, Gtk::PackOptions::PACK_SHRINK);
	_left.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);
	_left.pack_start(_up, Gtk::PackOptions::PACK_SHRINK);
	_left.pack_start(_down, Gtk::PackOptions::PACK_SHRINK);
	_left.pack_start(_clear_all, Gtk::PackOptions::PACK_SHRINK);

	_up.set_image_from_icon_name("go-up");
	_down.set_image_from_icon_name("go-down");
	_clear_all.set_image_from_icon_name("edit-delete");

	_control_bar.pack_end(_right, Gtk::PackOptions::PACK_SHRINK);
	_right.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);
	_right.pack_start(_shuffle, Gtk::PackOptions::PACK_SHRINK);
	_shuffle.set_label("shuffle");
	_shuffle_clicked = _shuffle.signal_clicked().connect(sigc::mem_fun(*this, &playlist_ui::on_shuffle_clicked));
}

void playlist_ui::add(std::string const & item)
{
	guint row = _ply_view.append();
	_ply_view.set_text(row, 1, item);
}

void playlist_ui::highlight(size_t idx, playback_state_e s)
{
	for (guint i = 0; i < _ply_view.size(); ++i)
	{
		if (i != idx || s == playback_state_e::invalid)
			_ply_view.set_text(i, 0, "");
		else
		{
			assert(s != playback_state_e::invalid);

			if (s == playback_state_e::playing)
				_ply_view.set_text(i, 0, "  >");
			else
				_ply_view.set_text(i, 0, "  ||");
		}
	}
}

int playlist_ui::selected()
{
	auto sel = _ply_view.get_selected();
	return !sel.empty() ? sel[0] : npos;
}

void playlist_ui::shuffle(bool state)
{
	_shuffle_clicked.block();
	_shuffle.set_active(state);
	_shuffle_clicked.unblock();
}

void playlist_ui::on_item_clicked(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column)
{
	int idx = selected();
	if (idx == npos)
		return;

	for (auto * l : listeners())
		l->on_playlist_play_item((size_t)idx);
}

void playlist_ui::on_shuffle_clicked()
{
	bool shuffle = _shuffle.get_active();

	for (auto * l : listeners())
		l->on_playlist_shuffle(shuffle);
}

void playlist_ui::on_playlist_button_press(GdkEventButton * button_event)
{
	if((button_event->type == GDK_BUTTON_PRESS) && (button_event->button == 3))
	{
		if (!_ply_view.get_selected().empty())
			_ply_menu.popup_at_pointer((GdkEvent *)button_event);
	}
}

void playlist_ui::on_playlist_popup_play()
{
	int idx = selected();
	if (idx == npos)
		return;

	for (auto * l : listeners())
		l->on_playlist_play_item((size_t)idx);
}

void playlist_ui::on_playlist_popup_remove()
{
	int idx = selected();
	if (idx == npos)
		return;

	for (auto * l : listeners())
		l->on_playlist_remove_item((size_t)idx);
}
