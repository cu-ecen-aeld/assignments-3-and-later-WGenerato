#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>


#define PORT 9000
#define BUFFER_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

int sockfd;
int new_sockfd;
FILE *file;

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
        exit(EXIT_SUCCESS);
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    ssize_t bytes_read;
    //size_t len;
    char *line = NULL;
    size_t linecap = 0;

    // Open syslog
    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    // Handle signals for graceful exit
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

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

        // Close the connection
        close(new_sockfd);
        fclose(file);
        file = NULL;

        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    close(sockfd);
    closelog();
    return 0;
}

