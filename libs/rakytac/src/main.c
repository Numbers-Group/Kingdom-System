#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

const char POST[] = "POST";
const char GET[] = "GET";
const char NOT_FOUND[] = "404 NOT FOUND";

/* REST API Routers */
void teachers_router(char resp[]) {
  strcpy(resp, "teachers {1,2,3,4}");
}

void health_router(char resp[]) {
  strcpy(resp, "health: good");
}

void not_found_router(char resp[]) {
  strcpy(resp, NOT_FOUND);
}

/* Main router */
void rakytac_router(char method[], char uri[], char resp[]) {
  
  char *strings[100];
  strings[1] = "/teachers";
  strings[2] = "/health";
  
  if (strcmp(method, POST) == 0)
    printf("POST_REQUEST\n");

  
  if (strcmp(method, GET) == 0) {
    printf("GET_REQUEST\n");
    if (strcmp(uri, strings[1]) == 0) {
      teachers_router(resp);
    } else if (strcmp(uri, strings[2]) == 0) {
      health_router(resp);
    } else {
      not_found_router(resp);
    }
  }
}

int main(int argc, char* argv[]) {
  char buffer[BUFFER_SIZE];
  
  char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>"
                  ""
                  "</html>\r\n";
  
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("[ RAKYTAC ]=> webserver (socket)");
    return 1;
  }
  printf("[ RAKYTAC ]=> socket created successfully\n");
  struct sockaddr_in host_addr;
  int host_addrlen = sizeof(host_addr);

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(DEFAULT_PORT);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
    perror("[ RAKYTAC ]=> webserver (bind)");
    return 1;
  }
  printf("[ RAKYTAC ]=> socket successfully bound to address\n");
  
  if (listen(sockfd, SOMAXCONN) != 0) {
    perror("[ RAKYTAC ]=> webserver (listen)");
    return 1;
  }
  printf("[ RAKYTAC ]=> server listening for connections\n");

  for (;;) {
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,
                           (socklen_t *)&host_addrlen);
    if (newsockfd < 0) {
      perror("[ RAKYTAC ]=> webserver (accept)");
      continue;
    }
    printf("[ RAKYTAC ]=> connection accepted\n");

    int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr,
                            (socklen_t *)&client_addrlen);
    if (sockn < 0) {
      perror("[ RAKYTAC ]=> webserver (getsockname)");
      continue;
    }

    int valread = read(newsockfd, buffer, BUFFER_SIZE);
    if (valread < 0) {
      perror("[ RAKYTAC ]=> webserver (read)");
      continue;
    }

    char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
    sscanf(buffer, "%s %s %s", method, uri, version);

    rakytac_router(method, uri, resp);
    
    printf("[ RAKYTAC ]=>[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),
           htons(client_addr.sin_port), method, version, uri);

    int valwrite = write(newsockfd, resp, strlen(resp));
    if (valwrite < 0) {
      perror("[ RAKYTAC ]=> webserver (write)");
      continue;
    }
    close(newsockfd);
  }
    
  return 0;
}
