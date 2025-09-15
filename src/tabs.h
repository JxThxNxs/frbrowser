#ifndef TABS_H
#define TABS_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "browser.h"

// Tab management functions
void fr_tab_init(FRTab *tab);
void fr_tab_free(FRTab *tab);
FRTab* fr_tab_new(const char *url);

// Tab operations
void fr_tab_set_title(FRTab *tab, const char *title);
void fr_tab_set_url(FRTab *tab, const char *url);
void fr_tab_set_loading(FRTab *tab, gboolean loading);

// Tab list management
GList* fr_tab_list_add(GList *tab_list, FRTab *tab);
GList* fr_tab_list_remove(GList *tab_list, FRTab *tab);
FRTab* fr_tab_list_find_by_webview(GList *tab_list, WebKitWebView *web_view);

#endif // TABS_H
