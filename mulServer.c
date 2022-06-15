#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX 5

int client[MAX];
int ActiveClients = 0;

void findMax(int *maxfd) {
  *maxfd = client[0];
  for (int i = 1; i < MAX; i++)
    if (client[i] > *maxfd)
      *maxfd = client[i];
}

int main() {
  int sockfd, maxfd, nread, found, i, j, yes = 1, sinsize = sizeof(struct sockaddr_in);
  char buf[128];
  fd_set fds;
  struct sockaddr_in server, clients[MAX];

  server.sin_family = AF_INET;
  server.sin_port = htons(2000);
  server.sin_addr.s_addr = INADDR_ANY;
  memset(server.sin_zero, '\0', 8);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }
  if (bind(sockfd, (struct sockaddr *)&server, sinsize) == -1) {
    perror("bind");
    return 1;
  }
  if (listen(sockfd, 5) == -1) {
    perror("listen");
    return 1;
  }

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  for (;;) {
    findMax(&maxfd);
    maxfd = (maxfd > sockfd ? maxfd : sockfd) + 1;
    FD_ZERO(&fds);        // clear file descriptor set
    FD_SET(sockfd, &fds); // add sockfd to file descriptor set
    for (i = 0; i < MAX; i++)
      if (client[i] != 0)
        FD_SET(client[i], &fds); // add each available client to file descriptor set

    /* wait for some input or connection request */
    select(maxfd, &fds, NULL, NULL, NULL);

    /* if there is a request for a new connection */
    if (FD_ISSET(sockfd, &fds)) {
      /* accept the request if nr of active clients is less than MAX */
      if (ActiveClients < MAX) {
        found = 0;
        for (i = 0; i < MAX && !found; i++) {
          if (client[i] == 0) {
            client[i] = accept(sockfd, (struct sockaddr *)&clients[i], (socklen_t *)&sinsize);
            found = 1;
            ActiveClients++;
            printf("%s:%d connected\n", inet_ntoa(clients[i].sin_addr), ntohs(clients[i].sin_port));
          }
        }
      }
    }

    /* if one of the clients has some input, read and send it to all others */
    for (i = 0; i < MAX; i++) {
      if (FD_ISSET(client[i], &fds)) {
        nread = recv(client[i], buf, sizeof(buf), 0);
        /* if error or eof, terminate the connection */
        if (nread < 1) {
          close(client[i]);
          client[i] = 0;
          ActiveClients--;
          printf("%s:%d disconnected\n", inet_ntoa(clients[i].sin_addr), ntohs(clients[i].sin_port));
        } else {
          /* broadcast the message */
          for (j = 0; j < MAX; j++)
            if (client[j] != 0 && i != j)
              send(client[j], buf, nread, 0);
        }
      }
    }
  }
}
