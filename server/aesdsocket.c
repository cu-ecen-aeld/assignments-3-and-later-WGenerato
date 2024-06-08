#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <getopt.h>
#include <sys/stat.h>
#define PORT 9000
#define BUFFER_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

int sockfd = -1;
int new_sockfd = -1;
FILE *file = NULL;

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        if (new_sockfd != -1) {
            close(new_sockfd);
        }
        if (sockfd != -1) {
            close(sockfd);
        }
        if (file) {
            fclose(file);
            remove(FILE_PATH);
        }
        closelog();
        remove(FILE_PATH);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    ssize_t bytes_read;
    char *line = NULL;
    size_t linecap = 0;
    
    int daemonize = 0;
    int option;

    // Parse command-line arguments
    while ((option = getopt(argc, argv, "d")) != -1) {
        switch (option) {
        case 'd':
            daemonize = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Daemonize if -d ion is present
    if (daemonize) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process
            exit(EXIT_SUCCESS);
        }

        // Child process
        umask(0); // Reset file mode creation mask

        // Create a new session
        if (setsid() < 0) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }

        // Change the working directory to root
        if (chdir("/") < 0) {
            perror("chdir");
            exit(EXIT_FAILURE);
        }

        // Close standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }




    // Open syslog
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    // Handle signals for graceful exit
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_ERR, "Socket creation failed: %s", strerror(errno));
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "Set socket options failed: %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Bind failed: %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    // Listen for connections
    if (listen(sockfd, 10) < 0) {
        syslog(LOG_ERR, "Listen failed: %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    syslog(LOG_INFO, "Server started on port %d", PORT);

    while (1) {
        // Accept connection
        if ((new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
            syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
            close(sockfd);
            return -1;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // Open file
        file = fopen(FILE_PATH, "a+");
        if (!file) {
            syslog(LOG_ERR, "File open failed: %s", strerror(errno));
            close(new_sockfd);
            close(sockfd);
            return -1;
        }

        // Receive data
        while ((bytes_read = recv(new_sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_read] = '\0';
            fprintf(file, "%s", buffer);
            fflush(file);

            // Check for newline
            char *newline_ptr = strchr(buffer, '\n');
            if (newline_ptr != NULL) {
                rewind(file);
                // Send the content of the file back to client
                while (getline(&line, &linecap, file) != -1) {
                    send(new_sockfd, line, strlen(line), 0);
                }
                free(line);
                line = NULL;
                linecap = 0;
            }
        }

        if (bytes_read < 0) {
            syslog(LOG_ERR, "Recv failed: %s", strerror(errno));
        }

        // Close the connection
        close(new_sockfd);
        new_sockfd = -1;
        fclose(file);
        file = NULL;

        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    close(sockfd);
    closelog();
    return 0;
}

