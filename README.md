
rpbar
======

Intro
-----

[Here's a screenshot](http://github.com/downloads/dimatura/rpbar/shot.png).
rpbar is the small bar at the bottom.

The 'default' way to switch windows in Ratpoison is to press C-t-w to get the
window list, find the number of the window you want to switch to (let's say
it's 2), and press C-t-2 to switch. rpbar is a simple taskbar that gives a
permanent display of the windows in the current group in order to make the
first C-t-w unnecesary. As a concession to The Rat, clicking on a window title
will select that window.

The main inspiration is the simple task bars you get in other minimalistic
window managers such as [Awesome](http://awesome.naquadah.org) or
(wmii)[http://wmii.suckless.org]. rpbars appearance is modeled on these task
bars.

The other inspiration is [ratbar.pl](http://xenotrout.com/prog/ratbar/).
rpbar does less that ratbar.pl, but does it better. As far as I can tell,
ratbar.pl uses polling to update its window list. This makes it unresponsive,
which is very annoying. rpbar uses Ratpoison's hook feature to refresh as
soon as you select windows, close windows, change groups, etc. It's not
as fast as it would be if it were part of Ratpoison, but it's pretty usable.

How it works
-------------
rpbar is a pretty simple C++ project. It consists of two executables:

- `rpbar` gets the window list from Ratpoison and displays
  it using the [FLTK](http://www.fltk.org) library
- `rpbarsend` sends a message to `rpbar`, telling it to refresh its window
  list. It should be invoked by Ratpoison hooks.

Compilation and usage
---------------------

Obviously, you should have a working g++ toolchain.  You'll also need the
[FLTK](http://www.fltk.org) library, version 1.1.x (I'm using 1.1.9).  If
you're using Debian or Ubuntu, what you need is in libfltk and libfltk-dev.

Then follow this steps:

1. Type `make` to compile.
1. Put `rpbar` and `rpbarsend` in your path. 
1. Add the following to your `.ratpoisonrc`:

        # tell ratpoison to ignore rpbar
        unmanage rpbar
        # start rpbar 
        exec rpbar
        # hooks
        addhook switchwin exec rpbarsend
        addhook switchframe exec rpbarsend
        addhook switchgroup exec rpbarsend
        addhook deletewindow exec rpbarsend

1. Restart Ratpoison or manually execute `rpbar` (it should run in the background).

Now, what if you don't like the colors? The font size? The bar height?  Right
now, the only way to change them is changing `settings.hh` and recompiling, a
la dwm. I know, it's unfriendly. I'll eventually add the capability to read a
config file at startup and/or take command line arguments.

Status
----------

I consider rpbar to be alpha. Maybe beta.  It's "good enough" for my daily
use, but it's pretty rough in the edges. Caveat emptor.
