/***
 *       _____  _                    __         ______       __
 *      / ___/ (_)____ ___   ____   / /___     / ____/___   / /_
 *      \__ \ / // __ `__ \ / __ \ / // _ \   / / __ / _ \ / __/
 *     ___/ // // / / / / // /_/ // //  __/  / /_/ //  __// /_
 *    /____//_//_/ /_/ /_// .___//_/ \___/   \____/ \___/ \__/
 *                       /_/
 *
 *                 _____
 *                / ___/ ___   _____ _   __ ___   _____
 *                \__ \ / _ \ / ___/| | / // _ \ / ___/
 *               ___/ //  __// /    | |/ //  __// /
 *              /____/ \___//_/     |___/ \___//_/
 *
 * DESCRIPTION:
 * simple_get_server is a server that aims to be a MVP (minimal viable program)
 * for responding to HTTP GET requests. To obtain this goal, SGS aims to have
 * the least of amount of code need to achieve this goal.
 *
 * Some features of common web servers are not present here and that is
 * intentional. These features are not need to achieve the goal of MVP. These
 * features however can be desirable features and thus can be added onto this
 * program through patches.
 *
 * LICENSE: MIT
 * Copyright 2019 Ryan Moore
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"

#define HTTP_200 \
"HTTP/1.0 200 OK\r\n"\
"Content-Type: %s\r\n"\
"Content-Length: %d\r\n"\
"\r\n"

enum log_type {
  ERROR,
  WARN,
  INFO
};

void log_to_file(const enum log_type type, const char* message)
{
  FILE* log = fopen("server.log", "a+");

  if (log == NULL) {
    return;
  }

  switch (type) {
    case ERROR:
      (void)fprintf(log, "[ERROR]: %s\n", message);
      break;
    case WARN:
      (void)fprintf(log, "[WARN]: %s\n", message);
      break;
    case INFO:
      (void)fprintf(log, "[INFO]: %s\n", message);
      break;
  }

  (void)fflush(log);
}

/* Sends a 404 error to client */
int send_404(int socket_fd)
{
  (void)write(socket_fd, HTTP_404, strlen(HTTP_404));
  sleep(1);
  return 0;
}

/* Processes a user's request */
int serve_client(int socket_fd)
{
  FILE* file = 0;
  int file_length = 0;
  size_t i = 0, read_size = 0, ext_size = 0;
  static char buffer[BUFFER_SIZE + 1];
  char const * content_type = (char*)0;

  /* Read the clients request */
  if ((read_size = read(socket_fd, buffer, BUFFER_SIZE)) < 1) {
    log_to_file(WARN, "Unable to read client's request");
    return send_404(socket_fd);
  }

  buffer[read_size] = 0; /* Ensures the buffer is null terminated */

  /* Check request is a GET request */
  if (strncasecmp(buffer, "GET ", 4) != 0) {
    return send_404(socket_fd);
  }

  /* End buffer after the path in the GET request */
  for (i = 4; i < BUFFER_SIZE; i++) {
    if (buffer[i] == ' ') {
      buffer[i] = 0;
      read_size = i;
      break;
    }
  }

  /* Ensure the request path does not contain .. */
  for (i = 4; i < read_size - 1; i++) {
    if (buffer[i] == '.' && buffer[i + 1] == '.') {
      return send_404(socket_fd);
    }
  }

  /* Convert empty / to /index.html */
  if (strncasecmp(buffer, "GET /\0", 6) == 0) {
    (void)strcpy(buffer, "GET /index.html");
    read_size = 16;
  }

  /* Check if the file extension is supported */
  for (i = 0; extensions[i].ext != 0; i++) {
    ext_size = strlen(extensions[i].ext);
    if (read_size - ext_size > 0) {
      if (strncmp(&buffer[read_size - ext_size - 1], extensions[i].ext,
                  ext_size) == 0)
      {
          content_type = extensions[i].mime;
      }
    }
  }

  /* If content type is null then the content type is not supported */
  if (content_type == 0) {
    return send_404(socket_fd);
  }

  file = fopen(&buffer[5], "r");
  if (file == NULL) {
    return send_404(socket_fd);
  }

  (void)fseek(file, 0, SEEK_END);
  file_length = ftell(file);
  (void)fseek(file, 0, SEEK_SET);

  char* file_input = malloc(file_length);
  (void)fread(file_input, 1, file_length, file);

  (void)dprintf(socket_fd, HTTP_200, content_type, file_length);
  (void)write(socket_fd, file_input, file_length);

  (void)fclose(file);
  free(file_input);
  sleep(1);
  return 0;
}

/* Starts listening for connections */
int start_server()
{
  static struct sigaction ignore;
  static struct sockaddr_in client_address;
  static struct sockaddr_in server_address;
  int listen_fd = 0;
  int socket_fd = 0;
  socklen_t client_address_size = 0;

  /* Set response to certain signals */
  ignore.sa_handler = SIG_IGN;
  (void)sigaction(SIGCHLD, &ignore, NULL); /* Ignore child death     */
  (void)sigaction(SIGHUP, &ignore, NULL);  /* Ignore terminal hangup */
  (void)setpgrp();

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    log_to_file(ERROR, "Unable to create socket file descriptor");
    return -1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(PORT);

  /* Bind this program to the port */
  if (bind(listen_fd, (struct sockaddr*)&server_address,
           sizeof(server_address)) == -1)
  {
    log_to_file(ERROR, "Failed to bind to the port!");
    return -1;
  }

  /* Mark listen_fd as a listening socket */
  if (listen(listen_fd, CONNECTION_BACKLOG_SIZE) == -1) {
    log_to_file(ERROR, "Failed to ready socket for listening!");
  }

  client_address_size = sizeof(client_address);
  while (1) {
    if ((socket_fd = accept(listen_fd, (struct sockaddr*)&client_address,
                            &client_address_size)) == -1)
    {
      log_to_file(WARN, "Failed to accept a connection!");
    }

    if (fork() == 0) {
      (void)close(listen_fd);
      return serve_client(socket_fd);
    } else {
      (void)close(socket_fd);
    }
  }
}

/* Entry point for the program */
int main()
{
  if (chdir(WEBSITE_DIR) == -1) {
    printf("Failed to change directory to %s\n", WEBSITE_DIR);
    return -1;
  }

  if (fork() != 0) {
    return 0;
  }
  return start_server();
}

