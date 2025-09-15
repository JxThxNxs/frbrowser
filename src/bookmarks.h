#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <gtk/gtk.h>
#include <glib.h>

typedef struct {
    char *title;
    char *url;
    char *folder;
    time_t created;
} FRBookmark;

typedef struct {
    GList *bookmarks;
    char *bookmarks_file;
} FRBookmarkManager;

// Bookmark manager functions
FRBookmarkManager* fr_bookmark_manager_new(void);
void fr_bookmark_manager_free(FRBookmarkManager *manager);
gboolean fr_bookmark_manager_load(FRBookmarkManager *manager);
gboolean fr_bookmark_manager_save(FRBookmarkManager *manager);

// Bookmark operations
FRBookmark* fr_bookmark_new(const char *title, const char *url, const char *folder);
void fr_bookmark_free(FRBookmark *bookmark);
gboolean fr_bookmark_manager_add(FRBookmarkManager *manager, FRBookmark *bookmark);
gboolean fr_bookmark_manager_remove(FRBookmarkManager *manager, const char *url);
FRBookmark* fr_bookmark_manager_find(FRBookmarkManager *manager, const char *url);
GList* fr_bookmark_manager_get_all(FRBookmarkManager *manager);
GList* fr_bookmark_manager_get_by_folder(FRBookmarkManager *manager, const char *folder);

// UI functions
GtkWidget* fr_bookmark_create_menu(FRBookmarkManager *manager);
void fr_bookmark_show_dialog(GtkWindow *parent, FRBookmarkManager *manager, const char *url, const char *title);

#endif // BOOKMARKS_H
