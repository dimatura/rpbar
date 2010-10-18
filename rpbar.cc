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
#include "rpbar.hh"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/time.h>

#include <algorithm>

namespace rpbar
{

unsigned long RpBar::get_color(const char *colstr) {
	Colormap cmap = DefaultColormap(display, screen);
	XColor color;
	if(!XAllocNamedColor(display, cmap, colstr, &color, &color)) {
    std::stringstream ss; ss << "Can not allocate color " << colstr;
    throw RpBarException(ss.str());
  }
	return color.pixel;
}

int RpBar::text_width(const std::string& text) {
	XRectangle rect;
	if(font.set) {
		XmbTextExtents(font.set, text.c_str(), text.length(), NULL, &rect);
    return rect.width; 
  }
	return XTextWidth(font.xfont, text.c_str(), text.length());
}

RpBar::~RpBar() {
  unlink(RPBAR_SOCKET_PATH);
	if(font.set) {
		XFreeFontSet(display, font.set);
  }	else {
		XFreeFont(display, font.xfont);
  }
	XFreePixmap(display, drawable);
	XFreeGC(display, gc);
	XDestroyWindow(display, win);
	XCloseDisplay(display);
  close(sock_fd);
}

void RpBar::handle_fd() {
  int numbytes;
  if ((numbytes = recv(sock_fd,
                       buffer,
                       RPBAR_BUFSIZE-1,
                       0))==-1) {
    throw RpBarException("recv failed");
  }
  buffer[numbytes] = '\0';
  // for now, ignore actual contents of message
  refresh();
}

void RpBar::handle_timeout() {
  refresh();
}

void RpBar::handle_xev() {
  XEvent ev;
  int win_ix;
  while(XPending(display)) {
    XNextEvent(display, &ev);
    switch (ev.type) {
      case Expose:
        if (ev.xexpose.count == 0) {
          refresh();
        }
        break;
      case ButtonPress:
        // figure out which 'button' was pressed
        win_ix = (ev.xbutton.x*windows.size())/bar_w;
        select_window(win_ix);
        break;
      default:
        break;
    }
  }
}

void RpBar::init_socket() {
  if ((sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    throw RpBarException("Error creating socket");
  }
  struct sockaddr_un servaddr;
  memset(&servaddr, NULL, sizeof(servaddr));
  servaddr.sun_family = AF_UNIX;
  strcpy(servaddr.sun_path, RPBAR_SOCKET_PATH);
  unlink(RPBAR_SOCKET_PATH);
  if (bind(sock_fd,
           (struct sockaddr *) &servaddr,
           sizeof(servaddr)) < 0) {
    throw RpBarException("Error binding socket");
  }
}

void RpBar::init_font(const char *fontstr) {
  // This function is pretty much copy+pasted from A. Garbe's dmenu
  // (http://tools.suckless.org/dmenu)
  if(!fontstr || fontstr[0] == '\0') {
    std::stringstream ss; ss << "Can not load font " << fontstr;
    throw RpBarException(ss.str());
  }
  char **missing = NULL;
  char *def;
  int i,n;
  font.set = XCreateFontSet(display, fontstr, &missing, &n, &def);
  if(missing) {
    XFreeStringList(missing);
  }
  if(font.set) {
    XFontSetExtents *font_extents;
    XFontStruct **xfonts;
    char **font_names;
    font.ascent = font.descent = 0;
    font_extents = XExtentsOfFontSet(font.set);
    n = XFontsOfFontSet(font.set, &xfonts, &font_names);
    for(i = 0, font.ascent = 0, font.descent = 0; i < n; i++) {
      if(font.ascent < (*xfonts)->ascent)
        font.ascent = (*xfonts)->ascent;
      if(font.descent < (*xfonts)->descent)
        font.descent = (*xfonts)->descent;
      xfonts++;
    }
  } else {
    if(!(font.xfont = XLoadQueryFont(display, fontstr))
       && !(font.xfont = XLoadQueryFont(display, "fixed"))) {
      std::stringstream ss; ss << "Can not load font " << fontstr;
      throw RpBarException(ss.str());
    }
    font.ascent = font.xfont->ascent;
    font.descent = font.xfont->descent;
  }
  font.height = font.ascent + font.descent;
}

void RpBar::init_gui() {
  // This function has some copy+paste from A. Garbe's dmenu
  // (http://tools.suckless.org/dmenu)
  if (!(display = XOpenDisplay(0))) {
    throw RpBarException("Cannot open display\n");
  }
	screen = DefaultScreen(display);
	root = RootWindow(display, screen);
  XSetWindowAttributes window_attribs;
  bordercolor = get_color(RPBAR_BORDERCOLOR);
  bgcolor = get_color(RPBAR_BGCOLOR);
  fgcolor = get_color(RPBAR_FGCOLOR);
  mainbgcolor = get_color(RPBAR_MAINBGCOLOR);
  mainfgcolor = get_color(RPBAR_MAINFGCOLOR);
  init_font(RPBAR_FONT_STR);
  window_attribs.override_redirect = 1;
  window_attribs.background_pixmap = ParentRelative;
  window_attribs.event_mask = ExposureMask | ButtonPressMask;
  bar_h = font.height + RPBAR_PADDING; 
  bar_x = 0;
  bar_y = DisplayHeight(display, screen) - bar_h;
  bar_w = DisplayWidth(display, screen);
  win = XCreateWindow(display, root, bar_x, bar_y, bar_w, bar_h, 0,
                      DefaultDepth(display, screen), CopyFromParent,
                      DefaultVisual(display, screen), CWOverrideRedirect |
                      CWBackPixmap | CWEventMask, &window_attribs);
  drawable = XCreatePixmap(display, root, bar_w, bar_h, DefaultDepth(display,
                                                                     screen));
  gc = XCreateGC(display, root, 0, 0);
  XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinMiter);
  if (!font.set) {
    XSetFont(display, gc, font.xfont->fid);
  }
  XMapRaised(display, win);
	refresh();
	XSync(display, false);
  x11_fd = ConnectionNumber(display);
}

void RpBar::run() {
  init_socket();
  init_gui();
  struct timeval timeout;
  while (true) {
    FD_ZERO(&fds);
    FD_SET(x11_fd, &fds);
    FD_SET(sock_fd, &fds);
    timeout.tv_usec = 0;
    timeout.tv_sec = RPBAR_TIMEOUT_S;
    // the 'max' is because 'select' checks the first n-1 first fd's.
    int ret = select(std::max(x11_fd,sock_fd)+1, &fds, 0, 0, &timeout);
    if (ret < 0) {
      throw RpBarException("Error on select");
    } else if (FD_ISSET(x11_fd, &fds)) {
      handle_xev();
    } else if (FD_ISSET(sock_fd, &fds)) {
      handle_fd();
    } else {
      handle_timeout();
    }
  }
}

// strip whitespace from the right.
void rstrip(char *s) {
  size_t n = strlen(s);
  do {
    --n;
  } while (n >= 0 && isspace(s[n]));
  s[n+1] = '\0';
}

void RpBar::get_rp_info() {
  //%n = number
  //%s = status
  //%% = %
  //%t = title
  //%f = frame number
  //%a = application name
  //%c = resource class
  windows.clear();
  const char* cmd = "ratpoison -c \"windows \%n \%t\%s\"";
  FILE* stream;
  if ((stream = popen(cmd, "r"))==NULL) {
    throw RpBarException("popen failed");
  }
  while(fgets(buffer, RPBAR_BUFSIZE, stream)) {
    rstrip(buffer);
    windows.push_back(std::string(buffer));
  }
  pclose(stream);
}

void RpBar::refresh(){
  get_rp_info();
  // blank out rectangle
	XRectangle rect = { 0, 0, bar_w, bar_h };
	XSetForeground(display, gc, bordercolor);
	XFillRectangles(display, drawable, gc, &rect, 1);
  
  int button_width_px = bar_w/windows.size();
  int curx = 1;

  for (std::vector<std::string>::iterator itr = windows.begin();
       itr != windows.end();
       ++itr) {
    std::string& button_label(*itr);
    bool is_main_win = button_label[button_label.length()-1]=='*';
    button_label.erase(button_label.length()-1);

    //decide colors
    unsigned long bg, fg;
    if (is_main_win) {
      // main window
      bg = mainbgcolor;
      fg = mainfgcolor;
    } else { 
      // ordinary window
      bg = bgcolor;
      fg = fgcolor;
    }
    
    // shave off characters until the width is acceptable
    while (text_width(button_label) > 
           (button_width_px - RPBAR_BUTTON_MARGIN)) {
      button_label.erase(button_label.length()-1);
    }

    //x y w h
    XRectangle rect2 = { curx, 1, button_width_px, bar_h };
    XSetForeground(display, gc, bg);
    XFillRectangles(display, drawable, gc, &rect2, 1);
    //TODO use bar_h?
    int h = font.ascent + font.descent;
    int x = curx + (button_width_px - text_width(button_label))/2 + (h/2);
    int y = (bar_h / 2) - (h / 2) + font.ascent;

    XSetForeground(display, gc, fg);
    if(font.set) {
      XmbDrawString(display, drawable, font.set, gc, x, y,
                    button_label.c_str(), button_label.length());
    } else {
      XDrawString(display, drawable, gc, x, y, button_label.c_str(),
                  button_label.length());
    }
    curx += button_width_px+1;
  }
	XCopyArea(display, drawable, win, gc, 0, 0, bar_w, bar_h, 0, 0);
	XFlush(display);
}

void RpBar::select_window(int win_ix) {
  std::string cmd("ratpoison -c \"select ");
  std::string win(windows.at(win_ix));
  size_t num_end_pos = 0;
  while (num_end_pos < win.length() && 
         isdigit(win.at(num_end_pos))) {
    ++num_end_pos;
  }
  cmd.append(win.c_str(), num_end_pos);
  cmd.append("\"");
  if(system(cmd.c_str())==-1) {
    throw RpBarException("system call failed");
  }
}

} /* end namespace rpbar */

int main(int argc, const char *argv[]) {
  rpbar::RpBar rpbar;
  rpbar.run();
  // TODO catch exceptions? It wouldn't accomplish much.
  // Leaving them at least allows core dump examination.
  return 0;
}
