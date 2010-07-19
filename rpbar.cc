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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "listener.hh"
#include "settings.hh"
#include "rpbar.hh"

namespace rpbar
{

int RpBar::run() {
  listener.start();
  this->color(FL_BLACK);
  Fl_Pack* pack = new Fl_Pack(0, 0, this->w(), this->h());
  pack->type(Fl_Pack::HORIZONTAL);
  //pack->spacing(5);
  pack->init_sizes();
  this->add(pack);

  Fl::add_timeout(0.1, static_timeout_cb, this);
  int fd = listener.get_fd();
  Fl::add_fd(fd, static_fd_cb, this);

  // docs say I should use this for double buffering
  Fl::visual(FL_DOUBLE|FL_INDEX);
  this->show();
  return Fl::run();
}

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
    perror("popen failed");
    exit(1);
  }
  while(fgets(buffer, RPBAR_BUFSIZE, stream)) {
    rstrip(buffer);
    windows.push_back(std::string(buffer));
  }
  pclose(stream);
}

void RpBar::refresh(){
  get_rp_info();

  int button_width_pixels = screen_width/windows.size();
  size_t max_label_chars = button_width_pixels/RPBAR_PIXELS_PER_CHAR;
  int curx = 5;

  Fl_Pack *pack = (Fl_Pack *)(this->child(0));
  pack->clear();

  // TODO consider using a list<> instead of vector<>
  for (std::vector<std::string>::iterator itr = windows.begin();
       itr != windows.end();
       ++itr) {
    std::string& button_label(*itr);
    bool is_main_win = button_label[button_label.length()-1]=='*';
    button_label.erase(button_label.length()-1);

    if (button_label.length() > max_label_chars) {
      button_label.erase(max_label_chars+1, std::string::npos);
    }
    // replace @ by # because FLTK has a special meaning for @. 
    // TODO replace @ by @@
    std::replace(button_label.begin(), button_label.end(), '@', '#');

    //x y w h
    Fl_Button* button = new Fl_Button(curx, 1, button_width_pixels, RPBAR_BARHEIGHT); 
    button->copy_label(button_label.c_str());
    button->box(FL_BORDER_BOX);
    button->labelsize(RPBAR_LABEL_SIZE);
    button->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP | FL_ALIGN_INSIDE);
    if (is_main_win) {
      // main window
      button->color(mainbgcolor);
      button->labelcolor(mainfgcolor);
    } else { 
      // ordinary window
      button->color(bgcolor);
      button->labelcolor(fgcolor);
    }
    button->callback(button_cb);
    pack->add(button);
    curx += button_width_pixels;
  }
  pack->init_sizes();
  pack->redraw();
}

void button_cb(Fl_Widget* o, void* data) {
  Fl_Button* b=(Fl_Button*) o;
  //std::cout << "Pressed: " << b->label() << std::endl;
  std::string cmd("ratpoison -c \"select ");

  const char * blabel = b->label();
  size_t num_end_pos = 0;
  while (isdigit(blabel[num_end_pos])) {
    ++num_end_pos;
  }
  cmd.append(blabel, num_end_pos);
  cmd.append("\"");
  if(system(cmd.c_str())==-1) {
    perror("system");
    exit(1);
  }
}

} /* end namespace rpbar */

int main(int argc, const char *argv[]) {
  rpbar::RpBar *rpbar = new rpbar::RpBar();
  int r = rpbar->run();
  return r;
}


