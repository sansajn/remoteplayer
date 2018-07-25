// Graphics Remote Player Client
#include <string>
#include <regex>
#include <iostream>
#include <cassert>
#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/searchentry.h>
#include "log.hpp"
#include "helpers.hpp"
#include "player_client.hpp"
#include "json.hpp"

using std::string;
using std::vector;
using std::regex_search;
using std::smatch;
using std::regex;
using std::cout;
using Glib::RefPtr;
using Glib::ustring;


class rplay_window : public Gtk::Window
{
public:
	rplay_window();

protected:
	void on_queue_button();
	void on_search();
	void repack_ui();
	void filter_media_library(string const & filter);
	fs::path const & get_media(int sel_idx) const;

	player_client _play;
	vector<int> _filtered_lookup;
	bool _filtered;

	// gui
	Gtk::Box _vbox;
	Gtk::SearchEntry _search;
	Gtk::ScrolledWindow _filtered_scroll;
	Gtk::ListViewText _filtered_media_list_view;
	Gtk::ScrolledWindow _scroll;
	Gtk::ListViewText _media_list_view;
	Gtk::ButtonBox _button_box;
	Gtk::Button _queue_button;
};

rplay_window::rplay_window()
	: _filtered{false}
	,  _vbox{Gtk::Orientation::ORIENTATION_VERTICAL}
	, _filtered_media_list_view{1}
	, _media_list_view{1}
	, _queue_button{">"}
{
	string const & host = "localhost";

	_play.connect(host);  // TODO: blocking

	set_title("Remote Player Client");
	set_default_size(600, 700);

//	_vbox.set_margin(6);
	add(_vbox);

	_search.set_placeholder_text("<Enter search terms there>");
	_search.signal_changed().connect(sigc::mem_fun(*this, &rplay_window::on_search));

	_filtered_scroll.add(_filtered_media_list_view);
	_filtered_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

	_scroll.add(_media_list_view);
	_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

//	_button_box.set_margin(5);
	_button_box.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);
	_button_box.pack_start(_queue_button, Gtk::PackOptions::PACK_SHRINK);
	_queue_button.signal_clicked().connect(sigc::mem_fun(*this, &rplay_window::on_queue_button));

	_filtered_media_list_view.set_column_title(0, "Media");
	_media_list_view.set_column_title(0, "Media");

	for (fs::path const & media : _play.list_media())
		_media_list_view.append(media.string());

	// pack
	_vbox.pack_start(_search, Gtk::PackOptions::PACK_SHRINK);
	_vbox.pack_start(_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_button_box, Gtk::PackOptions::PACK_SHRINK);

	show_all();
}

void rplay_window::repack_ui()
{
	if (_filtered)
	{
		_vbox.remove(_scroll);
		_vbox.pack_start(_filtered_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_vbox.reorder_child(_filtered_scroll, 1);
	}
	else
	{
		_vbox.remove(_filtered_scroll);
		_vbox.pack_start(_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_vbox.reorder_child(_scroll, 1);
	}

	show_all();
}

void rplay_window::on_queue_button()
{
	Gtk::ListViewText::SelectionList selection;
	if (_filtered)
		selection = _filtered_media_list_view.get_selected();
	else
		selection = _media_list_view.get_selected();

	assert(!selection.empty());

	fs::path const & media = get_media(selection[0]);
	_play.play(media.string());
}

void rplay_window::on_search()
{
	ustring filter = _search.get_text();
	filter_media_library(filter);
}

void rplay_window::filter_media_library(string const & filter)
{
	if (filter.empty())
	{
		_filtered = false;
		repack_ui();
		return;
	}

	regex pat{filter, std::regex_constants::icase};

	_filtered_lookup.clear();
	vector<fs::path> const & media_list = _play.list_media();
	for (size_t i = 0; i < media_list.size(); ++i)
	{
		fs::path const & media = media_list[i];
		smatch match;
		if (regex_search(media.native(), match, pat))
			_filtered_lookup.push_back(i);
	}

	_filtered_media_list_view.clear_items();
	for (int idx : _filtered_lookup)
		_filtered_media_list_view.append(media_list[idx].string());

	_filtered = true;
	repack_ui();
}

fs::path const & rplay_window::get_media(int sel_idx) const
{
	int media_idx = sel_idx;
	if (_filtered)
	{
		assert(sel_idx < (int)_filtered_lookup.size());
		media_idx = _filtered_lookup[sel_idx];
	}

	vector<fs::path> const & media_list = _play.list_media();
	assert(media_idx < (int)media_list.size());
	return media_list[media_idx];
}

int main(int argc, char * argv[])
{
	auto app = Gtk::Application::create("org.gtkmm.example");
	rplay_window w;
	return app->run(w, argc, argv);
}
