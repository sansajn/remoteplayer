#pragma once
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/checkbutton.h>
#include "rplib/observer.hpp"
#include "library_tree_view.hpp"

struct library_event_listener
{
	virtual void on_library_bed_time(bool bed_time) {}
};

class library_ui
	: public observable_with<library_event_listener>
{
public:
	Gtk::Box _container;

	Gtk::ScrolledWindow _filtered_scroll;
	Gtk::ListViewText _filtered_media_list_view;
	Gtk::ScrolledWindow _scroll;
	library_tree_view _media_list_view;
	Gtk::SearchEntry _search;
	Gtk::Button _playlist_add_button;  // list-add

	library_ui();
	void init();
	void bed_time(bool state);
	bool bed_time() const;

private:
	void on_bed_time_clicked();

	Gtk::Box _control_bar;
	Gtk::ButtonBox _library_control_bar;
	Gtk::CheckButton _bed_time;
	sigc::connection _bed_time_clicked;
};
