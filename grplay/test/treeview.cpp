#include <algorithm>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <gtkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/application.h>

using std::find_if;
using std::string;
using std::cout;
using Glib::RefPtr;
using Glib::ustring;
namespace fs = boost::filesystem;


inline string home_dir()
{
	return getenv("HOME");
}


class library_tree_view : public Gtk::TreeView
{
public:
	struct model_columns : public Gtk::TreeModel::ColumnRecord
	{
		model_columns() {add(name);}
		Gtk::TreeModelColumn<ustring> name;
	};

	library_tree_view();

	model_columns columns;

	void insert(string const & item);

private:
	RefPtr<Gtk::TreeStore> _store;
};

library_tree_view::library_tree_view()
	: _store{Gtk::TreeStore::create(columns)}
{
	set_model(_store);
	append_column("Name", columns.name);
}

void library_tree_view::insert(string const & item)
{
	Gtk::TreeStore::Children rows = _store->children();
	for (fs::path const & item_elem : fs::path{item})
	{
		Gtk::TreeStore::iterator it = find_if(rows.begin(), rows.end(),
			[this, item_elem](Gtk::TreeRow const & row) {
				return row[columns.name] == item_elem.string();});

		if (it != rows.end())
			rows = it->children();
		else
		{
			Gtk::TreeRow row = *_store->append(rows);
			row[columns.name] = item_elem.string();
			rows = row.children();
		}
	}
}



class example_window : public Gtk::Window
{
public:
	example_window();

private:
	void on_button_quit();
	void on_treeview_row_activated(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column);

	// columns_type
	struct model_columns : public Gtk::TreeModel::ColumnRecord
	{
		model_columns() {add(column_name);}
		Gtk::TreeModelColumn<ustring> column_name;
	};

	Gtk::Box _vbox;
	Gtk::ScrolledWindow _scrolled;
	RefPtr<Gtk::TreeStore> _model;
	Gtk::ButtonBox _button_box;
	Gtk::Button _quit;
	library_tree_view _view;
};

example_window::example_window()
	: _vbox{Gtk::ORIENTATION_VERTICAL}
	, _quit{"Quit"}
{
	set_title("Gtk::TreeView (TreeStore) example");
	set_border_width(5);
	set_default_size(400, 200);

	add(_vbox);

	_scrolled.add(_view);

	_vbox.pack_start(_scrolled);
	_vbox.pack_start(_button_box, Gtk::PACK_SHRINK);

	_button_box.pack_start(_quit, Gtk::PACK_SHRINK);
	_button_box.set_border_width(5);
	_button_box.set_layout(Gtk::BUTTONBOX_END);
	_quit.signal_clicked().connect(sigc::mem_fun(*this, &example_window::on_button_quit));

	for (fs::path p : fs::directory_iterator{home_dir() + "/Music"})
		_view.insert(p.string());

	_view.signal_row_activated().connect(sigc::mem_fun(*this, &example_window::on_treeview_row_activated));

	show_all_children();
}

void example_window::on_button_quit()
{
	hide();
}

void example_window::on_treeview_row_activated(Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * column)
{
	Gtk::TreeModel::iterator it = _model->get_iter(path);
	if (it)
	{
		Gtk::TreeModel::Row row = *it;
		cout << "Row activated: Name=" << row[_view.columns.name] << std::endl;
	}
}

int main(int argc, char * argv[])
{
	auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
	example_window w;
	return app->run(w);
}
