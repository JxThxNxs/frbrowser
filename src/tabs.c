#include "tabs.h"
#include <string.h>
#include <stdlib.h>

void fr_tab_init(FRTab *tab) {
    if (!tab) return;
    
    tab->web_view = NULL;
    tab->tab_label = NULL;
    tab->title = NULL;
    tab->url = NULL;
    tab->loading = FALSE;
}

void fr_tab_free(FRTab *tab) {
    if (!tab) return;
    
    if (tab->title) {
        g_free(tab->title);
    }
    if (tab->url) {
        g_free(tab->url);
    }
    
    g_free(tab);
}

FRTab* fr_tab_new(const char *url) {
    FRTab *tab = g_malloc0(sizeof(FRTab));
    fr_tab_init(tab);
    
    if (url) {
        tab->url = g_strdup(url);
    }
    
    return tab;
}

void fr_tab_set_title(FRTab *tab, const char *title) {
    if (!tab) return;
    
    if (tab->title) {
        g_free(tab->title);
    }
    
    tab->title = title ? g_strdup(title) : NULL;
}

void fr_tab_set_url(FRTab *tab, const char *url) {
    if (!tab) return;
    
    if (tab->url) {
        g_free(tab->url);
    }
    
    tab->url = url ? g_strdup(url) : NULL;
}

void fr_tab_set_loading(FRTab *tab, gboolean loading) {
    if (!tab) return;
    tab->loading = loading;
}

GList* fr_tab_list_add(GList *tab_list, FRTab *tab) {
    if (!tab) return tab_list;
    return g_list_append(tab_list, tab);
}

GList* fr_tab_list_remove(GList *tab_list, FRTab *tab) {
    if (!tab) return tab_list;
    return g_list_remove(tab_list, tab);
}

FRTab* fr_tab_list_find_by_webview(GList *tab_list, WebKitWebView *web_view) {
    if (!web_view) return NULL;
    
    for (GList *l = tab_list; l != NULL; l = l->next) {
        FRTab *tab = (FRTab*)l->data;
        if (tab && tab->web_view == web_view) {
            return tab;
        }
    }
    
    return NULL;
}
