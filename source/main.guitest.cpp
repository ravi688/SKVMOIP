#ifndef BUILD_GUITEST
#error "BUILD_GUITEST is not defined but still main.guidtest.cpp is being compiled"
#endif

#include <SKVMOIP/debug.h>

#define WINDOW_MIN_SIZE_X 600
#define WINDOW_MIN_SIZE_Y 900
#define WINDOW_DEF_SIZE_X WINDOW_MIN_SIZE_X
#define WINDOW_DEF_SIZE_Y WINDOW_MIN_SIZE_Y

// Include gtk
#include <gtk/gtk.h>

static void on_activate (GtkApplication *app) {
  // Create a new window
  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title(GTK_WINDOW(window), "SKVMOIP Client");
  gtk_widget_set_size_request(window, WINDOW_MIN_SIZE_X, WINDOW_MIN_SIZE_Y);
  gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_DEF_SIZE_X, WINDOW_DEF_SIZE_Y);
  gtk_window_set_gravity(GTK_WINDOW(window), GDK_GRAVITY_NORTH_WEST);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  // Create a new button
  GtkWidget *button = gtk_button_new_with_label ("Hello, World!");
  // When the button is clicked, close the window passed as an argument
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_close), window);
  gtk_container_add (GTK_CONTAINER(window), button);
  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication *app = gtk_application_new ("com.example.GtkApplication",
                                             G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  return g_application_run (G_APPLICATION (app), argc, argv);
}

