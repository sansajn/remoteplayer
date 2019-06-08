#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include "rplib/observer.hpp"
#include "player_client.hpp"

struct playlist_event_listener
{
	virtual void on_playlist_play_item(size_t idx) {}
	virtual void on_playlist_remove_item(size_t idx) {}
	virtual void on_playlist_shuffle(bool shuffle) {}
};

class playlist_ui
	: public observable_with<playlist_event_listener>
{
public:
	static int const npos = -1;

	Gtk::Box _container;

	// item list
	Gtk::ScrolledWindow _ply_scroll;
	Gtk::ListViewText _ply_view;
	Gtk::Menu _ply_menu;

	// controls
	Gtk::Box _control_bar;
	Gtk::ButtonBox _left;
	Gtk::Button _up, _down, _clear_all;

	playlist_ui();
	void init();
	void add(std::string const & item);
	void highlight(size_t idx, playback_state_e s);
	int selected();
	void shuffle(bool state);

private:
	void on_item_clicked(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column);
	void on_shuffle_clicked();
	void on_playlist_button_press(GdkEventButton * button_event);
	void on_playlist_popup_play();
	void on_playlist_popup_remove();

	// controls
	Gtk::ButtonBox _right;
	Gtk::CheckButton _shuffle;
	sigc::connection _shuffle_clicked;
};
