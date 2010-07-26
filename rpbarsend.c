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

/*
 * based on talker.c fron beej's network guide, which is public 
 * domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "settings.hh"

int main(int argc, const char *argv[]) {
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;

  const char *default_message = "m";

  const char *message;
  if (argc == 2) {
    message = argv[1];
  } else {
    message = default_message;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo("localhost", RPBAR_PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1) {
      perror("rpbarsend: socket");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "failed to bind socket\n");
    return 2;
  }

  if ((numbytes = sendto(sockfd, message, strlen(message), 0,
                         p->ai_addr, p->ai_addrlen)) == -1) {
    perror("sendto");
    exit(1);
  }

  freeaddrinfo(servinfo);

  close(sockfd);
  return 0;
}

