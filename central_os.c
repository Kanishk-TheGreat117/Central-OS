#include <gtk/gtk.h>
#include <dirent.h>
#include <time.h>

static void update_clock(GtkLabel *label) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", local);
    gtk_label_set_text(label, buffer);
}

static gboolean tick(gpointer user_data) {
    update_clock(GTK_LABEL(user_data));
    return TRUE;
}

static GtkWidget* create_file_explorer(const gchar *path) {
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *list = gtk_list_box_new();
    gtk_widget_add_css_class(scroller, "glass-panel");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), list);

    DIR *dir = opendir(path);
    if (!dir) {
        GtkWidget *error_label = gtk_label_new("Failed to open directory.");
        gtk_list_box_append(GTK_LIST_BOX(list), error_label);
        return scroller;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        GtkWidget *icon = gtk_image_new_from_icon_name("text-x-generic");
        GtkWidget *label = gtk_label_new(entry->d_name);
        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), label);
        gtk_list_box_append(GTK_LIST_BOX(list), row);
    }
    closedir(dir);
    return scroller;
}

static void switch_view(GtkButton *button, gpointer user_data) {
    const gchar *view_name = g_object_get_data(G_OBJECT(button), "view-name");
    gtk_stack_set_visible_child_name(GTK_STACK(user_data), view_name);
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Load CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Central OS");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    gtk_widget_add_css_class(window, "main-window");

    // Top bar
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(top_bar, "top-bar");
    GtkWidget *greeting = gtk_label_new("Hi Kanishk");
    GtkWidget *clock = gtk_label_new("");
    update_clock(GTK_LABEL(clock));
    g_timeout_add_seconds(1, tick, clock);
    gtk_box_append(GTK_BOX(top_bar), greeting);
    gtk_box_append(GTK_BOX(top_bar), clock);

    // Stack views
    GtkWidget *stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    GtkWidget *home_view = gtk_label_new("🏠 Welcome to Central OS");
    GtkWidget *files_view = create_file_explorer(".");
    GtkWidget *terminal_view = gtk_label_new("🖥️ Terminal View");
    GtkWidget *settings_view = gtk_label_new("⚙️ Settings View");
    gtk_stack_add_named(GTK_STACK(stack), home_view, "home");
    gtk_stack_add_named(GTK_STACK(stack), files_view, "files");
    gtk_stack_add_named(GTK_STACK(stack), terminal_view, "terminal");
    gtk_stack_add_named(GTK_STACK(stack), settings_view, "settings");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "home");

    // Dock
    GtkWidget *dock = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(dock, "dock");

    GtkWidget *btn_home = gtk_button_new_from_icon_name("go-home-symbolic");
    GtkWidget *btn_files = gtk_button_new_from_icon_name("system-file-manager-symbolic");
    GtkWidget *btn_terminal = gtk_button_new_from_icon_name("utilities-terminal-symbolic");
    GtkWidget *btn_settings = gtk_button_new_from_icon_name("preferences-system-symbolic");
    GtkWidget *btn_browser = gtk_button_new_from_icon_name("web-browser-symbolic");

    g_object_set_data(G_OBJECT(btn_home), "view-name", "home");
    g_object_set_data(G_OBJECT(btn_files), "view-name", "files");
    g_object_set_data(G_OBJECT(btn_terminal), "view-name", "terminal");
    g_object_set_data(G_OBJECT(btn_settings), "view-name", "settings");

    g_signal_connect(btn_home, "clicked", G_CALLBACK(switch_view), stack);
    g_signal_connect(btn_files, "clicked", G_CALLBACK(switch_view), stack);
    g_signal_connect(btn_terminal, "clicked", G_CALLBACK(switch_view), stack);
    g_signal_connect(btn_settings, "clicked", G_CALLBACK(switch_view), stack);
    g_signal_connect(btn_browser, "clicked", G_CALLBACK(g_spawn_command_line_async), "cmd.exe /c start chrome");

    gtk_box_append(GTK_BOX(dock), btn_home);
    gtk_box_append(GTK_BOX(dock), btn_files);
    gtk_box_append(GTK_BOX(dock), btn_terminal);
    gtk_box_append(GTK_BOX(dock), btn_settings);
    gtk_box_append(GTK_BOX(dock), btn_browser);

    // Layout
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(layout), top_bar);
    gtk_widget_set_vexpand(stack, TRUE);
    gtk_box_append(GTK_BOX(layout), stack);
    gtk_box_append(GTK_BOX(layout), dock);

    gtk_window_set_child(GTK_WINDOW(window), layout);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.central.os", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
