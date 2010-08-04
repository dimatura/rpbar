#ifndef SETTINGS_N7KJ765I
#define SETTINGS_N7KJ765I

#define RPBAR_WIN_NAME          "rpbar"
#define RPBAR_FIFOPATH          "/tmp/rpbarfifo"

#define RPBAR_BARHEIGHT         13
#define RPBAR_BUFSIZE           2048
#define RPBAR_TIMEOUT_S         2
#define RPBAR_BUTTON_MARGIN     10
#define RPBAR_LABEL_FONT        0
#define RPBAR_LABEL_SIZE        10 

#define RPBAR_BGCOLOR           fl_rgb_color(0x08,0x08,0x08)
#define RPBAR_FGCOLOR           fl_rgb_color(0xD0,0xD0,0xD0)
#define RPBAR_MAINBGCOLOR       fl_rgb_color(0x20,0x20,0x20) 
#define RPBAR_MAINFGCOLOR       fl_rgb_color(0x76,0xff,0x00)

//%n = number
//%s = status
//%% = %
//%t = title
//%f = frame number
//%a = application name
//%c = resource class
#define RPBAR_CMD               "ratpoison -c \"windows \%n \%t\%s\""

#endif /* end of include guard: SETTINGS_N7KJ765I */
