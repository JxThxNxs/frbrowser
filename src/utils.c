#include "utils.h"
#include "browser.h"
#include "window.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char* fr_browser_sanitize_url(const char *input) {
    if (!input || strlen(input) == 0) {
        return g_strdup(DEFAULT_HOME_PAGE);
    }
    
    char *trimmed = fr_browser_trim_whitespace(input);
    
    // Check if it's already a valid URL with protocol
    if (g_str_has_prefix(trimmed, "http://") || 
        g_str_has_prefix(trimmed, "https://") ||
        g_str_has_prefix(trimmed, "file://") ||
        g_str_has_prefix(trimmed, "ftp://")) {
        return trimmed;
    }
    
    // Check if it looks like a domain (contains a dot and no spaces)
    if (strchr(trimmed, '.') && !strchr(trimmed, ' ')) {
        char *url = g_strdup_printf("https://%s", trimmed);
        g_free(trimmed);
        return url;
    }
    
    // Otherwise, treat as search query
    char *search_url = fr_browser_create_search_url(trimmed);
    g_free(trimmed);
    return search_url;
}

gboolean fr_browser_is_valid_url(const char *url) {
    if (!url) return FALSE;
    
    return (g_str_has_prefix(url, "http://") || 
            g_str_has_prefix(url, "https://") ||
            g_str_has_prefix(url, "file://") ||
            g_str_has_prefix(url, "ftp://"));
}

char* fr_browser_create_search_url(const char *query) {
    if (!query) return g_strdup(DEFAULT_HOME_PAGE);
    
    // URL encode the query
    char *encoded_query = g_uri_escape_string(query, NULL, FALSE);
    char *search_url = g_strdup_printf("https://duckduckgo.com/?q=%s", encoded_query);
    
    g_free(encoded_query);
    return search_url;
}

char* fr_browser_trim_whitespace(const char *str) {
    if (!str) return NULL;
    
    // Skip leading whitespace
    while (isspace(*str)) str++;
    
    if (*str == 0) return g_strdup("");
    
    // Find end of string
    const char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    
    // Create new string
    size_t len = end - str + 1;
    char *trimmed = g_malloc(len + 1);
    memcpy(trimmed, str, len);
    trimmed[len] = '\0';
    
    return trimmed;
}

gboolean fr_browser_string_is_empty(const char *str) {
    if (!str) return TRUE;
    
    char *trimmed = fr_browser_trim_whitespace(str);
    gboolean is_empty = (strlen(trimmed) == 0);
    g_free(trimmed);
    
    return is_empty;
}

char* fr_browser_get_config_dir(void) {
    const char *config_home = g_get_user_config_dir();
    return g_build_filename(config_home, "fr-browser", NULL);
}

char* fr_browser_get_cache_dir(void) {
    const char *cache_home = g_get_user_cache_dir();
    return g_build_filename(cache_home, "fr-browser", NULL);
}

gboolean fr_browser_ensure_directory(const char *path) {
    if (!path) return FALSE;
    
    if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
        return TRUE;
    }
    
    return (g_mkdir_with_parents(path, 0755) == 0);
}

// Signal callback implementations
void on_new_tab_clicked(GtkButton *button, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    fr_browser_new_tab(browser, DEFAULT_HOME_PAGE);
}

void on_url_entry_activate(GtkEntry *entry, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    const char *url = gtk_entry_get_text(entry);
    fr_browser_navigate_to(browser, url);
}

void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    
    switch (load_event) {
        case WEBKIT_LOAD_STARTED:
            // Show loading indicator
            break;
        case WEBKIT_LOAD_COMMITTED:
            fr_browser_update_ui(browser);
            break;
        case WEBKIT_LOAD_FINISHED:
            fr_browser_update_ui(browser);
            break;
        default:
            break;
    }
}

void on_title_changed(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    const char *title = webkit_web_view_get_title(web_view);
    
    if (title) {
        // Find the tab containing this web view and update its label
        int n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(browser->notebook));
        for (int i = 0; i < n_pages; i++) {
            GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), i);
            WebKitWebView *page_web_view = WEBKIT_WEB_VIEW(gtk_bin_get_child(GTK_BIN(page)));
            
            if (page_web_view == web_view) {
                GtkWidget *tab_label_widget = gtk_notebook_get_tab_label(GTK_NOTEBOOK(browser->notebook), page);
                GList *children = gtk_container_get_children(GTK_CONTAINER(tab_label_widget));
                if (children && children->data) {
                    GtkLabel *label = GTK_LABEL(children->data);
                    
                    // Truncate long titles
                    char *short_title = g_strndup(title, 20);
                    if (strlen(title) > 20) {
                        strcat(short_title, "...");
                    }
                    
                    gtk_label_set_text(label, short_title);
                    g_free(short_title);
                }
                g_list_free(children);
                break;
            }
        }
        
        // Update window title if this is the current tab
        if (gtk_notebook_get_current_page(GTK_NOTEBOOK(browser->notebook)) == 
            gtk_notebook_page_num(GTK_NOTEBOOK(browser->notebook), 
                                 gtk_widget_get_parent(GTK_WIDGET(web_view)))) {
            fr_window_update_title(browser, title);
        }
    }
}

void on_uri_changed(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    fr_browser_update_ui(browser);
}

void on_tab_close_clicked(GtkButton *button, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    GtkWidget *page = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "page-widget"));
    
    if (page) {
        int page_num = gtk_notebook_page_num(GTK_NOTEBOOK(browser->notebook), page);
        fr_browser_close_tab(browser, page_num);
    }
}
