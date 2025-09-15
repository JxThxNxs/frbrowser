#include "history.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <json-glib/json-glib.h>

#define MAX_HISTORY_ENTRIES 1000

FRHistoryManager* fr_history_manager_new(void) {
    FRHistoryManager *manager = g_malloc0(sizeof(FRHistoryManager));
    manager->entries = NULL;
    manager->max_entries = MAX_HISTORY_ENTRIES;
    
    char *config_dir = fr_browser_get_config_dir();
    fr_browser_ensure_directory(config_dir);
    manager->history_file = g_build_filename(config_dir, "history.json", NULL);
    g_free(config_dir);
    
    return manager;
}

void fr_history_manager_free(FRHistoryManager *manager) {
    if (!manager) return;
    
    g_list_free_full(manager->entries, (GDestroyNotify)fr_history_entry_free);
    g_free(manager->history_file);
    g_free(manager);
}

FRHistoryEntry* fr_history_entry_new(const char *title, const char *url) {
    FRHistoryEntry *entry = g_malloc0(sizeof(FRHistoryEntry));
    entry->title = title ? g_strdup(title) : g_strdup("Untitled");
    entry->url = url ? g_strdup(url) : NULL;
    entry->visited = time(NULL);
    entry->visit_count = 1;
    
    return entry;
}

void fr_history_entry_free(FRHistoryEntry *entry) {
    if (!entry) return;
    
    g_free(entry->title);
    g_free(entry->url);
    g_free(entry);
}

void fr_history_manager_add(FRHistoryManager *manager, const char *title, const char *url) {
    if (!manager || !url) return;
    
    // Check if entry already exists
    for (GList *l = manager->entries; l != NULL; l = l->next) {
        FRHistoryEntry *entry = (FRHistoryEntry*)l->data;
        if (entry && entry->url && strcmp(entry->url, url) == 0) {
            // Update existing entry
            entry->visited = time(NULL);
            entry->visit_count++;
            if (title && strcmp(entry->title, title) != 0) {
                g_free(entry->title);
                entry->title = g_strdup(title);
            }
            fr_history_manager_save(manager);
            return;
        }
    }
    
    // Add new entry
    FRHistoryEntry *entry = fr_history_entry_new(title, url);
    manager->entries = g_list_prepend(manager->entries, entry);
    
    // Limit history size
    if (g_list_length(manager->entries) > manager->max_entries) {
        GList *last = g_list_last(manager->entries);
        if (last) {
            fr_history_entry_free((FRHistoryEntry*)last->data);
            manager->entries = g_list_delete_link(manager->entries, last);
        }
    }
    
    fr_history_manager_save(manager);
}

void fr_history_manager_clear(FRHistoryManager *manager) {
    if (!manager) return;
    
    g_list_free_full(manager->entries, (GDestroyNotify)fr_history_entry_free);
    manager->entries = NULL;
    fr_history_manager_save(manager);
}

GList* fr_history_manager_search(FRHistoryManager *manager, const char *query) {
    if (!manager || !query) return NULL;
    
    GList *results = NULL;
    char *lower_query = g_utf8_strdown(query, -1);
    
    for (GList *l = manager->entries; l != NULL; l = l->next) {
        FRHistoryEntry *entry = (FRHistoryEntry*)l->data;
        if (!entry) continue;
        
        char *lower_title = entry->title ? g_utf8_strdown(entry->title, -1) : NULL;
        char *lower_url = entry->url ? g_utf8_strdown(entry->url, -1) : NULL;
        
        gboolean match = FALSE;
        if (lower_title && strstr(lower_title, lower_query)) {
            match = TRUE;
        } else if (lower_url && strstr(lower_url, lower_query)) {
            match = TRUE;
        }
        
        if (match) {
            results = g_list_append(results, entry);
        }
        
        g_free(lower_title);
        g_free(lower_url);
    }
    
    g_free(lower_query);
    return results;
}

GList* fr_history_manager_get_recent(FRHistoryManager *manager, int count) {
    if (!manager) return NULL;
    
    GList *recent = NULL;
    int added = 0;
    
    for (GList *l = manager->entries; l != NULL && added < count; l = l->next) {
        recent = g_list_append(recent, l->data);
        added++;
    }
    
    return recent;
}

gboolean fr_history_manager_load(FRHistoryManager *manager) {
    if (!manager || !manager->history_file) return FALSE;
    
    if (!g_file_test(manager->history_file, G_FILE_TEST_EXISTS)) {
        return TRUE; // No file exists yet, that's OK
    }
    
    GError *error = NULL;
    char *contents;
    gsize length;
    
    if (!g_file_get_contents(manager->history_file, &contents, &length, &error)) {
        g_error_free(error);
        return FALSE;
    }
    
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, length, &error)) {
        g_free(contents);
        g_object_unref(parser);
        g_error_free(error);
        return FALSE;
    }
    
    JsonNode *root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_ARRAY(root)) {
        g_free(contents);
        g_object_unref(parser);
        return FALSE;
    }
    
    JsonArray *array = json_node_get_array(root);
    guint length_array = json_array_get_length(array);
    
    for (guint i = 0; i < length_array; i++) {
        JsonNode *element = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(element)) continue;
        
        JsonObject *obj = json_node_get_object(element);
        const char *title = json_object_get_string_member(obj, "title");
        const char *url = json_object_get_string_member(obj, "url");
        
        if (url) {
            FRHistoryEntry *entry = fr_history_entry_new(title, url);
            if (json_object_has_member(obj, "visited")) {
                entry->visited = (time_t)json_object_get_int_member(obj, "visited");
            }
            if (json_object_has_member(obj, "visit_count")) {
                entry->visit_count = json_object_get_int_member(obj, "visit_count");
            }
            manager->entries = g_list_append(manager->entries, entry);
        }
    }
    
    g_free(contents);
    g_object_unref(parser);
    return TRUE;
}

gboolean fr_history_manager_save(FRHistoryManager *manager) {
    if (!manager || !manager->history_file) return FALSE;
    
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_array(builder);
    
    for (GList *l = manager->entries; l != NULL; l = l->next) {
        FRHistoryEntry *entry = (FRHistoryEntry*)l->data;
        if (!entry) continue;
        
        json_builder_begin_object(builder);
        
        json_builder_set_member_name(builder, "title");
        json_builder_add_string_value(builder, entry->title ? entry->title : "");
        
        json_builder_set_member_name(builder, "url");
        json_builder_add_string_value(builder, entry->url ? entry->url : "");
        
        json_builder_set_member_name(builder, "visited");
        json_builder_add_int_value(builder, (gint64)entry->visited);
        
        json_builder_set_member_name(builder, "visit_count");
        json_builder_add_int_value(builder, entry->visit_count);
        
        json_builder_end_object(builder);
    }
    
    json_builder_end_array(builder);
    
    JsonGenerator *generator = json_generator_new();
    JsonNode *root = json_builder_get_root(builder);
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    
    char *json_data = json_generator_to_data(generator, NULL);
    
    GError *error = NULL;
    gboolean success = g_file_set_contents(manager->history_file, json_data, -1, &error);
    
    if (error) {
        g_error_free(error);
        success = FALSE;
    }
    
    g_free(json_data);
    json_node_free(root);
    g_object_unref(generator);
    g_object_unref(builder);
    
    return success;
}

GtkWidget* fr_history_create_menu(FRHistoryManager *manager) {
    GtkWidget *menu = gtk_menu_new();
    
    if (!manager) return menu;
    
    GList *recent = fr_history_manager_get_recent(manager, 10);
    
    for (GList *l = recent; l != NULL; l = l->next) {
        FRHistoryEntry *entry = (FRHistoryEntry*)l->data;
        if (!entry) continue;
        
        GtkWidget *item = gtk_menu_item_new_with_label(entry->title);
        g_object_set_data_full(G_OBJECT(item), "history-url", 
                              g_strdup(entry->url), g_free);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    
    if (g_list_length(recent) == 0) {
        GtkWidget *item = gtk_menu_item_new_with_label("No history");
        gtk_widget_set_sensitive(item, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    
    g_list_free(recent);
    gtk_widget_show_all(menu);
    return menu;
}

void fr_history_show_dialog(GtkWindow *parent, FRHistoryManager *manager) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("History",
                                                    parent,
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "Close", GTK_RESPONSE_CLOSE,
                                                    "Clear All", GTK_RESPONSE_REJECT,
                                                    NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Create list store and tree view
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    // Add columns
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view),
                                                -1, "Title", renderer,
                                                "text", 0, NULL);
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view),
                                                -1, "URL", renderer,
                                                "text", 1, NULL);
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view),
                                                -1, "Visited", renderer,
                                                "text", 2, NULL);
    
    // Populate with history entries
    for (GList *l = manager->entries; l != NULL; l = l->next) {
        FRHistoryEntry *entry = (FRHistoryEntry*)l->data;
        if (!entry) continue;
        
        char *time_str = g_strdup(ctime(&entry->visited));
        if (time_str) {
            // Remove newline from ctime
            char *newline = strchr(time_str, '\n');
            if (newline) *newline = '\0';
        }
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, entry->title ? entry->title : "",
                          1, entry->url ? entry->url : "",
                          2, time_str ? time_str : "",
                          -1);
        
        g_free(time_str);
    }
    
    gtk_container_add(GTK_CONTAINER(scrolled), tree_view);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled);
    
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_REJECT) {
        // Clear history
        fr_history_manager_clear(manager);
    }
    
    gtk_widget_destroy(dialog);
}
