#include "browser.h"
#include "tabs.h"
#include "utils.h"
#include "window.h"
#include <string.h>
#include <stdlib.h>

FRBrowser* fr_browser_new(void) {
    FRBrowser *browser = g_malloc0(sizeof(FRBrowser));
    browser->current_tab = -1;
    browser->tab_count = 0;
    return browser;
}

void fr_browser_free(FRBrowser *browser) {
    if (browser) {
        g_free(browser);
    }
}

void fr_browser_navigate_to(FRBrowser *browser, const char *url) {
    if (!browser || !url || browser->current_tab < 0) return;
    
    char *sanitized_url = fr_browser_sanitize_url(url);
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page && GTK_IS_SCROLLED_WINDOW(current_page)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(current_page));
        if (child && WEBKIT_IS_WEB_VIEW(child)) {
            WebKitWebView *web_view = WEBKIT_WEB_VIEW(child);
            webkit_web_view_load_uri(web_view, sanitized_url);
        }
    }
    
    gtk_entry_set_text(GTK_ENTRY(browser->url_entry), sanitized_url);
    g_free(sanitized_url);
}

void fr_browser_go_back(FRBrowser *browser) {
    if (!browser || browser->current_tab < 0) return;
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page && GTK_IS_SCROLLED_WINDOW(current_page)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(current_page));
        if (child && WEBKIT_IS_WEB_VIEW(child)) {
            WebKitWebView *web_view = WEBKIT_WEB_VIEW(child);
            webkit_web_view_go_back(web_view);
        }
    }
}

void fr_browser_go_forward(FRBrowser *browser) {
    if (!browser || browser->current_tab < 0) return;
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page && GTK_IS_SCROLLED_WINDOW(current_page)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(current_page));
        if (child && WEBKIT_IS_WEB_VIEW(child)) {
            WebKitWebView *web_view = WEBKIT_WEB_VIEW(child);
            webkit_web_view_go_forward(web_view);
        }
    }
}

void fr_browser_refresh(FRBrowser *browser) {
    if (!browser || browser->current_tab < 0) return;
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page && GTK_IS_SCROLLED_WINDOW(current_page)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(current_page));
        if (child && WEBKIT_IS_WEB_VIEW(child)) {
            WebKitWebView *web_view = WEBKIT_WEB_VIEW(child);
            webkit_web_view_reload(web_view);
        }
    }
}

void fr_browser_go_home(FRBrowser *browser) {
    fr_browser_navigate_to(browser, DEFAULT_HOME_PAGE);
}


void fr_browser_new_tab(FRBrowser *browser, const char *url) {
    if (!browser) return;
    
    const char *target_url = url ? url : DEFAULT_HOME_PAGE;
    
    // Create scrolled window for the web view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Create web view
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(web_view));
    
    // Create tab label with close button
    GtkWidget *tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *tab_label = gtk_label_new("New Tab");
    GtkWidget *close_button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(close_button), GTK_RELIEF_NONE);
    
    gtk_box_pack_start(GTK_BOX(tab_box), tab_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(tab_box), close_button, FALSE, FALSE, 0);
    gtk_widget_show_all(tab_box);
    
    // Add tab to notebook
    int page_num = gtk_notebook_append_page(GTK_NOTEBOOK(browser->notebook), scrolled_window, tab_box);
    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(browser->notebook), scrolled_window, TRUE);
    
    // Connect web view signals
    g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), browser);
    g_signal_connect(web_view, "notify::title", G_CALLBACK(on_title_changed), browser);
    g_signal_connect(web_view, "notify::uri", G_CALLBACK(on_uri_changed), browser);
    
    // Connect close button signal
    g_signal_connect(close_button, "clicked", G_CALLBACK(on_tab_close_clicked), browser);
    g_object_set_data(G_OBJECT(close_button), "page-widget", scrolled_window);
    
    // Switch to new tab
    gtk_notebook_set_current_page(GTK_NOTEBOOK(browser->notebook), page_num);
    browser->current_tab = page_num;
    browser->tab_count++;
    
    // Load URL
    webkit_web_view_load_uri(web_view, target_url);
    
    gtk_widget_show_all(scrolled_window);
}

void fr_browser_close_tab(FRBrowser *browser, int tab_index) {
    if (!browser || tab_index < 0) return;
    
    GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), tab_index);
    if (page) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(browser->notebook), tab_index);
        browser->tab_count--;
        
        if (browser->tab_count == 0) {
            // Create a new tab if all tabs are closed
            fr_browser_new_tab(browser, DEFAULT_HOME_PAGE);
        } else {
            // Update current tab index
            int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(browser->notebook));
            browser->current_tab = current;
            fr_browser_update_ui(browser);
        }
    }
}

void fr_browser_switch_tab(FRBrowser *browser, int tab_index) {
    if (!browser || tab_index < 0) return;
    
    gtk_notebook_set_current_page(GTK_NOTEBOOK(browser->notebook), tab_index);
    browser->current_tab = tab_index;
    fr_browser_update_ui(browser);
}

void fr_browser_update_ui(FRBrowser *browser) {
    if (!browser || browser->current_tab < 0) return;
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page && GTK_IS_SCROLLED_WINDOW(current_page)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(current_page));
        if (child && WEBKIT_IS_WEB_VIEW(child)) {
            WebKitWebView *web_view = WEBKIT_WEB_VIEW(child);
            
            // Update URL entry
            const char *uri = webkit_web_view_get_uri(web_view);
            if (uri) {
                gtk_entry_set_text(GTK_ENTRY(browser->url_entry), uri);
            }
            
            // Update navigation buttons
            gboolean can_go_back = webkit_web_view_can_go_back(web_view);
            gboolean can_go_forward = webkit_web_view_can_go_forward(web_view);
            
            gtk_widget_set_sensitive(browser->back_button, can_go_back);
            gtk_widget_set_sensitive(browser->forward_button, can_go_forward);
        }
    }
}

