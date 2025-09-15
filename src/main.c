#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdio.h>
#include <stdlib.h>
#include "browser.h"
#include "window.h"
#include "utils.h"

static void activate(GtkApplication *app, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    
    // Create main window
    browser->main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(browser->main_window), FR_BROWSER_NAME);
    gtk_window_set_default_size(GTK_WINDOW(browser->main_window), 1200, 800);
    // Try to load custom icon first, fallback to system icon
    GError *error = NULL;
    GdkPixbuf *icon = NULL;
    
    // Try different possible locations for the icon
    const char *icon_paths[] = {
        "../icons/fr-browser.png",  // From build directory
        "icons/fr-browser.png",    // From project root
        "./icons/fr-browser.png",   // Current directory
        "/usr/share/pixmaps/fr-browser.png",  // System install location
        "/usr/share/icons/hicolor/256x256/apps/fr-browser.png",  // Standard icon location
        NULL
    };
    
    for (int i = 0; icon_paths[i] != NULL; i++) {
        icon = gdk_pixbuf_new_from_file(icon_paths[i], &error);
        if (icon) {
            gtk_window_set_icon(GTK_WINDOW(browser->main_window), icon);
            g_object_unref(icon);
            break;
        }
        if (error) {
            g_error_free(error);
            error = NULL;
        }
    }
    
    // Fallback to system icon if no custom icon found
    if (!icon) {
        gtk_window_set_icon_name(GTK_WINDOW(browser->main_window), "web-browser");
    }
    
    // Create main container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(browser->main_window), vbox);
    
    // Create toolbar
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_set_homogeneous(GTK_BOX(toolbar), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    // Navigation buttons with improved styling
    browser->back_button = gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_LARGE_TOOLBAR);
    browser->forward_button = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_LARGE_TOOLBAR);
    browser->refresh_button = gtk_button_new_from_icon_name("view-refresh", GTK_ICON_SIZE_LARGE_TOOLBAR);
    browser->home_button = gtk_button_new_from_icon_name("go-home", GTK_ICON_SIZE_LARGE_TOOLBAR);
    
    // Add tooltips for better UX
    gtk_widget_set_tooltip_text(browser->back_button, "Go Back");
    gtk_widget_set_tooltip_text(browser->forward_button, "Go Forward");
    gtk_widget_set_tooltip_text(browser->refresh_button, "Refresh Page");
    gtk_widget_set_tooltip_text(browser->home_button, "Go Home");
    
    // Make buttons more prominent
    gtk_button_set_relief(GTK_BUTTON(browser->back_button), GTK_RELIEF_NORMAL);
    gtk_button_set_relief(GTK_BUTTON(browser->forward_button), GTK_RELIEF_NORMAL);
    gtk_button_set_relief(GTK_BUTTON(browser->refresh_button), GTK_RELIEF_NORMAL);
    gtk_button_set_relief(GTK_BUTTON(browser->home_button), GTK_RELIEF_NORMAL);
    
    gtk_box_pack_start(GTK_BOX(toolbar), browser->back_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), browser->forward_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), browser->refresh_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(toolbar), browser->home_button, FALSE, FALSE, 2);
    
    // URL entry
    browser->url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(browser->url_entry), "Enter URL or search...");
    gtk_box_pack_start(GTK_BOX(toolbar), browser->url_entry, TRUE, TRUE, 5);
    
    // New tab and menu buttons with improved styling
    browser->new_tab_button = gtk_button_new_from_icon_name("tab-new", GTK_ICON_SIZE_LARGE_TOOLBAR);
    browser->menu_button = gtk_button_new_from_icon_name("open-menu", GTK_ICON_SIZE_LARGE_TOOLBAR);
    
    // Add tooltips
    gtk_widget_set_tooltip_text(browser->new_tab_button, "New Tab");
    gtk_widget_set_tooltip_text(browser->menu_button, "Menu");
    
    // Make buttons more prominent
    gtk_button_set_relief(GTK_BUTTON(browser->new_tab_button), GTK_RELIEF_NORMAL);
    gtk_button_set_relief(GTK_BUTTON(browser->menu_button), GTK_RELIEF_NORMAL);
    
    gtk_box_pack_start(GTK_BOX(toolbar), browser->new_tab_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), browser->menu_button, FALSE, FALSE, 0);
    
    // Create notebook for tabs
    browser->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(browser->notebook), TRUE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(browser->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(browser->notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), browser->notebook, TRUE, TRUE, 0);
    
    // Setup menu
    fr_window_setup_menu(browser);
    
    // Connect signals
    g_signal_connect(browser->back_button, "clicked", G_CALLBACK(fr_browser_go_back), browser);
    g_signal_connect(browser->forward_button, "clicked", G_CALLBACK(fr_browser_go_forward), browser);
    g_signal_connect(browser->refresh_button, "clicked", G_CALLBACK(fr_browser_refresh), browser);
    g_signal_connect(browser->home_button, "clicked", G_CALLBACK(fr_browser_go_home), browser);
    g_signal_connect(browser->new_tab_button, "clicked", G_CALLBACK(on_new_tab_clicked), browser);
    
    // URL entry activation
    g_signal_connect(browser->url_entry, "activate", G_CALLBACK(on_url_entry_activate), browser);
    
    // Create initial tab
    fr_browser_new_tab(browser, DEFAULT_HOME_PAGE);
    
    // Show all widgets
    gtk_widget_show_all(browser->main_window);
}

int main(int argc, char **argv) {
    FRBrowser *browser = fr_browser_new();
    
    browser->app = gtk_application_new("org.frbrowser.FRBrowser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(browser->app, "activate", G_CALLBACK(activate), browser);
    
    int status = g_application_run(G_APPLICATION(browser->app), argc, argv);
    
    g_object_unref(browser->app);
    fr_browser_free(browser);
    
    return status;
}
