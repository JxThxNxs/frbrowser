#ifndef BROWSER_H
#define BROWSER_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#define FR_BROWSER_NAME "FR Browser"
#define FR_BROWSER_VERSION "1.0.0"
#define DEFAULT_HOME_PAGE "https://duckduckgo.com"

typedef struct {
    GtkApplication *app;
    GtkWidget *main_window;
    GtkWidget *notebook;
    GtkWidget *url_entry;
    GtkWidget *back_button;
    GtkWidget *forward_button;
    GtkWidget *refresh_button;
    GtkWidget *home_button;
    GtkWidget *new_tab_button;
    GtkWidget *menu_button;
    
    // Menu items
    GtkWidget *bookmarks_menu;
    GtkWidget *history_menu;
    
    // Current tab info
    int current_tab;
    int tab_count;
} FRBrowser;

typedef struct {
    WebKitWebView *web_view;
    GtkWidget *tab_label;
    char *title;
    char *url;
    gboolean loading;
} FRTab;

// Function declarations
FRBrowser* fr_browser_new(void);
void fr_browser_free(FRBrowser *browser);
int fr_browser_run(FRBrowser *browser, int argc, char **argv);

// Navigation functions
void fr_browser_navigate_to(FRBrowser *browser, const char *url);
void fr_browser_go_back(FRBrowser *browser);
void fr_browser_go_forward(FRBrowser *browser);
void fr_browser_refresh(FRBrowser *browser);
void fr_browser_go_home(FRBrowser *browser);

// Tab management
void fr_browser_new_tab(FRBrowser *browser, const char *url);
void fr_browser_close_tab(FRBrowser *browser, int tab_index);
void fr_browser_switch_tab(FRBrowser *browser, int tab_index);

// Utility functions
char* fr_browser_sanitize_url(const char *input);
void fr_browser_update_ui(FRBrowser *browser);

#endif // BROWSER_H
