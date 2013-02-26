#ifndef SETTINGS_N7KJ765I
#define SETTINGS_N7KJ765I

#define RPBAR_WIN_NAME              "rpbar"
// used to communicate with rpbarsend
#define RPBAR_SOCKET_PATH           "/tmp/rpbarsocket"
// hopefully none of your programs have this as their title
#define RPBAR_SEP                   "$)@="

// to be used for whether rpbar is on top or bottom
#define RPBAR_TOP                   1

// padding around text (top and bottom)
#define RPBAR_PADDING               4
// for communication with ratpoison. Bigger shouldn't hurt.
#define RPBAR_BUFSIZE               2048
#define RPBAR_TIMEOUT_S             5
// affects padding in left and right
#define RPBAR_BUTTON_MARGIN         10

// Look at wmii or vim colorschemes for inspiration
#define RPBAR_BORDERCOLOR           "#666666"
#define RPBAR_BGCOLOR               "#080808"
#define RPBAR_FGCOLOR               "#D0D0D0"
#define RPBAR_MAINBGCOLOR           "#202020"
#define RPBAR_MAINFGCOLOR           "#76ff00"

#define RPBAR_FONT_STR              "-*-terminus-medium-r-normal-*-12-*-*-*-*-*-*-*"

#endif /* end of include guard: SETTINGS_N7KJ765I */
