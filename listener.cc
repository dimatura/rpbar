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

// heavily based on beej's network guide udp datagram example (public domain)

#include "listener.hh"

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

namespace rpbar
{

Listener::~Listener() {
  close(sockfd_);
}

int Listener::start() {
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4
	hints.ai_socktype = SOCK_DGRAM; // udp
	hints.ai_flags = AI_PASSIVE; // use my IP

  struct addrinfo *servinfo;
	int rv;
	if ((rv = getaddrinfo(NULL, RPBAR_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
  struct addrinfo *p;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd_ = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd_, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd_);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return -1;
	}
	freeaddrinfo(servinfo);
  return 0;
}

char Listener::listen() {
  int numbytes;
  if ((numbytes = recv(sockfd_, buf_, RPBAR_BUFSIZE-1 , 0))==-1) {
    perror("recv");
    return -1;
  }
  //buf_[numbytes] = '\0';
  return buf_[0];
}

} /*  */
