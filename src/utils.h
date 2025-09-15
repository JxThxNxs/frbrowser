#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib.h>

// URL utilities
char* fr_browser_sanitize_url(const char *input);
gboolean fr_browser_is_valid_url(const char *url);
char* fr_browser_create_search_url(const char *query);

// String utilities
char* fr_browser_trim_whitespace(const char *str);
gboolean fr_browser_string_is_empty(const char *str);

// File utilities
char* fr_browser_get_config_dir(void);
char* fr_browser_get_cache_dir(void);
gboolean fr_browser_ensure_directory(const char *path);

// Signal callback declarations
void on_url_entry_activate(GtkEntry *entry, gpointer user_data);
void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, gpointer user_data);
void on_title_changed(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data);
void on_uri_changed(WebKitWebView *web_view, GParamSpec *pspec, gpointer user_data);
void on_tab_close_clicked(GtkButton *button, gpointer user_data);
void on_new_tab_clicked(GtkButton *button, gpointer user_data);

#endif // UTILS_H
