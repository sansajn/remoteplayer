#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/searchentry.h>
#include "library_tree_view.hpp"

class library_ui
{
public:
	Gtk::Box _container;

	Gtk::ScrolledWindow _filtered_scroll;
	Gtk::ListViewText _filtered_media_list_view;
	Gtk::ScrolledWindow _scroll;
	library_tree_view _media_list_view;
	Gtk::SearchEntry _search;
	Gtk::ButtonBox _library_control_bar;
	Gtk::Button _playlist_add_button;  // list-add

	library_ui();
};
