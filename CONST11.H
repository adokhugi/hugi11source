//+konstanten fÅr den viewer - hugi #11 (hellfire-pic)
//-mauscursorfarben - hinweis: mÅssen in diesen farben gezeichnet sein!
#define VIEWER_MOUSE_COLOR_MAIN    113
#define VIEWER_MOUSE_COLOR_BORDER  114
//-fortschrittsbalken
#define VIEWER_STATUSBAR_X1        463
#define VIEWER_STATUSBAR_Y1        465
#define VIEWER_STATUSBAR_Y2        469
#define VIEWER_STATUSBAR_LENGTH    144
#define VIEWER_STATUSBAR_COLOR     112
#define VIEWER_STATUSBAR_NOCOLOR   0
//-zurÅckscroll-button
#define VIEWER_BUTTON_BACK_X1      124
#define VIEWER_BUTTON_BACK_Y1      427
#define VIEWER_BUTTON_BACK_X2      174
#define VIEWER_BUTTON_BACK_Y2      479
//-vorscroll-button
#define VIEWER_BUTTON_FORWARD_X1   194
#define VIEWER_BUTTON_FORWARD_Y1   427
#define VIEWER_BUTTON_FORWARD_X2   244
#define VIEWER_BUTTON_FORWARD_Y2   479
//-drucken-button
#define VIEWER_BUTTON_PRINT_X1     12
#define VIEWER_BUTTON_PRINT_Y1     427
#define VIEWER_BUTTON_PRINT_X2     80
#define VIEWER_BUTTON_PRINT_Y2     479
//-max. anzahl der links pro seite und maximale lÑnge eines article-name + 1
#define VIEWER_LINKS_NUMBER        40
#define VIEWER_LINKS_NAME_LENGTH   256
//-y-position des textfensters
#define VIEWER_TEXTWINDOW_Y1       101
#define VIEWER_TEXTWINDOW_Y2       VIEWER_TEXTWINDOW_Y1+320
#define VIEWER_TEXTWINDOW_LINES    ((VIEWER_TEXTWINDOW_Y2-VIEWER_TEXTWINDOW_Y1)/16)
//-transparenz-farbe fÅr textfenster
#define VIEWER_TRANSPARENCY        0
//-unterstreich-farbe von links ("relativ", das hei·t eine der text-farben)
#define VIEWER_UNDERLINE_COLOR     2

//-indexdaten
#include "hugidx11.h"

//+externe files
extern "C"
{
  extern char externfont[];           // die font
  extern unsigned char idxfile[];     // das index file
  extern unsigned char pcxpic1[];     // rle komprimiertes bild :>
  extern unsigned char pcxcursor[];   // rle komprimierter cursor :>
  extern unsigned char pcxpic_t[];    // rle komprimiertes bild :>
  extern unsigned char pcxpic_c[];    // rle komprimiertes bild :>
  extern unsigned char SONG_XM[];
  extern char txtdata[];
  #define SONG_XM_LEN 413812
  #define TEXT_LEN    1042728
};

//-obsolet, nur fÅr abwÑrtskompatibilitÑt
//-scroll-geschwindigkeit
#define VIEWER_SCROLL_SPEED        8
#define VIEWER_SCROLL_LINE_LAST    319-VIEWER_SCROLL_SPEED
//-bildwiederholfrequenz (hz)
#define SCREEN_FREQUENCE           60000
