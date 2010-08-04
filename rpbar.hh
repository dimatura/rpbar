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

#ifndef RPBAR_Y2DAPIQS
#define RPBAR_Y2DAPIQS

#include <unistd.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>

#include "settings.hh"

namespace rpbar
{

class RpBar : public Fl_Double_Window {
public:
  RpBar () : 
      Fl_Double_Window(0, 
                Fl::h()-RPBAR_BARHEIGHT,
                Fl::w(),
                RPBAR_BARHEIGHT,
                RPBAR_WIN_NAME),
      sock_fd(-1),
      screen_width(Fl::w()),
      screen_height(Fl::h()),
      bgcolor(RPBAR_BGCOLOR),
      fgcolor(RPBAR_FGCOLOR),
      mainbgcolor(RPBAR_MAINBGCOLOR),
      mainfgcolor(RPBAR_MAINFGCOLOR)
  { }

  int run();

private:
  RpBar (const RpBar& other);
  RpBar& operator=(const RpBar& other);

  static void static_timeout_cb(void *data) {
    ((RpBar *)data)->timeout_cb();
  }

  static void static_fd_cb(int fd, void *data) {
    ((RpBar *)data)->fd_cb();
  }

  static void static_button_cb(Fl_Widget *o, void *data) {
    ((RpBar *)data)->button_cb(o, data);
  }

  bool init_socket();

  bool init_gui();

  void fd_cb();

  void timeout_cb();

  void button_cb(Fl_Widget* o, void* data);

  void refresh();

  virtual ~RpBar();

  void get_rp_info();

  int sock_fd;
  int screen_width;
  int screen_height;
  Fl_Color bgcolor, fgcolor, mainbgcolor, mainfgcolor;
  char buffer[RPBAR_BUFSIZE];
  std::vector<std::string> windows;
};

void rstrip(char *s);

} /*  */

#endif /* end of include guard: RPBAR_Y2DAPIQS */
