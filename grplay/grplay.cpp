// Graphics Remote Player Client
#include <algorithm>
#include <mutex>
#include <string>
#include <regex>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <iterator>
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
#include "library_tree_view.hpp"

using std::mutex;
using std::lock_guard;
using std::string;
using std::vector;
using std::ostringstream;
using std::regex_search;
using std::smatch;
using std::regex;
using std::cout;
using std::min;
using std::sort;
using std::advance;
using boost::lexical_cast;
using Glib::RefPtr;
using Glib::ustring;

void to_min_and_sec(long t, int & min, int & sec);
void parse_title_author_album(string const & media, string & title, string & author,
	string & album);
string format_media(string const & media);
static void weakly_directory_first_sort(vector<string> & files);


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
	bool on_seek(Gtk::ScrollType scroll, double value);
	void filter_media_library(string const & filter);
	std::string get_media(int sel_idx) const;  //!< \note it will take a lock for

	// player_client events, note: called from player_client's thread
	void on_play_progress(string const & media, long position, long duration, size_t playlist_idx) override;
	void on_playlist_change(size_t playlist_id, std::vector<std::string> const & items) override;
	void on_list_media(std::vector<std::string> const & items) override;

	static int update_cb(gpointer user_data);

	player_client _play;
	vector<size_t> _filtered_lookup;
	bool _filtered;

	// position, duration, media
	string _media;
	size_t _playlist_idx;
	long _position;
	long _duration;
	std::chrono::high_resolution_clock::time_point _last_progress_update;
	std::vector<std::string> _library;
	std::vector<std::string> _playlist;
	size_t _playlist_id;
	bool _seek_position_lock;
	mutable std::mutex _player_data_locker;

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
	library_tree_view _media_list_view;
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
	, _playlist_idx{0}
	, _position{0}
	, _duration{0}
	, _playlist_id{0}
	, _last_used_playlist_id{0}
	, _seek_position_lock{false}
	, _vbox{Gtk::Orientation::ORIENTATION_VERTICAL}
	, _playlist_view{1}
	, _filtered_media_list_view{1}
{
	_play.register_listener(this);

	LOG(info) << "connecting to server ...";
	_play.connect(host, port);
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
	_player_progress.signal_change_value().connect(sigc::mem_fun(*this, &rplay_window::on_seek));

	_player_position.set_text("0:00");
	_player_duration.set_text("00:00");

	_progress_hbox.add(_player_position);
	_progress_hbox.pack_start(_player_progress, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_progress_hbox.add(_player_duration);

	_playlist_scroll.set_size_request(-1, 150);
	_playlist_scroll.add(_playlist_view);
	_playlist_scroll.set_policy(Gtk::PolicyType::POLICY_AUTOMATIC, Gtk::PolicyType::POLICY_AUTOMATIC);

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
	_filtered_media_list_view.set_column_title(0, "Filtered Media:");

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

	// library
	if (_media_list_view.size() != _library.size())
	{
		_media_list_view.clear();
		for (string const & media : _library)
			_media_list_view.insert(media);
	}

	// playlist
	if (_playlist_id != _last_used_playlist_id)
	{
		_playlist_view.clear_items();
		for (string const & media : _playlist)
			_playlist_view.append(media);

		_last_used_playlist_id = _playlist_id;
	}

	if (_media.empty())
		return;

	// highlight played media in playlist
	Gtk::TreeModel::Children playlist_items = _playlist_view.get_model()->children();
	if (playlist_items.size() > _playlist_idx)
	{
		auto it = playlist_items.begin();
		advance(it, _playlist_idx);
		_playlist_view.set_cursor(Gtk::TreeModel::Path{it});  // TODO: better way to set cursor ?
	}

	// media
	_player_media.set_text(format_media(_media));

	// position
	if (!_seek_position_lock)
	{
		auto elapsed = std::chrono::high_resolution_clock::now() - _last_progress_update;
		long position = min(
			_position + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count(),
			_duration);
		_player_position.set_text(format_position(position));

		// scale
		_progress_adj->set_value((double)position/(double)_duration);
	}
	else
	{
		double pos = _player_progress.get_value();  // new seeked position
		_player_position.set_text(format_position(pos * _duration));
	}

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

bool rplay_window::on_seek(Gtk::ScrollType scroll, double value)
{
	lock_guard<mutex> lock{_player_data_locker};

	assert(_duration > 0.0);
	double pos = value * _duration;
	_seek_position_lock = true;

	_play.seek(long(pos), _media);

	LOG(debug) << "on_seek(pos=" << pos << "ns)";
	return true;
}

void rplay_window::on_queue_button()
{
	if (!_filtered)  // from media library
	{
		RefPtr<Gtk::TreeSelection> selection = _media_list_view.get_selection();
		if (selection)
		{
			Gtk::TreeModel::iterator it = selection->get_selected();
			if (it)
			{
				Gtk::TreeModel::Row row = *it;
				string media = row[_media_list_view.columns._media_id];
				_play.play(media);
			}
		}
	}
	else  // from filtered media
	{
		Gtk::ListViewText::SelectionList selection = _filtered_media_list_view.get_selected();
		_play.play(get_media(selection[0]));
	}
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
	lock_guard<mutex> lock{_player_data_locker};

	if (filter.empty())
	{
		_filtered = false;
		repack_ui();
		return;
	}

	regex pat{filter, std::regex_constants::icase};

	_filtered_lookup.clear();
	for (size_t i = 0; i < _library.size(); ++i)
	{
		fs::path const & media = _library[i];
		smatch match;
		if (regex_search(media.native(), match, pat))
			_filtered_lookup.push_back(i);
	}

	_filtered_media_list_view.clear_items();
	for (size_t idx : _filtered_lookup)
		_filtered_media_list_view.append(_library[idx]);

	_filtered = true;
	repack_ui();
}

string rplay_window::get_media(int sel_idx) const
{
	lock_guard<mutex> lock{_player_data_locker};

	size_t media_idx = (size_t)sel_idx;
	if (_filtered)
	{
		assert(sel_idx < (int)_filtered_lookup.size());
		media_idx = _filtered_lookup[sel_idx];
	}

	assert(media_idx < _library.size());
	return _library[media_idx];
}

void rplay_window::on_play_progress(string const & media, long position, long duration,
	size_t playlist_idx)
{
	lock_guard<mutex> lock{_player_data_locker};
	_media = media;
	_position = position;
	_duration = duration;
	_playlist_idx = playlist_idx;
	_last_progress_update = std::chrono::high_resolution_clock::now();
	_seek_position_lock = false;
}

void rplay_window::on_playlist_change(size_t playlist_id, vector<string> const & items)
{
	lock_guard<mutex> lock{_player_data_locker};
	_playlist_id = playlist_id;
	_playlist = items;
}

bool has_dotdot(fs::path const & p)
{
	static fs::path dotdot{".."};
	for (auto e : p)
		if (e == dotdot)
			return true;
	return false;
}

bool has_dot(fs::path const & p)
{
	static fs::path dot{"."};
	for (auto e : p)
		if (e == dot)
			return true;
	return false;
}

bool sub_directory(fs::path const & a, fs::path const & b)  //!< true if a is b sub-directory
{
	fs::path const rel = relative(a.parent_path(), b.parent_path());
	if (has_dotdot(rel) || has_dot(rel))
		return false;
	else
		return true;
}

void weakly_directory_first_sort(vector<string> & files)
{
	// FIXME: direct call from files cause crash, thre must be something wrong with _library access
	vector<fs::path> patches;
	for (string const & f : files)
		patches.push_back(fs::path{f});

	sort(patches.begin(), patches.end(), [](fs::path const & a, fs::path const & b) {
		if (sub_directory(a, b))
			return true;
		else if (sub_directory(b, a))
			return false;
		else
			return a < b;
	});

	files.clear();
	for (fs::path const & p : patches)
		files.push_back(p.string());
}


void rplay_window::on_list_media(vector<string> const & items)
{
	lock_guard<mutex> lock{_player_data_locker};
	_library = items;
	weakly_directory_first_sort(_library);
}

int main(int argc, char * argv[])
{
	// config
	string const host = argc > 1 ? argv[1] : "localhost";
	unsigned short const port = argc > 2 ? lexical_cast<unsigned short>(argv[2]) : 13333;

	auto app = Gtk::Application::create("org.gtkmm.example");

	rplay_window w{host, port};

	return app->run(w, 0, nullptr);
}
