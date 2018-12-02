#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/volumebutton.h>

using Glib::RefPtr;

class playback_ui
{
public:
	Gtk::Box _vbox;  // _containter
	Gtk::Label _player_media;
	Gtk::Box _progress_hbox;
	Gtk::Label _player_position;
	Gtk::Scale _player_progress;
	RefPtr<Gtk::Adjustment> _progress_adj;
	Gtk::Label _player_duration;
	Gtk::Box _control_bar_hbox;
	Gtk::ButtonBox _control_bar_l;
	Gtk::Button _prev_button, _play_button, _stop_button, _next_button;
	Gtk::ButtonBox _control_bar_r;
	Gtk::VolumeButton _volume;
	RefPtr<Gtk::Adjustment> _volume_adj;

	playback_ui();
};
