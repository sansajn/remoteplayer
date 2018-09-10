#include <algorithm>
#include "fs.hpp"
#include "library_tree_view.hpp"

using std::find_if;
using std::string;


library_tree_view::library_tree_view()
	: _store{Gtk::TreeStore::create(columns)}
	, _item_count{0}
{
	set_model(_store);
	append_column("Library:", columns.name);
}

void library_tree_view::insert(string const & media)
{
	Gtk::TreeRow row;
	Gtk::TreeStore::Children rows = _store->children();
	for (fs::path const & item_elem : fs::path{media})
	{
		Gtk::TreeStore::iterator it = find_if(rows.begin(), rows.end(),
			[this, item_elem](Gtk::TreeRow const & row) {
				return row[columns.name] == item_elem.string();});

		if (it != rows.end())
			rows = it->children();
		else
		{
			row = *_store->append(rows);
			row[columns.name] = item_elem.string();
			rows = row.children();
		}
	}

	if (row)  // new item inserted
	{
		row[columns._media_id] = media;
		++_item_count;
	}
}

void library_tree_view::clear()
{
	_store->clear();
	_item_count = 0;
}

size_t library_tree_view::size() const
{
	return _item_count;
}
