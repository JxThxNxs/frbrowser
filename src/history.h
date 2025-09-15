#ifndef HISTORY_H
#define HISTORY_H

#include <gtk/gtk.h>
#include <glib.h>
#include <time.h>

typedef struct {
    char *title;
    char *url;
    time_t visited;
    int visit_count;
} FRHistoryEntry;

typedef struct {
    GList *entries;
    char *history_file;
    int max_entries;
} FRHistoryManager;

// History manager functions
FRHistoryManager* fr_history_manager_new(void);
void fr_history_manager_free(FRHistoryManager *manager);
gboolean fr_history_manager_load(FRHistoryManager *manager);
gboolean fr_history_manager_save(FRHistoryManager *manager);

// History operations
FRHistoryEntry* fr_history_entry_new(const char *title, const char *url);
void fr_history_entry_free(FRHistoryEntry *entry);
void fr_history_manager_add(FRHistoryManager *manager, const char *title, const char *url);
void fr_history_manager_clear(FRHistoryManager *manager);
GList* fr_history_manager_search(FRHistoryManager *manager, const char *query);
GList* fr_history_manager_get_recent(FRHistoryManager *manager, int count);

// UI functions
GtkWidget* fr_history_create_menu(FRHistoryManager *manager);
void fr_history_show_dialog(GtkWindow *parent, FRHistoryManager *manager);

#endif // HISTORY_H
