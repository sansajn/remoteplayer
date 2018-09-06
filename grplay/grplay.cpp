// Graphics Remote Player Client
#include <mutex>
#include <string>
#include <regex>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/scale.h>
#include "log.hpp"
#include "player_client.hpp"
#include "json.hpp"
#include "fs.hpp"

using std::mutex;
using std::lock_guard;
using std::string;
using std::vector;
using std::ostringstream;
using std::regex_search;
using std::smatch;
using std::regex;
using std::cout;
using boost::lexical_cast;
using Glib::RefPtr;
using Glib::ustring;

void to_min_and_sec(long t, int & min, int & sec);
void parse_title_author_album(string const & media, string & title, string & author,
	string & album);
string format_media(string const & media);

class rplay_window
	: public Gtk::Window
	, public player_client_listener
{
public:
	rplay_window(string const & host, unsigned short port);

private:
	void update_ui();
	void on_queue_button();
	void on_stop_button();
	void on_search();
	void repack_ui();
	void filter_media_library(string const & filter);
	fs::path const & get_media(int sel_idx) const;

	// player_client events, note: called from player_client's thread
	void on_play_progress(string const & media, long position, long duration) override;
	void on_playlist_change(size_t playlist_id, std::vector<std::string> const & items) override;

	static int update_cb(gpointer user_data);

	player_client _play;
	vector<int> _filtered_lookup;
	bool _filtered;

	// position, duration, media
	string _media;
	long _position;
	long _duration;
	std::chrono::high_resolution_clock::time_point _last_progress_update;
	std::vector<std::string> _playlist;
	size_t _playlist_id;
	std::mutex _player_data_locker;

	size_t _last_used_playlist_id;

	// gui
	Gtk::Box _vbox;
	Gtk::Label _player_media;
	Gtk::Box _progress_hbox;
	Gtk::Label _player_position;
	Gtk::Scale _player_progress;
	RefPtr<Gtk::Adjustment> _progress_adj;
	Gtk::Label _player_duration;
	Gtk::ScrolledWindow _playlist_scroll;
	Gtk::ListViewText _playlist_view;
	Gtk::SearchEntry _search;
	Gtk::ScrolledWindow _filtered_scroll;
	Gtk::ListViewText _filtered_media_list_view;
	Gtk::ScrolledWindow _scroll;
	Gtk::ListViewText _media_list_view;
	Gtk::ButtonBox _button_box;
	Gtk::Button _queue_button;
//	Gtk::Button _pause_button;
	Gtk::Button _stop_button;
};

int rplay_window::update_cb(gpointer user_data)
{
	rplay_window * rplay = (rplay_window *)user_data;
	rplay->update_ui();
	return 1;
}

rplay_window::rplay_window(string const & host, unsigned short port)
	: _filtered{false}
	, _playlist_id{0}
	, _last_used_playlist_id{0}
	,  _vbox{Gtk::Orientation::ORIENTATION_VERTICAL}
	, _playlist_view{1}
	, _filtered_media_list_view{1}
	, _media_list_view{1}
{
	_play.register_listener(this);

	LOG(info) << "connecting to server ...";
	_play.connect(host, port);  // TODO: blocking
	LOG(info) << "connected";

	set_title("Remote Player Client");
	set_default_size(600, 700);

//	_vbox.set_margin(6);
	add(_vbox);

	_player_media.set_text("waiting ...");

	_progress_adj = Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01);
	_player_progress.set_digits(2);
	_player_progress.set_draw_value(false);
	_player_progress.set_adjustment(_progress_adj);

	_player_position.set_text("0:00");
	_player_duration.set_text("00:00");

	_progress_hbox.add(_player_position);
	_progress_hbox.pack_start(_player_progress, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_progress_hbox.add(_player_duration);

	_playlist_scroll.set_size_request(-1, 150);
	_playlist_scroll.add(_playlist_view);
	_playlist_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);
	_playlist_scroll.set_min_content_height(30);
	_playlist_scroll.set_max_content_height(80);

	_search.set_placeholder_text("<Enter search terms there>");
	_search.signal_changed().connect(sigc::mem_fun(*this, &rplay_window::on_search));

	_filtered_scroll.add(_filtered_media_list_view);
	_filtered_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

	_scroll.add(_media_list_view);
	_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

//	_button_box.set_margin(5);
	_button_box.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);
	_button_box.pack_start(_queue_button, Gtk::PackOptions::PACK_SHRINK);
//	_button_box.pack_start(_pause_button, Gtk::PackOptions::PACK_SHRINK);
	_button_box.pack_start(_stop_button, Gtk::PackOptions::PACK_SHRINK);
	_queue_button.signal_clicked().connect(sigc::mem_fun(*this, &rplay_window::on_queue_button));
	_stop_button.signal_clicked().connect(sigc::mem_fun(*this, &rplay_window::on_stop_button));

	_playlist_view.set_column_title(0, "Playlist:");
	_filtered_media_list_view.set_column_title(0, "Media");
	_media_list_view.set_column_title(0, "Library");

	for (fs::path const & media : _play.list_media())
		_media_list_view.append(media.string());

	_queue_button.set_image_from_icon_name("media-playback-start");
//	_pause_button.set_image_from_icon_name("media-playback-pause");
	_stop_button.set_image_from_icon_name("media-playback-stop");

	// pack
	_vbox.pack_start(_player_media, Gtk::PackOptions::PACK_SHRINK);
	_vbox.pack_start(_progress_hbox, Gtk::PackOptions::PACK_SHRINK);
	_vbox.pack_start(_playlist_scroll, Gtk::PackOptions::PACK_SHRINK);
	_vbox.pack_start(_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_search, Gtk::PackOptions::PACK_SHRINK);
	_vbox.pack_start(_button_box, Gtk::PackOptions::PACK_SHRINK);

	show_all();

	gdk_threads_add_timeout(1000, update_cb, this);  // update ui every 1s
}

void to_min_and_sec(long t, int & min, int & sec)
{
	int tmp_sec = t / 1000000000;
	min = tmp_sec / 60;
	sec = tmp_sec % 60;
}

string format_position(long pos)
{
	int min, sec;
	to_min_and_sec(pos, min, sec);

	ostringstream sout;
	sout << min << ":" << std::setfill('0') << std::setw(2) << sec;

	return sout.str();
}

void parse_title_author_album(string const & media, string & title, string & author,
	string & album)
{
	// /home/music/ANNA - Rave On Snow 2017 (BE-AT.TV)-7LNsnWbLcFU.opus

	fs::path p{media};
	title = p.stem().string();
	author = "";
	album = "";

	// TODO: remove hash
}

string format_media(string const & media)
{
	string title, author, album;
	parse_title_author_album(media, title, author, album);

	string result = title;
	if (!author.empty())
		result += "\n" + author;
	if (!album.empty())
		result += "\n" + album;

	return result;
}

void rplay_window::update_ui()
{
	lock_guard<mutex> lock{_player_data_locker};  // TODO: do not lock whole function

	if (_media.empty())
		return;

	// playlist
	if (_playlist_id != _last_used_playlist_id)
	{
		_playlist_view.clear_items();
		for (string const & media : _playlist)
			_playlist_view.append(media);

		_last_used_playlist_id = _playlist_id;
	}

	// media
	_player_media.set_text(format_media(_media));

	// position
	auto elapsed = std::chrono::high_resolution_clock::now() - _last_progress_update;
	long position = _position + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
	_player_position.set_text(format_position(position));

	// scale
	_progress_adj->set_value((double)_position/(double)_duration);

	// duration
	_player_duration.set_text(format_position(_duration));
}

void rplay_window::repack_ui()
{
	if (_filtered)
	{
		_vbox.remove(_scroll);
		_vbox.pack_start(_filtered_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_vbox.reorder_child(_filtered_scroll, 3);
	}
	else
	{
		_vbox.remove(_filtered_scroll);
		_vbox.pack_start(_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_vbox.reorder_child(_scroll, 3);
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

void rplay_window::on_stop_button()
{
	_play.stop();
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

void rplay_window::on_play_progress(string const & media, long position, long duration)
{
	lock_guard<mutex> lock{_player_data_locker};
	_position = position;
	_duration = duration;
	_media = media;
	_last_progress_update = std::chrono::high_resolution_clock::now();
}

void rplay_window::on_playlist_change(size_t playlist_id, vector<string> const & items)
{
	lock_guard<mutex> lock{_player_data_locker};
	_playlist_id = playlist_id;
	_playlist = items;
}

int main(int argc, char * argv[])
{
	// config
	string const host = argc > 1 ? argv[1] : "localhost";
	unsigned short const port = argc > 2 ? lexical_cast<unsigned short>(argv[2]) : 13333;

	auto app = Gtk::Application::create("org.gtkmm.example");

	rplay_window w{host, port};

//	return app->run(w, argc, argv);
	return app->run(w, 0, nullptr);
}
