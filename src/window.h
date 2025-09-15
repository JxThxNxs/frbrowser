#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>
#include "browser.h"

// Window management functions
void fr_window_init(FRBrowser *browser);
void fr_window_setup_ui(FRBrowser *browser);
void fr_window_setup_menu(FRBrowser *browser);
void fr_window_update_title(FRBrowser *browser, const char *title);

// Menu callbacks
void on_menu_new_tab(GtkMenuItem *item, gpointer user_data);
void on_menu_close_tab(GtkMenuItem *item, gpointer user_data);
void on_menu_quit(GtkMenuItem *item, gpointer user_data);
void on_menu_about(GtkMenuItem *item, gpointer user_data);
void on_menu_bookmarks(GtkMenuItem *item, gpointer user_data);
void on_menu_history(GtkMenuItem *item, gpointer user_data);
void on_menu_add_bookmark(GtkMenuItem *item, gpointer user_data);

#endif // WINDOW_H
