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

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <fontconfig/fontconfig.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "settings.hh"

namespace rpbar
{

class RpBarException : public std::runtime_error {
public:
  RpBarException(const std::string& what_arg) :
    std::runtime_error(what_arg) { }
};

class RpBar {
public:
  RpBar() { }
  ~RpBar();
  void run();

private:
  RpBar (const RpBar& other);
  RpBar& operator=(const RpBar& other);

  void init_socket();

  void
  init_font(const char *fontstr);

  XftFont *
  load_font_by_pattern(FcPattern * const);

  int
  get_font_height();

  void init_gui();

  void refresh();

  int
  draw_text(const int, const int, const char * const,
    const char * const, const bool);

  bool
  draw_character(XftDraw *, const XftColor,
    int * const, const int, const char * const, size_t * const,
    const bool);

  XftFont *
  load_font_for_codepoint(long);

  void handle_fd();
  void handle_timeout();
  void handle_xev();
  void select_window(int win_ix);

  void get_rp_info();

  int
  text_width_in_font(XftFont * const, const char * const,
    const int);

  int
  text_width(const char * const);

  unsigned long get_color(const char *colstr);

  // this class does too much stuff.

  int sock_fd, x11_fd;
  fd_set fds;
  std::string socket_path;

  int bar_x, bar_y, bar_w, bar_h;
  unsigned long bordercolor, bgcolor, fgcolor, mainbgcolor, mainfgcolor;

  char buffer[RPBAR_BUFSIZE];
  std::vector<std::string> windows;

  // X stuff
  Drawable drawable;
  GC gc;
  Display *display;
  int screen;
  Window root, win;
  std::vector<XftFont *> xft_fonts;
  FcPattern * fc_pattern;
};

void rstrip(char *s);
void rpbarsend(int argc, const char *argv[]);

} /*  */

#endif /* end of include guard: RPBAR_Y2DAPIQS */
