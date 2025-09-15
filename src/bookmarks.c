#include "bookmarks.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <json-glib/json-glib.h>

FRBookmarkManager* fr_bookmark_manager_new(void) {
    FRBookmarkManager *manager = g_malloc0(sizeof(FRBookmarkManager));
    manager->bookmarks = NULL;
    
    char *config_dir = fr_browser_get_config_dir();
    fr_browser_ensure_directory(config_dir);
    manager->bookmarks_file = g_build_filename(config_dir, "bookmarks.json", NULL);
    g_free(config_dir);
    
    return manager;
}

void fr_bookmark_manager_free(FRBookmarkManager *manager) {
    if (!manager) return;
    
    g_list_free_full(manager->bookmarks, (GDestroyNotify)fr_bookmark_free);
    g_free(manager->bookmarks_file);
    g_free(manager);
}

FRBookmark* fr_bookmark_new(const char *title, const char *url, const char *folder) {
    FRBookmark *bookmark = g_malloc0(sizeof(FRBookmark));
    bookmark->title = title ? g_strdup(title) : g_strdup("Untitled");
    bookmark->url = url ? g_strdup(url) : NULL;
    bookmark->folder = folder ? g_strdup(folder) : g_strdup("Default");
    bookmark->created = time(NULL);
    
    return bookmark;
}

void fr_bookmark_free(FRBookmark *bookmark) {
    if (!bookmark) return;
    
    g_free(bookmark->title);
    g_free(bookmark->url);
    g_free(bookmark->folder);
    g_free(bookmark);
}

gboolean fr_bookmark_manager_add(FRBookmarkManager *manager, FRBookmark *bookmark) {
    if (!manager || !bookmark) return FALSE;
    
    // Check if bookmark already exists
    if (fr_bookmark_manager_find(manager, bookmark->url)) {
        return FALSE; // Already exists
    }
    
    manager->bookmarks = g_list_append(manager->bookmarks, bookmark);
    return fr_bookmark_manager_save(manager);
}

gboolean fr_bookmark_manager_remove(FRBookmarkManager *manager, const char *url) {
    if (!manager || !url) return FALSE;
    
    FRBookmark *bookmark = fr_bookmark_manager_find(manager, url);
    if (bookmark) {
        manager->bookmarks = g_list_remove(manager->bookmarks, bookmark);
        fr_bookmark_free(bookmark);
        return fr_bookmark_manager_save(manager);
    }
    
    return FALSE;
}

FRBookmark* fr_bookmark_manager_find(FRBookmarkManager *manager, const char *url) {
    if (!manager || !url) return NULL;
    
    for (GList *l = manager->bookmarks; l != NULL; l = l->next) {
        FRBookmark *bookmark = (FRBookmark*)l->data;
        if (bookmark && bookmark->url && strcmp(bookmark->url, url) == 0) {
            return bookmark;
        }
    }
    
    return NULL;
}

GList* fr_bookmark_manager_get_all(FRBookmarkManager *manager) {
    return manager ? manager->bookmarks : NULL;
}

GList* fr_bookmark_manager_get_by_folder(FRBookmarkManager *manager, const char *folder) {
    if (!manager || !folder) return NULL;
    
    GList *result = NULL;
    for (GList *l = manager->bookmarks; l != NULL; l = l->next) {
        FRBookmark *bookmark = (FRBookmark*)l->data;
        if (bookmark && bookmark->folder && strcmp(bookmark->folder, folder) == 0) {
            result = g_list_append(result, bookmark);
        }
    }
    
    return result;
}

gboolean fr_bookmark_manager_load(FRBookmarkManager *manager) {
    if (!manager || !manager->bookmarks_file) return FALSE;
    
    if (!g_file_test(manager->bookmarks_file, G_FILE_TEST_EXISTS)) {
        return TRUE; // No file exists yet, that's OK
    }
    
    GError *error = NULL;
    char *contents;
    gsize length;
    
    if (!g_file_get_contents(manager->bookmarks_file, &contents, &length, &error)) {
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
        const char *folder = json_object_get_string_member(obj, "folder");
        
        if (url) {
            FRBookmark *bookmark = fr_bookmark_new(title, url, folder);
            if (json_object_has_member(obj, "created")) {
                bookmark->created = (time_t)json_object_get_int_member(obj, "created");
            }
            manager->bookmarks = g_list_append(manager->bookmarks, bookmark);
        }
    }
    
    g_free(contents);
    g_object_unref(parser);
    return TRUE;
}

gboolean fr_bookmark_manager_save(FRBookmarkManager *manager) {
    if (!manager || !manager->bookmarks_file) return FALSE;
    
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_array(builder);
    
    for (GList *l = manager->bookmarks; l != NULL; l = l->next) {
        FRBookmark *bookmark = (FRBookmark*)l->data;
        if (!bookmark) continue;
        
        json_builder_begin_object(builder);
        
        json_builder_set_member_name(builder, "title");
        json_builder_add_string_value(builder, bookmark->title ? bookmark->title : "");
        
        json_builder_set_member_name(builder, "url");
        json_builder_add_string_value(builder, bookmark->url ? bookmark->url : "");
        
        json_builder_set_member_name(builder, "folder");
        json_builder_add_string_value(builder, bookmark->folder ? bookmark->folder : "Default");
        
        json_builder_set_member_name(builder, "created");
        json_builder_add_int_value(builder, (gint64)bookmark->created);
        
        json_builder_end_object(builder);
    }
    
    json_builder_end_array(builder);
    
    JsonGenerator *generator = json_generator_new();
    JsonNode *root = json_builder_get_root(builder);
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    
    char *json_data = json_generator_to_data(generator, NULL);
    
    GError *error = NULL;
    gboolean success = g_file_set_contents(manager->bookmarks_file, json_data, -1, &error);
    
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

GtkWidget* fr_bookmark_create_menu(FRBookmarkManager *manager) {
    GtkWidget *menu = gtk_menu_new();
    
    if (!manager) return menu;
    
    for (GList *l = manager->bookmarks; l != NULL; l = l->next) {
        FRBookmark *bookmark = (FRBookmark*)l->data;
        if (!bookmark) continue;
        
        GtkWidget *item = gtk_menu_item_new_with_label(bookmark->title);
        g_object_set_data_full(G_OBJECT(item), "bookmark-url", 
                              g_strdup(bookmark->url), g_free);
        
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    
    if (g_list_length(manager->bookmarks) == 0) {
        GtkWidget *item = gtk_menu_item_new_with_label("No bookmarks");
        gtk_widget_set_sensitive(item, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    
    gtk_widget_show_all(menu);
    return menu;
}

void fr_bookmark_show_dialog(GtkWindow *parent, FRBookmarkManager *manager, const char *url, const char *title) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Bookmark",
                                                    parent,
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    "Add", GTK_RESPONSE_OK,
                                                    NULL);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    
    // Title entry
    GtkWidget *title_label = gtk_label_new("Title:");
    GtkWidget *title_entry = gtk_entry_new();
    if (title) {
        gtk_entry_set_text(GTK_ENTRY(title_entry), title);
    }
    
    gtk_grid_attach(GTK_GRID(grid), title_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), title_entry, 1, 0, 1, 1);
    
    // URL entry
    GtkWidget *url_label = gtk_label_new("URL:");
    GtkWidget *url_entry = gtk_entry_new();
    if (url) {
        gtk_entry_set_text(GTK_ENTRY(url_entry), url);
    }
    
    gtk_grid_attach(GTK_GRID(grid), url_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), url_entry, 1, 1, 1, 1);
    
    // Folder entry
    GtkWidget *folder_label = gtk_label_new("Folder:");
    GtkWidget *folder_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(folder_entry), "Default");
    
    gtk_grid_attach(GTK_GRID(grid), folder_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), folder_entry, 1, 2, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_OK) {
        const char *bookmark_title = gtk_entry_get_text(GTK_ENTRY(title_entry));
        const char *bookmark_url = gtk_entry_get_text(GTK_ENTRY(url_entry));
        const char *bookmark_folder = gtk_entry_get_text(GTK_ENTRY(folder_entry));
        
        if (bookmark_url && strlen(bookmark_url) > 0) {
            FRBookmark *bookmark = fr_bookmark_new(bookmark_title, bookmark_url, bookmark_folder);
            fr_bookmark_manager_add(manager, bookmark);
        }
    }
    
    gtk_widget_destroy(dialog);
}
