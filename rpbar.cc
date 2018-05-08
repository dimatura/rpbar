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

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>

#include "drw.h"

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

// text_width_in_font determines width of the given text in the given font.
//
// You should make sure the font has the characters for the characters you want
// to know the width of before using this function.
int
RpBar::text_width_in_font(XftFont * const xft_font, const char * const text,
  const int len)
{
  if (!xft_font || !text || strlen(text) == 0) {
    return -1;
  }

  XGlyphInfo glyph_info;
  memset(&glyph_info, 0, sizeof(XGlyphInfo));

  XftTextExtentsUtf8(display, xft_font, (FcChar8 *) text, len, &glyph_info);

  return glyph_info.xOff;
}

// text_width determines the width the text would have if displayed.
//
// It uses the same logic used in drawing the text itself, so it may use
// several fonts.
int
RpBar::text_width(const char * const text)
{
  if (!text || strlen(text) == 0) {
    return -1;
  }

  // x, y, color do not matter in this case.
  // This points to poor design, but mostly duplicating draw_text() to
  // get only the width part is not so nice either.
  const int x = 0;
  const int y = 0;
  const char * const color = NULL;

  // Don't actually draw anything.
  const bool render = false;

  return draw_text(x, y, text, color, render);
}

RpBar::~RpBar() {
  unlink(socket_path.c_str());

  if (display) {
    for (std::vector<XftFont *>::iterator i = xft_fonts.begin();
        i != xft_fonts.end();
        i++)
    {
      XftFont * xft_font = *i;
      XftFontClose(display, xft_font);
    }
  }

  if (fc_pattern) {
    FcPatternDestroy(fc_pattern);
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
        if (bar_w == 0) {
          break;
        }
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
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sun_family = AF_UNIX;
  // get user id to support multiple users at the same time
  // since the socket is named file in /tmp
  uid_t uid = geteuid();
  std::stringstream ss;
  ss << RPBAR_SOCKET_PATH << "-" << uid;
  socket_path = ss.str();

  strcpy(servaddr.sun_path, socket_path.c_str());
  unlink(socket_path.c_str());
  if (bind(sock_fd,
           (struct sockaddr *) &servaddr,
           sizeof(servaddr)) < 0) {
    throw RpBarException("Error binding socket");
  }
}

// init_font loads a single font, and sets up the FontConfig pattern so that
// we can load additional fonts if necessary.
void
RpBar::init_font(const char *fontstr) {
  if (!fontstr || strlen(fontstr) == 0) {
    throw RpBarException("init_font called without a font string");
  }

  XftFont * xft_font = XftFontOpenName(display, screen, fontstr);
  if (!xft_font) {
    throw RpBarException("Font not found");
  }

  xft_fonts.push_back(xft_font);

  fc_pattern = FcNameParse((FcChar8 *) fontstr);
  if (!fc_pattern) {
    throw RpBarException("Cannot parse font name to pattern");
  }
}

XftFont *
RpBar::load_font_by_pattern(FcPattern * const pattern)
{
  if (!pattern) {
    return NULL;
  }
  return XftFontOpenPattern(display, pattern);
}

int
RpBar::get_font_height()
{
  const XftFont * const xft_font = xft_fonts[0];
  return xft_font->ascent + xft_font->descent;
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
  mainbgcolor = get_color(RPBAR_MAINBGCOLOR);
  init_font(RPBAR_FONT_STR);
  window_attribs.override_redirect = 1;
  window_attribs.background_pixmap = ParentRelative;
  window_attribs.event_mask = ExposureMask | ButtonPressMask;
  bar_h = get_font_height() + RPBAR_PADDING;
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
  // TODO make sure this is The Right Way (tm)
  while(fgets(buffer, RPBAR_BUFSIZE, stream)) {
    rstrip(buffer);
    windows.push_back(std::string(buffer));
  }
  pclose(stream);
}

void RpBar::refresh(){
  get_rp_info();
  XSetForeground(display, gc, bordercolor);
  XFillRectangle(display, drawable, gc, 0, 0, bar_w, bar_h);

  int button_width = bar_w;
  if (windows.size() != 0) {
    button_width = bar_w/windows.size();
  }
  int curx = 0;

  for (std::vector<std::string>::iterator itr = windows.begin();
       itr != windows.end();
       ++itr) {
    std::string& button_label(*itr);
    char last_char = button_label[button_label.length()-1];
    // '*' -> main win
    // '+' -> 'alternate' win
    // '-' -> other
    // else, it's a 'no managed windows' message.
    if (last_char!='s') {
      button_label.erase(button_label.length()-1);
    }
    // highlight current window
    unsigned long bg;
    const char * fg_color = NULL;
    if (last_char=='*') {
      bg = mainbgcolor;
      fg_color = RPBAR_MAINFGCOLOR;
    } else {
      bg = bgcolor;
      fg_color = RPBAR_FGCOLOR;
    }

    // shave off characters until the width is acceptable
    while (text_width(button_label.c_str()) >
        button_width - 2*RPBAR_BUTTON_MARGIN) {
      button_label.erase(button_label.length()-1);
    }

    XSetForeground(display, gc, bg);
    // TODO handle this in a smarter way.
    int width = (itr==windows.end()-1)? bar_w - curx - 2 : button_width - 1;
    XFillRectangle(display, drawable, gc, curx+1, 1, width, bar_h-2);

    int x = curx + (button_width - text_width(button_label.c_str()))/2;
    int y = (bar_h / 2) - (get_font_height() / 2) + xft_fonts[0]->ascent;

    const bool render = true;
    draw_text(x, y, button_label.c_str(), fg_color, render);

    curx += button_width;
  }
  XCopyArea(display, drawable, win, gc, 0, 0, bar_w, bar_h, 0, 0);
  XFlush(display);
}

// draw_text serves two purposes.
//
// First, you can draw text to the screen by passing render true.
//
// Second, by passing render false, you can determine the actual width
// the text would have if displayed.
//
// It returns the width of the text. It returns -1 on failure.
int
RpBar::draw_text(const int x, const int y, const char * const text,
  const char * const color, const bool render)
{
  // color may be NULL. It's only relevant if we're rendering.
  if (!text || strlen(text) == 0) {
    return -1;
  }
  if (render && !color) {
    return -1;
  }

  XftDraw * xft_draw = NULL;
  XftColor xft_color;
  memset(&xft_color, 0, sizeof(XftColor));

  if (render) {
    xft_draw = XftDrawCreate(display, drawable,
        DefaultVisual(display, screen), DefaultColormap(display, screen));
    if (!xft_draw) {
      return -1;
    }

    if (!XftColorAllocName(display, DefaultVisual(display, screen),
          DefaultColormap(display, screen), color, &xft_color)) {
      XftDrawDestroy(xft_draw);
      return -1;
    }
  }

  int cur_x = x;

  for (size_t i = 0; i < strlen(text); ) {
    draw_character(xft_draw, xft_color, &cur_x, y, text, &i, render);
  }

  if (render) {
    XftColorFree(display, DefaultVisual(display, screen),
      DefaultColormap(display, screen), &xft_color);
    XftDrawDestroy(xft_draw);
  }

  return cur_x-x;
}

bool
RpBar::draw_character(XftDraw * xft_draw, const XftColor xft_color,
  int * const x, const int y, const char * const text, size_t * const pos,
  const bool render)
{
  // xft_draw is only required if we are rendering.
  if (!text || strlen(text) == 0 || !x || !pos) {
    return false;
  }
  if (render && !xft_draw) {
    return false;
  }

  // Find the codepoint to draw.
  // We do this so we can check if the font has it to draw.
  long utf8codepoint = 0;
  const int utf8charlen = utf8decode(text+*pos, &utf8codepoint, UTF_SIZ);

  // Try to render the character in a font we've loaded.
  for (std::vector<XftFont *>::iterator i = xft_fonts.begin();
      i != xft_fonts.end();
      i++)
  {
    XftFont * const xft_font = *i;

    if (XftCharExists(display, xft_font, utf8codepoint)) {
      if (render) {
        XftDrawStringUtf8(xft_draw, &xft_color, xft_font, *x, y,
            (FcChar8 *) text+*pos, utf8charlen);
      }
      *x += text_width_in_font(xft_font, text+*pos, utf8charlen);
      *pos += utf8charlen;
      return true;
    }
  }

  // No font yet loaded has the character.
  //
  // We try to load another font that does have it and use that.
  // We use the pattern we were given initially to find another font.
  //
  // If we can't load another font, or still cannot find the character, we
  // currently skip over the character and draw nothing.

  XftFont * new_font = load_font_for_codepoint(utf8codepoint);
  if (!new_font) {
    *pos += utf8charlen;
    return false;
  }

  if (XftCharExists(display, new_font, utf8codepoint)) {
    if (render) {
      XftDrawStringUtf8(xft_draw, &xft_color, new_font, *x, y,
          (FcChar8 *) text+*pos, utf8charlen);
    }
    *x += text_width_in_font(new_font, text+*pos, utf8charlen);
    *pos += utf8charlen;
    return true;
  }

  *pos += utf8charlen;

  return false;
}

// load_font_for_codepoint attempts to load another font given the
// codepoint. We use the instance provided font config pattern.
//
// You don't need to clean up the returned font. We keep it around in
// the object and clean it up in our destructor.
//
// Note much of this is taken with dmenu's drw.c drw_text() as reference.
XftFont *
RpBar::load_font_for_codepoint(long codepoint)
{
  FcCharSet * fc_charset = FcCharSetCreate();
  if (!fc_charset) {
    return NULL;
  }

  if (!FcCharSetAddChar(fc_charset, codepoint)) {
    FcCharSetDestroy(fc_charset);
    return NULL;
  }

  FcPattern * fc_pattern_dup = FcPatternDuplicate(fc_pattern);
  if (!fc_pattern_dup) {
    FcCharSetDestroy(fc_charset);
    return NULL;
  }

  if (!FcPatternAddCharSet(fc_pattern_dup, FC_CHARSET, fc_charset)) {
    FcCharSetDestroy(fc_charset);
    FcPatternDestroy(fc_pattern_dup);
    return NULL;
  }

  if (!FcConfigSubstitute(NULL, fc_pattern_dup, FcMatchPattern)) {
    FcCharSetDestroy(fc_charset);
    FcPatternDestroy(fc_pattern_dup);
    return NULL;
  }

  FcDefaultSubstitute(fc_pattern_dup);

  FcResult fc_result;
  memset(&fc_result, 0, sizeof(FcResult));

  FcPattern * fc_pattern_match = XftFontMatch(display, screen, fc_pattern_dup,
      &fc_result);
  if (!fc_pattern_match) {
    FcCharSetDestroy(fc_charset);
    FcPatternDestroy(fc_pattern_dup);
    return NULL;
  }

  FcCharSetDestroy(fc_charset);
  fc_charset = NULL;
  FcPatternDestroy(fc_pattern_dup);
  fc_pattern_dup = NULL;

  XftFont * new_font = load_font_by_pattern(fc_pattern_match);
  if (!new_font) {
    FcPatternDestroy(fc_pattern_match);
    return NULL;
  }

  FcPatternDestroy(fc_pattern_match);
  fc_pattern_match = NULL;

  xft_fonts.push_back(new_font);

  return new_font;
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
  setlocale(LC_CTYPE, "");
  rpbar::RpBar rpbar;
  rpbar.run();

  // TODO catch exceptions? It wouldn't accomplish much.
  // Leaving them at least allows core dump examination.

  FcConfig * const fc_config = FcConfigGetCurrent();
  FcConfigDestroy(fc_config);

  return 0;
}
