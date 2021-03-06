#include "pool_browser_part.hpp"
#include "pool/pool.hpp"
#include "widgets/tag_entry.hpp"
#include <set>

namespace horizon {
PoolBrowserPart::PoolBrowserPart(Pool *p, const UUID &uu) : PoolBrowser(p), entity_uuid(uu)
{
    construct();
    MPN_entry = create_search_entry("MPN");
    manufacturer_entry = create_search_entry("Manufacturer");
    tag_entry = create_tag_entry("Tags");

    install_pool_item_source_tooltip();
    search();
}

void PoolBrowserPart::set_MPN(const std::string &s)
{
    MPN_entry->set_text(s);
    search();
}

void PoolBrowserPart::set_entity_uuid(const UUID &uu)
{
    entity_uuid = uu;
    search();
}

Glib::RefPtr<Gtk::ListStore> PoolBrowserPart::create_list_store()
{
    return Gtk::ListStore::create(list_columns);
}

void PoolBrowserPart::create_columns()
{
    {
        auto col = append_column_with_item_source_cr("MPN", list_columns.MPN, Pango::ELLIPSIZE_END);
        col->set_resizable(true);
        col->set_expand(true);
        col->set_min_width(200);
    }
    {
        auto col = append_column("Manufacturer", list_columns.manufacturer, Pango::ELLIPSIZE_END);
        col->set_resizable(true);
        col->set_min_width(200);
    }
    {
        auto col = append_column("Description", list_columns.description, Pango::ELLIPSIZE_END);
        col->set_resizable(true);
        col->set_min_width(100);
    }
    {
        auto col = append_column("Package", list_columns.package, Pango::ELLIPSIZE_END);
        col->set_resizable(true);
        col->set_min_width(100);
    }
    {
        auto col = append_column("Tags", list_columns.tags, Pango::ELLIPSIZE_END);
        col->set_resizable(true);
        col->set_min_width(100);
    }
    path_column = append_column("Path", list_columns.path, Pango::ELLIPSIZE_START);
}

void PoolBrowserPart::add_sort_controller_columns()
{
    sort_controller->add_column(0, "parts.MPN");
    sort_controller->add_column(1, "parts.manufacturer");
    sort_controller->add_column(2, "packages.name");
}

void PoolBrowserPart::search()
{
    auto selected_uuid = get_selected();
    treeview->unset_model();
    store->clear();
    Gtk::TreeModel::Row row;
    std::string MPN_search = MPN_entry->get_text();
    std::string manufacturer_search = manufacturer_entry->get_text();

    auto tags = tag_entry->get_tags();
    std::stringstream query;
    if (tags.size() == 0) {
        query << "SELECT parts.uuid, parts.MPN, parts.manufacturer, "
                 "packages.name, tags_view.tags, parts.filename, "
                 "parts.description, parts.pool_uuid, parts.overridden "
                 "FROM parts LEFT JOIN tags_view ON "
                 "tags_view.uuid = parts.uuid AND tags_view.type = 'part' "
                 "LEFT JOIN packages ON packages.uuid = parts.package ";
    }
    else {
        query << "SELECT parts.uuid, parts.MPN, parts.manufacturer, "
                 "packages.name, tags_view.tags, parts.filename, "
                 "parts.description, parts.pool_uuid, parts.overridden FROM parts "
                 "LEFT JOIN tags_view ON tags_view.uuid = parts.uuid AND tags_view.type = 'part' "
                 "LEFT JOIN packages ON packages.uuid = parts.package "
                 "INNER JOIN (SELECT uuid FROM tags WHERE tags.tag IN (";
        int i = 0;
        for (const auto &it : tags) {
            (void)sizeof it;
            query << "$tag" << i << ", ";
            i++;
        }
        query << "'') AND tags.type = 'part' "
                 "GROUP by tags.uuid HAVING count(*) >= $ntags) as x ON x.uuid = parts.uuid ";
    }
    query << "WHERE parts.MPN LIKE $mpn "
             "AND parts.manufacturer LIKE $manufacturer "
             "AND (parts.entity=$entity or $entity_all) ";
    query << sort_controller->get_order_by();
    std::cout << query.str() << std::endl;
    SQLite::Query q(pool->db, query.str());
    q.bind("$mpn", "%" + MPN_search + "%");
    q.bind("$manufacturer", "%" + manufacturer_search + "%");
    q.bind("$entity", entity_uuid);
    q.bind("$entity_all", entity_uuid == UUID());
    int i = 0;
    for (const auto &it : tags) {
        q.bind(("$tag" + std::to_string(i)).c_str(), it);
        i++;
    }
    if (tags.size())
        q.bind("$ntags", tags.size());

    if (show_none) {
        row = *(store->append());
        row[list_columns.uuid] = UUID();
        row[list_columns.MPN] = "none";
        row[list_columns.manufacturer] = "none";
        row[list_columns.package] = "none";
    }

    while (q.step()) {
        row = *(store->append());
        row[list_columns.uuid] = q.get<std::string>(0);
        row[list_columns.MPN] = q.get<std::string>(1);
        row[list_columns.manufacturer] = q.get<std::string>(2);
        row[list_columns.package] = q.get<std::string>(3);
        row[list_columns.tags] = q.get<std::string>(4);
        row[list_columns.path] = q.get<std::string>(5);
        row[list_columns.description] = q.get<std::string>(6);
        row[list_columns.source] = pool_item_source_from_db(q.get<std::string>(7), q.get<int>(8));
    }
    /*
    SQLite::Query q2(pool->db, "EXPLAIN QUERY PLAN " + query.str());
    while (q2.step()) {
        std::cout << q2.get<int>(0) << "|" << q2.get<int>(1) << "|" << q2.get<int>(2) << "|" << q2.get<std::string>(3)
                  << std::endl;
    }
    */

    treeview->set_model(store);
    select_uuid(selected_uuid);
    scroll_to_selection();
}

UUID PoolBrowserPart::uuid_from_row(const Gtk::TreeModel::Row &row)
{
    return row[list_columns.uuid];
}
PoolBrowser::PoolItemSource PoolBrowserPart::pool_item_source_from_row(const Gtk::TreeModel::Row &row)
{
    return row[list_columns.source];
}
} // namespace horizon
