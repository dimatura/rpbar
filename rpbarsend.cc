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

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

// TODO just use c stuff
#include <string>
#include <sstream>

#include "settings.hh"

int main(int argc, const char *argv[]) {
  const char *default_message = "m";
  const char *message;
  if (argc == 2) {
    message = argv[1];
  } else {
    message = default_message;
  }

  int sockfd;
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
      perror("socket");
      exit(2);
  }

  struct sockaddr_un servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sun_family = AF_UNIX;

  uid_t uid = geteuid();
  std::stringstream ss;
  ss << RPBAR_SOCKET_PATH << "-" << uid;
  std::string socket_path(ss.str());

  strcpy(servaddr.sun_path, socket_path.c_str());
  int numbytes;
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
