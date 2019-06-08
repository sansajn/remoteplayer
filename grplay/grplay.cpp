// Graphics Remote Player Client
#include <algorithm>
#include <mutex>
#include <atomic>
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
#include <gtkmm/paned.h>
#include "rplib/log.hpp"
#include "player_client.hpp"
#include "json.hpp"
#include "fs.hpp"
#include "playback_ui.hpp"
#include "playlist_ui.hpp"
#include "library_ui.hpp"

using std::mutex;
using std::lock_guard;
using std::atomic_bool;
using std::string;
using std::vector;
using std::ostringstream;
using std::regex_search;
using std::smatch;
using std::regex;
using std::regex_error;
using std::cout;
using std::min;
using std::sort;
using std::advance;
using boost::lexical_cast;
using Glib::RefPtr;
using Glib::ustring;

void to_min_and_sec(long t, int & min, int & sec);
bool parse_file_name(string const & name, string & author, string & title);
bool remove_ext(string & name);
string format_media(string const & media);
static void weakly_directory_first_sort(vector<string> & files);


class grplay_window
	: public Gtk::Window
	, public player_client_listener
	, public playlist_event_listener
	, public library_event_listener
{
public:
	grplay_window(string const & host, unsigned short port);

private:
	void update_ui();
	void on_prev_button();
	void on_play_button();
	void on_stop_button();
	void on_next_button();
	void on_playlist_up_button();
	void on_playlist_down_button();
	void on_playlist_clear_all_button();
	void on_playlist_add_button();
	void on_search();
	bool on_seek(Gtk::ScrollType scroll, double value);
	void on_volume_change();
	void repack_ui();
	void filter_media_library(string const & filter);
	string get_media(int sel_idx) const;  //!< \note it will take a lock for

	// player_client events, note: called from player_client's thread
	void on_play_progress(long position, long duration, size_t playlist_id,
		size_t media_idx, playback_state_e playback_state, int playlist_mode_list) override;
	void on_playlist_change(size_t playlist_id, std::vector<std::string> const & items) override;
	void on_list_media(std::vector<std::string> const & items) override;
	void on_volume(int val) override;
	void on_stop() override;

	// playlist events
	void on_playlist_play_item(size_t idx) override;
	void on_playlist_remove_item(size_t idx) override;
	void on_playlist_shuffle(bool shuffle) override;

	// library events
	void on_library_bed_time(bool bed_time) override;

	static int update_cb(gpointer user_data);

	rplay_client _play;
	vector<size_t> _filtered_lookup;
	bool _filtered;

	// position, duration, media
	string _media;
	size_t _media_idx;
	playback_state_e _playback_state;
	long _position;
	long _duration;
	std::chrono::high_resolution_clock::time_point _last_progress_update;
	std::vector<std::string> _library;
	std::vector<std::string> _playlist;
	size_t _playlist_id;
	bool _seek_position_lock;
	bool _shuffle, _bed_time;
	int _serv_volume;
	mutable std::mutex _player_data_locker;

	size_t _last_used_playlist_id;
	atomic_bool _playback_stoped;
	atomic_bool _highlight_media_in_playlist;

	playback_ui _plb_ui;
	playlist_ui _ply_ui;
	library_ui _lib_ui;

	Gtk::VPaned _playlist_library_paned;  // move down
};

int grplay_window::update_cb(gpointer user_data)
{
	grplay_window * rplay = (grplay_window *)user_data;
	rplay->update_ui();
	return 1;
}

grplay_window::grplay_window(string const & host, unsigned short port)
	: _filtered{false}
	, _media_idx{(size_t)-1}
	, _playback_state{playback_state_e::invalid}
	, _position{0}
	, _duration{0}
	, _playlist_id{0}
	, _seek_position_lock{false}
	, _shuffle{false}
	, _bed_time{false}
	, _serv_volume{0}
	, _last_used_playlist_id{0}
	, _playback_stoped{false}
	, _highlight_media_in_playlist{false}
{
	_play.register_listener(this);
	_lib_ui.register_listener(this);

	LOG(info) << "connecting to server ...";
	_play.connect(host, port);
	LOG(info) << "connected";

	set_title("Remote Player Client");
	set_default_size(600, 700);

	add(_plb_ui._vbox);

	_plb_ui._player_media.property_wrap() = true;
	_plb_ui._player_media.property_max_width_chars() = 30;
	_plb_ui._player_media.set_text("waiting ...");

	_plb_ui._progress_adj = Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01);
	_plb_ui._player_progress.set_digits(2);
	_plb_ui._player_progress.set_draw_value(false);
	_plb_ui._player_progress.set_adjustment(_plb_ui._progress_adj);
	_plb_ui._player_progress.signal_change_value().connect(sigc::mem_fun(*this, &grplay_window::on_seek));

	_plb_ui._player_position.set_text("0:00");
	_plb_ui._player_duration.set_text("00:00");

	_plb_ui._progress_hbox.add(_plb_ui._player_position);
	_plb_ui._progress_hbox.pack_start(_plb_ui._player_progress, Gtk::PackOptions::PACK_EXPAND_WIDGET);
	_plb_ui._progress_hbox.add(_plb_ui._player_duration);

	// control bar
	_plb_ui._control_bar_hbox.pack_start(_plb_ui._control_bar_l, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._control_bar_hbox.pack_end(_plb_ui._control_bar_r, Gtk::PackOptions::PACK_SHRINK);

	_plb_ui._control_bar_l.set_layout(Gtk::ButtonBoxStyle::BUTTONBOX_START);

	_plb_ui._prev_button.set_image_from_icon_name("media-skip-backward");
	_plb_ui._prev_button.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_prev_button));
	_plb_ui._play_button.set_image_from_icon_name("media-playback-start");
	_plb_ui._play_button.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_play_button));
	_plb_ui._stop_button.set_image_from_icon_name("media-playback-stop");
	_plb_ui._stop_button.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_stop_button));
	_plb_ui._next_button.set_image_from_icon_name("media-skip-forward");
	_plb_ui._next_button.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_next_button));

	_plb_ui._control_bar_l.pack_start(_plb_ui._prev_button, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._control_bar_l.pack_start(_plb_ui._play_button, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._control_bar_l.pack_start(_plb_ui._stop_button, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._control_bar_l.pack_start(_plb_ui._next_button, Gtk::PackOptions::PACK_SHRINK);

	_plb_ui._volume_adj = Gtk::Adjustment::create(0, 0, 100, 1);
	_plb_ui._volume.set_adjustment(_plb_ui._volume_adj);
	_plb_ui._volume_adj->signal_value_changed().connect(sigc::mem_fun(*this, &grplay_window::on_volume_change));

	_plb_ui._control_bar_r.pack_start(_plb_ui._volume, Gtk::PackOptions::PACK_SHRINK);

	// playlist
	_ply_ui.init();
	_ply_ui._ply_menu.accelerate(*this);
	_ply_ui.register_listener(this);

	_playlist_library_paned.add1(_ply_ui._container);

	// playlist controls
	_ply_ui._up.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_playlist_up_button));
	_ply_ui._down.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_playlist_down_button));
	_ply_ui._clear_all.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_playlist_clear_all_button));

	// library
	_lib_ui.init();
	_playlist_library_paned.add2(_lib_ui._container);

	_lib_ui._search.signal_changed().connect(sigc::mem_fun(*this, &grplay_window::on_search));
	_lib_ui._playlist_add_button.signal_clicked().connect(sigc::mem_fun(*this, &grplay_window::on_playlist_add_button));

	RefPtr<Gtk::TreeSelection> selection = _lib_ui._media_list_view.get_selection();
	assert(selection);
	selection->set_mode(Gtk::SELECTION_MULTIPLE);

	// pack
	_plb_ui._vbox.pack_start(_plb_ui._player_media, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._vbox.pack_start(_plb_ui._progress_hbox, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._vbox.pack_start(_plb_ui._control_bar_hbox, Gtk::PackOptions::PACK_SHRINK);
	_plb_ui._vbox.pack_start(_playlist_library_paned, Gtk::PackOptions::PACK_EXPAND_WIDGET);

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

bool parse_file_name(string const & name, string & author, string & title)
{
	try {
		// {author}@{title}-{hash}.{ext}
		static regex const pat{R"((.+)@(.+)-(.{11})\..+$)"};

		smatch match;
		if (regex_match(name, match, pat))
		{
			author = match[1];
			title = match[2];
			return true;
		}
		else
			return false;
	}
	catch (std::regex_error & e) {
		cout << e.what() << "\n";
	}

	return false;
}

bool remove_ext(string & name)
{
	try {
		// {title}-{hash}.{ext}
		static regex const pat{R"((.+)-.{11}\..+$)"};

		smatch match;
		if (regex_match(name, match, pat))
		{
			name = match[1];
			return true;
		}
		else
		{
			fs::path p{name};
			name = p.stem().string();
			return true;
		}
	}
	catch (std::regex_error & e) {
		cout << e.what() << "\n";
	}

	return false;
}

string format_media(string const & media)
{
	fs::path p{media};
	string author, title;
	if (parse_file_name(p.filename().string()/*p.stem().string()*/, author, title))
	{
		string result;
		result = author;
		result += "@" + title;
		return result;
	}
	else
	{
		// at least try to remove file:// and hash if present
		string result{p.filename().string()};
		remove_ext(result);
		return result;
	}
}

void grplay_window::update_ui()
{
	lock_guard<mutex> lock{_player_data_locker};  // TODO: do not lock whole function

	auto elapsed = std::chrono::high_resolution_clock::now() - _last_progress_update;
	long playback_position = min(
		_position + std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count(),
		_duration);

	bool eos = playback_position == _duration;

	// library
	if (_lib_ui._media_list_view.size() != _library.size())
	{
		_lib_ui._media_list_view.clear();
		for (string const & media : _library)
			_lib_ui._media_list_view.insert(media);

		_lib_ui._media_list_view.expand_smart();
	}

	if (_lib_ui.bed_time() != _bed_time)
		_lib_ui.bed_time(_bed_time);

	// playlist
	if (_playlist_id != _last_used_playlist_id)
	{
		_ply_ui._ply_view.clear_items();
		for (string const & media : _playlist)
			_ply_ui.add(media);

		if (_ply_ui._ply_view.size() > _media_idx
			&& (_playback_state == playback_state_e::playing
				|| _playback_state == playback_state_e::paused))
		{
			_ply_ui.highlight(_media_idx, _playback_state);
		}

		_last_used_playlist_id = _playlist_id;
	}

	// play_pause
	static playback_state_e prev_state = playback_state_e::invalid;
	if (_playback_state != prev_state)
	{
		if (_playback_state == playback_state_e::paused)
			_plb_ui._play_button.set_image_from_icon_name("media-playback-start");
		else if (_playback_state == playback_state_e::playing)
			_plb_ui._play_button.set_image_from_icon_name("media-playback-pause");
		else
			_plb_ui._play_button.set_image_from_icon_name("media-playback-start");

		if (_ply_ui._ply_view.size() > _media_idx)
			_ply_ui.highlight(_media_idx, _playback_state);

		prev_state = _playback_state;
	}

	// volume
	if ((int)_plb_ui._volume_adj->get_value() != _serv_volume)
		_plb_ui._volume_adj->set_value(_serv_volume);

	if (_media.empty())
		return;

	// highlight played media in playlist
	if (_highlight_media_in_playlist)
	{
		Gtk::TreeModel::Children playlist_items = _ply_ui._ply_view.get_model()->children();
		if (playlist_items.size() > _media_idx)
		{
			auto it = playlist_items.begin();
			advance(it, _media_idx);
			_ply_ui._ply_view.set_cursor(Gtk::TreeModel::Path{it});  // TODO: better way to set cursor ?

			_ply_ui.highlight(_media_idx, _playback_state);
		}
		_highlight_media_in_playlist = false;
	}

	_ply_ui.shuffle(_shuffle);

	// media
	_plb_ui._player_media.set_text(format_media(_media));

	if (_playback_stoped)
	{
		_position = _duration = 0;
		_playback_state = playback_state_e::invalid;
		_playback_stoped = false;
	}

	// position
	if (_position == 0 && _duration  == 0)
	{
		_plb_ui._player_position.set_text("0:00");
		_plb_ui._player_duration.set_text("0:00");
		_plb_ui._progress_adj->set_value(0.0);
	}
	else if (!_seek_position_lock && _playback_state == playback_state_e::playing)
	{
		_plb_ui._player_position.set_text(format_position(playback_position));

		// scale
		_plb_ui._progress_adj->set_value((double)playback_position/(double)_duration);
	}
	else
	{
		double pos = _plb_ui._player_progress.get_value();  // new seeked position
		_plb_ui._player_position.set_text(format_position(pos * _duration));
	}

	// duration
	_plb_ui._player_duration.set_text(format_position(_duration));

	if (eos)
	{
		_playback_state = playback_state_e::invalid;
	}
}

void grplay_window::repack_ui()
{
	if (_filtered)
	{
		_lib_ui._container.remove(_lib_ui._scroll);
		_lib_ui._container.pack_start(_lib_ui._filtered_scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_lib_ui._container.reorder_child(_lib_ui._filtered_scroll, 0);
	}
	else
	{
		_lib_ui._container.remove(_lib_ui._filtered_scroll);
		_lib_ui._container.pack_start(_lib_ui._scroll, Gtk::PackOptions::PACK_EXPAND_WIDGET);
		_lib_ui._container.reorder_child(_lib_ui._scroll, 0);
	}

	show_all();
}

bool grplay_window::on_seek(Gtk::ScrollType scroll, double value)
{
	lock_guard<mutex> lock{_player_data_locker};

	assert(_duration > 0.0);
	double pos = value * _duration;
	_seek_position_lock = true;

	_play.seek(long(pos), _media);

	return true;
}

void grplay_window::on_volume_change()
{
	_play.volume((int)_plb_ui._volume_adj->get_value());
}

void grplay_window::on_prev_button()
{
	lock_guard<mutex> locker{_player_data_locker};
	if (_media_idx > 0)
		_play.play(_playlist_id, _media_idx-1);
	else
		_play.play(_playlist_id, 0);
}

void grplay_window::on_play_button()
{
	lock_guard<mutex> locker{_player_data_locker};
	if (_playback_state == playback_state_e::invalid)
		_play.play(_playlist_id, 0);  // play from playlist beginning
	else  // pause/resume
		_play.pause();
}

void grplay_window::on_stop_button()
{
	_play.stop();
}

void grplay_window::on_next_button()
{
	lock_guard<mutex> locker{_player_data_locker};
	if (_media_idx+1 < _playlist.size())
		_play.play(_playlist_id, _media_idx+1);
}

void grplay_window::on_playlist_up_button()
{
	lock_guard<mutex> locker{_player_data_locker};

	size_t idx = (size_t)_ply_ui.selected();

	if (idx > 0)
		_play.playlist_move_item(_playlist_id, idx, idx-1);
}

void grplay_window::on_playlist_down_button()
{
	lock_guard<mutex> locker{_player_data_locker};

	size_t idx = (size_t)_ply_ui.selected();

	if (idx < _playlist.size()-1)
		_play.playlist_move_item(_playlist_id, idx, idx+1);
}

void grplay_window::on_playlist_clear_all_button()
{
	lock_guard<mutex> locker{_player_data_locker};

	vector<size_t> items;
	for (size_t i = 0; i < _playlist.size(); ++i)
		items.push_back(i);

	_play.playlist_remove(_playlist_id, items);
}

void grplay_window::on_playlist_add_button()
{
	if (!_filtered)  // from media library
	{
		RefPtr<Gtk::TreeSelection> selection = _lib_ui._media_list_view.get_selection();
		if (selection)
		{
			vector<fs::path> items;
			for (Gtk::TreeModel::Path const & p : selection->get_selected_rows())
			{
				Gtk::TreeModel::iterator it = _lib_ui._media_list_view.store().get_iter(p);
				assert(it);
				Gtk::TreeModel::Row row = *it;
				string media = row[_lib_ui._media_list_view.columns._media_id];
				items.push_back(media);
			}

			if (!items.empty())
				_play.playlist_add(items);
		}
	}
	else  // from filtered media
	{
		Gtk::ListViewText::SelectionList selection = _lib_ui._filtered_media_list_view.get_selected();
		_play.playlist_add({get_media(selection[0])});
	}
}

void grplay_window::on_playlist_play_item(size_t idx)
{
	lock_guard<mutex> locker{_player_data_locker};
	_play.play(_playlist_id, idx);
}

void grplay_window::on_playlist_remove_item(size_t idx)
{
	lock_guard<mutex> locker{_player_data_locker};
	_play.playlist_remove(_playlist_id, vector<size_t>{idx});
}

void grplay_window::on_playlist_shuffle(bool shuffle)
{
	_play.playlist_shuffle(shuffle);
}

void grplay_window::on_library_bed_time(bool bed_time)
{
	_play.bed_time(bed_time);
}

void grplay_window::on_search()
{
	ustring filter = _lib_ui._search.get_text();
	filter_media_library(filter);
}

void grplay_window::filter_media_library(string const & filter)
{
	lock_guard<mutex> lock{_player_data_locker};

	if (filter.empty())
	{
		_filtered = false;
		repack_ui();
		return;
	}

	try { // search only if we can create regex expression
		regex pat{filter, std::regex_constants::icase};

		_filtered_lookup.clear();
		for (size_t i = 0; i < _library.size(); ++i)
		{
			fs::path const & media = _library[i];
			smatch match;
			if (regex_search(media.native(), match, pat))
				_filtered_lookup.push_back(i);
		}

		_lib_ui._filtered_media_list_view.clear_items();
		for (size_t idx : _filtered_lookup)
			_lib_ui._filtered_media_list_view.append(_library[idx]);

		_filtered = true;
		repack_ui();
	}
	catch (regex_error &) {
		// do nothing
	}
}

string grplay_window::get_media(int sel_idx) const
{
	size_t media_idx = (size_t)sel_idx;
	if (_filtered)
	{
		assert(sel_idx < (int)_filtered_lookup.size());
		media_idx = _filtered_lookup[sel_idx];
	}

	lock_guard<mutex> lock{_player_data_locker};
	assert(media_idx < _library.size());
	return _library[media_idx];
}

void grplay_window::on_play_progress(long position, long duration, size_t playlist_id,
	size_t media_idx, playback_state_e playback_state, int playlist_mode_list)
{
	lock_guard<mutex> lock{_player_data_locker};
	_position = position;
	_duration = duration;
	_playback_state = playback_state;
	_last_progress_update = std::chrono::high_resolution_clock::now();
	_seek_position_lock = false;
	_shuffle = playlist_mode_list & playlist_mode_e::playlist_mode_shuffle;
	_bed_time = playlist_mode_list & playlist_mode_e::playlist_mode_bed_time;

	bool new_media = _media_idx != media_idx;
	_media_idx = media_idx;

	if (_playlist_id == playlist_id)
	{
		_media = _playlist[_media_idx];

		if (new_media)
			_highlight_media_in_playlist = true;
	}
}

void grplay_window::on_playlist_change(size_t playlist_id, vector<string> const & items)
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


void grplay_window::on_list_media(vector<string> const & items)
{
	lock_guard<mutex> lock{_player_data_locker};
	_library = items;
	weakly_directory_first_sort(_library);
}

void grplay_window::on_volume(int val)
{
	lock_guard<mutex> lock{_player_data_locker};
	_serv_volume = val;
}

void grplay_window::on_stop()
{
	_playback_stoped = true;
	_highlight_media_in_playlist = true;

	lock_guard<mutex> lock{_player_data_locker};
	_playback_state = playback_state_e::invalid;
}

int main(int argc, char * argv[])
{
	// config
	string const host = argc > 1 ? argv[1] : "localhost";
	unsigned short const port = argc > 2 ? lexical_cast<unsigned short>(argv[2]) : 13333;

	rpl::log_to_console();

	auto app = Gtk::Application::create("org.gtkmm.example", Gio::APPLICATION_NON_UNIQUE);
	assert(app);

	grplay_window w{host, port};

	return app->run(w, 0, nullptr);
}
