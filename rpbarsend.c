//  Copyright (C) 2010 Daniel Maturana
//  This file is part of rpbar.
// 
//  rpbar is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  rpbar is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with rpbar. If not, see <http://www.gnu.org/licenses/>.
// 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "settings.hh"

int main(int argc, const char *argv[]) {
  int sockfd;
  struct sockaddr_un servaddr;
  int numbytes;
  const char *default_message = "m";
  const char *message;

  if (argc == 2) {
    message = argv[1];
  } else {
    message = default_message;
  }

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
      perror("client: socket");
      exit(2);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sun_family = AF_UNIX;
  strcpy(servaddr.sun_path, RPBAR_SOCKET_PATH);

  if ((numbytes = sendto(sockfd,
                         message,
                         strlen(message),
                         0,
                         (struct sockaddr *) &servaddr,
                         sizeof(servaddr))) == -1) {
    perror("sendto");
    exit(1);
  }

  close(sockfd);
  return 0;
}

