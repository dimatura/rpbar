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

#ifndef LISTENER_JG6Z7DQK
#define LISTENER_JG6Z7DQK

#include "settings.hh"

namespace rpbar
{

class Listener {
public:
  Listener () :
      sockfd_(-1)
  {}

  int start();
  char listen();
  int get_fd() { return sockfd_; }

  virtual ~Listener();
private:
  int sockfd_;
	char buf_[RPBAR_BUFSIZE];

  Listener (const Listener& other);
  Listener& operator=(const Listener& other);
  
};

} /*  */

#endif /* end of include guard: LISTENER_JG6Z7DQK */
