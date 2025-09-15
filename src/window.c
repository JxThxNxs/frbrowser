#include "window.h"
#include "bookmarks.h"
#include "history.h"
#include <string.h>

void fr_window_setup_menu(FRBrowser *browser) {
    if (!browser) return;
    
    // Create menu bar
    GtkWidget *menu_bar = gtk_menu_bar_new();
    
    // File menu
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *new_tab_item = gtk_menu_item_new_with_label("New Tab");
    GtkWidget *close_tab_item = gtk_menu_item_new_with_label("Close Tab");
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_tab_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), close_tab_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    
    // Bookmarks menu
    GtkWidget *bookmarks_item = gtk_menu_item_new_with_label("Bookmarks");
    browser->bookmarks_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(bookmarks_item), browser->bookmarks_menu);
    
    GtkWidget *add_bookmark_item = gtk_menu_item_new_with_label("Add Bookmark");
    gtk_menu_shell_append(GTK_MENU_SHELL(browser->bookmarks_menu), add_bookmark_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(browser->bookmarks_menu), gtk_separator_menu_item_new());
    
    // History menu
    GtkWidget *history_item = gtk_menu_item_new_with_label("History");
    browser->history_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(history_item), browser->history_menu);
    
    GtkWidget *show_history_item = gtk_menu_item_new_with_label("Show All History");
    gtk_menu_shell_append(GTK_MENU_SHELL(browser->history_menu), show_history_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(browser->history_menu), gtk_separator_menu_item_new());
    
    // Help menu
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);
    
    // Add to menu bar
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), bookmarks_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), history_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    
    // Connect signals
    g_signal_connect(new_tab_item, "activate", G_CALLBACK(on_menu_new_tab), browser);
    g_signal_connect(close_tab_item, "activate", G_CALLBACK(on_menu_close_tab), browser);
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_menu_quit), browser);
    g_signal_connect(add_bookmark_item, "activate", G_CALLBACK(on_menu_add_bookmark), browser);
    g_signal_connect(show_history_item, "activate", G_CALLBACK(on_menu_history), browser);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_menu_about), browser);
    
    // Add menu bar to main window
    GtkWidget *vbox = gtk_bin_get_child(GTK_BIN(browser->main_window));
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(vbox), menu_bar, 0);
    
    gtk_widget_show_all(menu_bar);
}

void fr_window_update_title(FRBrowser *browser, const char *title) {
    if (!browser || !browser->main_window) return;
    
    char *window_title;
    if (title && strlen(title) > 0) {
        window_title = g_strdup_printf("%s - %s", title, FR_BROWSER_NAME);
    } else {
        window_title = g_strdup(FR_BROWSER_NAME);
    }
    
    gtk_window_set_title(GTK_WINDOW(browser->main_window), window_title);
    g_free(window_title);
}

// Menu callbacks
void on_menu_new_tab(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    fr_browser_new_tab(browser, DEFAULT_HOME_PAGE);
}

void on_menu_close_tab(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    if (browser->current_tab >= 0) {
        fr_browser_close_tab(browser, browser->current_tab);
    }
}

void on_menu_quit(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    if (browser->main_window) {
        gtk_widget_destroy(browser->main_window);
    }
}

void on_menu_about(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    
    GtkWidget *about_dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), FR_BROWSER_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), FR_BROWSER_VERSION);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), 
                                  "A lightweight, fast, and open-source web browser");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog), "https://github.com/JxThxNxs/frbrowser");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_MIT_X11);
    
    // Set the logo for the about dialog
    GError *error = NULL;
    GdkPixbuf *logo = NULL;
    
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
        logo = gdk_pixbuf_new_from_file(icon_paths[i], &error);
        if (logo) {
            // Scale the logo to appropriate size for about dialog
            GdkPixbuf *scaled_logo = gdk_pixbuf_scale_simple(logo, 128, 128, GDK_INTERP_BILINEAR);
            gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog), scaled_logo);
            g_object_unref(logo);
            g_object_unref(scaled_logo);
            break;
        }
        if (error) {
            g_error_free(error);
            error = NULL;
        }
    }
    
    const char *authors[] = {"FR Browser Team", "JxThxNxs", NULL};
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
    
    gtk_window_set_transient_for(GTK_WINDOW(about_dialog), GTK_WINDOW(browser->main_window));
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);
}

void on_menu_bookmarks(GtkMenuItem *item, gpointer user_data) {
    // This will be called when bookmark menu items are clicked
    FRBrowser *browser = (FRBrowser*)user_data;
    const char *url = (const char*)g_object_get_data(G_OBJECT(item), "bookmark-url");
    if (url) {
        fr_browser_navigate_to(browser, url);
    }
}

void on_menu_history(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    // Show history dialog - this would need a history manager instance
    // fr_history_show_dialog(GTK_WINDOW(browser->main_window), browser->history_manager);
}

void on_menu_add_bookmark(GtkMenuItem *item, gpointer user_data) {
    FRBrowser *browser = (FRBrowser*)user_data;
    
    if (browser->current_tab < 0) return;
    
    GtkWidget *current_page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(browser->notebook), browser->current_tab);
    if (current_page) {
        WebKitWebView *web_view = WEBKIT_WEB_VIEW(gtk_bin_get_child(GTK_BIN(current_page)));
        const char *url = webkit_web_view_get_uri(web_view);
        const char *title = webkit_web_view_get_title(web_view);
        
        // Show bookmark dialog - this would need a bookmark manager instance
        // fr_bookmark_show_dialog(GTK_WINDOW(browser->main_window), browser->bookmark_manager, url, title);
    }
}
