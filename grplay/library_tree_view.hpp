#pragma once
#include <string>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

class library_tree_view : public Gtk::TreeView
{
public:
	struct columns_type : public Gtk::TreeModel::ColumnRecord
	{
		columns_type() {add(name); add(_media_id);}
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<std::string> _media_id;
	};

	library_tree_view();

	columns_type columns;

	void insert(std::string const & media);
	void clear();
	size_t size() const;
	void expand_smart();
	Gtk::TreeStore & store();  // TODO: better name

private:
	Glib::RefPtr<Gtk::TreeStore> _store;
	size_t _item_count;
};
