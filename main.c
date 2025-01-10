// Copyright (C) 2025 ITlopa
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

static GtkWidget *text_view;
static GtkWidget *entry;

void execute_command(GtkWidget *widget, gpointer data) {
    const char *command = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(command) == 0) {
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_insert_at_cursor(buffer, "> ", -1);
    gtk_text_buffer_insert_at_cursor(buffer, command, -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        FILE *input = fopen("/dev/stdin", "r");
        if (input) {
            dup2(fileno(input), STDIN_FILENO);
            fclose(input);
        }

        char *args[] = {"/bin/sh", "-c", (char *)command, NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else {
        close(pipefd[1]);

        int status;
        waitpid(pid, &status, 0);
        char buffer_output[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer_output, sizeof(buffer_output) - 1)) > 0) {
            buffer_output[bytes_read] = '\0';
            gtk_text_buffer_insert_at_cursor(buffer, buffer_output, -1);
        }
        close(pipefd[0]);
    }

    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Ituxalien Terminal V0.1.0");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *label = gtk_label_new("Welcome to Ituxalien Terminal: Version - 0. Minor - 1. Patch - 0.\n(C) 2025 ITlopa. All rights are reserved");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
    g_signal_connect(entry, "activate", G_CALLBACK(execute_command), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
