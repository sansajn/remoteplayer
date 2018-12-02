#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include "player_client.hpp"

class playlist_ui
{
public:
	static int const npos = -1;

	Gtk::Box _container;

	// item list
	Gtk::ScrolledWindow _ply_scroll;
	Gtk::ListViewText _ply_view;
	Gtk::Menu _ply_menu;

	// controls
	Gtk::Box _controls;
	Gtk::ButtonBox _left;
	Gtk::Button _up, _down, _clear_all;

	playlist_ui();
	void init();
	void add(std::string const & item);
	void highlight(size_t idx, playback_state_e s);
	int selected();

private:
	void on_playlist_button_press(GdkEventButton * button_event);
};
