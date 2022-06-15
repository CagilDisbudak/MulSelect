#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
  int sockfd, nread;
  char buf[128], enter, resp, IP[16];
  fd_set fds;
  struct sockaddr_in server;

  printf("Enter IP address of the server: ");
  scanf("%s%c", IP, &enter);
  server.sin_family = AF_INET;
  server.sin_port = htons(2000);
  server.sin_addr.s_addr = inet_addr(IP);
  memset(server.sin_zero, '\0', 8);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return 1;
  }

  if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
    perror("connect");
    return 1;
  }

  printf("Enter a message (E to exit)\n");
  for (;;) {
    FD_ZERO(&fds);        // clear file descriptor set
    FD_SET(sockfd, &fds); // add server to file descriptor set
    FD_SET(0, &fds);      // add stdin to file descriptor set
    /* Wait for some input. */
    select(sockfd + 1, &fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

    if (FD_ISSET(sockfd, &fds)) { // if a message has been sent from server
      nread = recv(sockfd, buf, sizeof(buf), 0);

      if (nread < 1) {
        close(sockfd);
        return 0;
      }
      buf[nread] = 0;
      printf("%s", buf);
    }

    if (FD_ISSET(0, &fds)) { // if user entered from keyboard
      nread = read(0, buf, sizeof(buf));
      if (nread < 1) { // terminate if error or eof
        close(sockfd);
        return 0;
      } else if ((buf[0] == 'e' || buf[0] == 'E') && nread == 2) {
        close(sockfd);
        return 0;
      } else
        send(sockfd, buf, nread, 0);
    }
  }
}
