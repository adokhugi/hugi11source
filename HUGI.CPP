//adok's experimente ;)
//a.k.a. hugi
//textviewerfenster kann jetzt frei positioniert werden (siehe const*.h)
//titel- und schlu·bild
//crossfading beim blÑttern
//kleinere fixes und erweiterungen
//textfile wird wÑhrend des anzeigens des titelbilds geladen

//-------------------------------------------------------

//+ausgabenspezifische daten

//-indexdaten, konstanten fÅr den viewer und externals - von hugi #11
#include "const11.h"

//-------------------------------------------------------

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <dos.h>
#include <sys\stat.h>

#include "xmplay.h"
#include "binfmem.h"
#include "ims.h"

#include "vesavbe.h"

#include "advret.h"
#include "adpcxmem.h"

#include "admouse.h"
#include "adfont.h"
#include "adprint.h"

void setpal(char *pal);
#pragma aux setpal=\
"mov dx,3c8h" \
"mov al,0" \
"out dx,al" \
"inc dx" \
"mov cx,768" \
"start: " \
"mov al,[esi]" \
"out dx,al" \
"inc esi" \
"dec ecx" \
"jnz start" \
parm[esi] modify [eax ecx edx];

//+struct-definition fÅr den viewer
struct ADOK_VIEWER_LINK
{
  char name[256];
  int  x1;
  int  y1;
  int  x2;
  int  y2;
};

//+globale variablen
signed char sound_off=0;          //sound on/off
//-palette
char        *pal,                 //aktuelle farbpalette
            *pal_bkpic,           //palette fÅr hintergrundbild
            *pal_text,            //palette fÅr text
            *pal_saved,           //gesicherte farbpalette
            *pal_black;           //komplett schwarze farbpalette
//-font-daten
char        *font;                //geladene font-datei
//-maus-struct
struct ADOK_MOUSE  mouse;                //maus-struct
//-virtual screens
struct ADOK_SCREEN vs_page1,             //aktuelles / auf bildschirm
                                         //anzuzeigendes textfenster
                   vs_page2,             //nÑchste textseite
                   vs_bkground;          //hintergrundbild (text UND buttons)

//+diverse funktionen

void VBE_SetPixel(int x,int y,char col)
{
  long temp=640*y+x;

  if(lfb_detected)
  {
    *(realscreen.mem+temp)=col;
  }
  else
  {
    VBE_SetBank((short)(temp>>16));
    *((char*)0xa0000+(temp&0xffff))=col;
  }
}

  void font_char_print_screen_crossfade
                              (struct ADOK_SCREEN *vscreen,char *font_ram,
                              char font_height,
                              int x,int y,char ascii,char color,
                              struct ADOK_SCREEN *vs_bk)
  /*
    aufgabe:       anzeigen eines zeichens eines geladenen fonts in einem
                   virtual screen auf angegebener position. gleichzeitig wird
                   ÅberprÅft, welche farbe der pixel an dieser stelle in vs_bk
                   hat, und die farbe wird so gesetzt, da· crossfade ermîglicht
                   wird
    hinweis:       vs_bk darf, wenn lfb deaktiviert, nicht realscreen sein!
    vorgangsweise: *) berechnen des offsets innerhalb von *font_ram, an dem
                      das gewÅnschte zeichen gespeichert ist:
                      offset=ascii*font_height
                   schleife von 0 bis 15 durchlaufen
                     *) char_line auf *(font_ram+offset) setzen
                     variable bit_number in schleife von 0 bis 7 durchlaufen
                       *) ist bit bit_number gesetzt?
                          temp=1<<bit_number;
                          if(temp==(char_line&temp)) ...
                          wenn ja: pixel setzen
                              *((*vscreen).mem+(*vscreen).x*y+x+7-bit_number)=
                              (color+1)*16+(*((*vs_bk).mem+(*vs_bk).x*(y+VIEWER_TEXTWINDOW_Y1)+x+7-bit_number))
                     *) y um 1 erhîhen
                     *) offset um 1 erhîhen

    parameter:     *vscreen:    pointer auf den virtual screen, auf den
                                geschrieben werden soll
                   *font_ram:   pointer auf die font-daten
                   font_height: zeichenhîhe
                   x:           x-koordinate, wo ausgegeben werden soll
                   y:           y-koordinate, wo ausgegeben werden soll
                   ascii:       zeichen, das ausgegeben werden soll
                   color:       farbe, in der das zeichen ausgegeben werden
                                soll
                   *vs_bk:      pointer auf den virtual screen, von dem die
                                hintergrund-daten geholt werden
  */
  {
    int  offset=ascii*font_height;
    char char_line,
         y_counter,
         bit_number,
         temp;

    for(y_counter=0;y_counter<font_height;y_counter++)
    {
      char_line=*(font_ram+offset);
      for(bit_number=0;bit_number<=7;bit_number++)
      {
        temp=1<<bit_number;
        if(temp==(char_line&temp))
        {
          *((*vscreen).mem+(*vscreen).x*y+x+7-bit_number)=
          (color+1)*16+(*((*vs_bk).mem+(*vs_bk).x*(y+VIEWER_TEXTWINDOW_Y1)+x+7-bit_number));
        }
/*
        else
        {
          *((*vscreen).mem+(*vscreen).x*y+x+7-bit_number)=
          (*((*vs_bk).mem+(*vs_bk).x*(y+VIEWER_TEXTWINDOW_Y1)+x+7-bit_number));
        }
*/
      }
      y++;
      offset++;
    }
  }


//+viewer-funktionen

void viewer_statusbar_show(struct ADOK_SCREEN *vscreen,int statusbar_status)
/*
  aufgabe:       zeigt den aktuellen stand des fortschrittsbalkens an
  vorgangsweise: *) berechnen der differenz von VIEWER_STATUSBAR_LENGTH und
                    statusbar_status => pixel, die in farbe
                    VIEWER_STATUSBAR_COLOR gesetzt werden mÅssen
                    => statusbar_clear_number
                 *) VIEWER_STATUSBAR_X1 plus statusbar_status => position, ab
                    der in farbe VIEWER_STATUSBAR_NOCOLOR gesetzt werden mu·
                    => statusbar_clear_pos_x
                 durchlauf einer schleife von VIEWER_STATUSBAR_Y1 bis
                 VIEWER_STATUSBAR_Y1 (schleifenzÑhler y_counter)
                   *) an dieser stelle auf *vscreen statusbar_status anzahl
                      von pixeln in farbe VIEWER_STATUSBAR_COLOR setzen
                      memset((*vscreen).mem+y_counter*(*vscreen).x+
                             VIEWER_STATUSBAR_X1,VIEWER_STATUSBAR_COLOR,
                             statusbar_status)
                   *) nun die VIEWER_STATUSBAR_NOCOLOR-pixel setzen
                      memset((*vscreen).mem+y_counter*(*vscreen).x+
                             statusbar_clear_pos_x,VIEWER_STATUSBAR_NOCOLOR,
                             statusbar_clear_number)
  parameter:     *vscreen:         pointer auf virtual screen
                 statusbar_status: aktueller status des fortschrittsbalkens
                                   (in pixel)
  hinweis:       wird von viewer_crossfade, viewer_fade, viewer_scroll_back und
                 viewer_scroll_forward benîtigt
*/
{
  int statusbar_clear_number=VIEWER_STATUSBAR_LENGTH-statusbar_status,
      statusbar_clear_pos_x =VIEWER_STATUSBAR_X1+statusbar_status,
      x_counter,
      y_counter;

  for(y_counter=VIEWER_STATUSBAR_Y1;y_counter<=VIEWER_STATUSBAR_Y2;
      y_counter++)
  {
    if(lfb_detected)
    {
      memset((*vscreen).mem+y_counter*(*vscreen).x+VIEWER_STATUSBAR_X1,
             VIEWER_STATUSBAR_COLOR,statusbar_status);
      memset((*vscreen).mem+y_counter*(*vscreen).x+statusbar_clear_pos_x,
             VIEWER_STATUSBAR_NOCOLOR,statusbar_clear_number);
    }
    else
    {
      for(x_counter=VIEWER_STATUSBAR_X1;
          x_counter<VIEWER_STATUSBAR_X1+statusbar_status;
          x_counter++)
      VBE_SetPixel(x_counter,y_counter,VIEWER_STATUSBAR_COLOR);

      for(x_counter=statusbar_clear_pos_x;
          x_counter<statusbar_clear_pos_x+statusbar_clear_number;
          x_counter++)
      VBE_SetPixel(x_counter,y_counter,VIEWER_STATUSBAR_NOCOLOR);
    }
  }
}

int viewer_statusbar_status_get(long article_pages_number,
                                long article_page_act)
/*
  aufgabe:       ermitteln des aktuellen status des fortschrittsbalkens
  vorgangsweise: *) wenn letzte seite (article_pages_number==article_page_act)
                    rÅckgabewert gleich VIEWER_STATUSBAR_LENGTH
                    ansonsten: rÅckgabewert=(VIEWER_STATUSBAR_LENGTH*
                               article_page_act)/article_pages_number
  parameter:     article_pages_number: anzahl der textseiten, die der artikel
                                       umfa·t
                 article_page_act:     aktuelle textseite
  rÅckgabe:      ist doch klar... aktueller status des fortschrittsbalkens!
*/
{
  if(article_pages_number==article_page_act)
    return(VIEWER_STATUSBAR_LENGTH);
  else
    return((VIEWER_STATUSBAR_LENGTH*article_page_act)/article_pages_number);
}

void viewer_crossfade(int statusbar_status_act)
/*
  aufgabe:       fadet zuerst zu background, dann zu nÑchster textseite
  wichtig:       *) die aktuelle page (die, die gerade angezeigt wird) mu·
                    sich im virtual screen vs_page1 befinden
                 *) die neue page (die, auf die gescrollt wird) mu· sich im
                    virtual screen vs_page2 befinden
  parameter:     statusbar_status_act:  der stand des fortschrittsbalkens, der
                                        nach dem anzeigen der aktuellen seite
                                        des artikels berechnet wurde
*/
{
  int i;

  pal8_fadeto(pal_bkpic,4,sound_off);
  vscreen_copy_full(&vs_page1,&vs_page2);
  vscreen_copy_part_dest(&realscreen,&vs_page1,0,VIEWER_TEXTWINDOW_Y1);
  viewer_statusbar_show(&realscreen,statusbar_status_act);
  pal8_fadeto(pal_text,4,sound_off);
}

signed char viewer_article_find(char *article_name,
                               long *article_offset,long *article_length)
/*
  aufgabe:       finden des artikel-offsets und der artikel-lÑnge im
                 index-file *idxfile; schreiben Åber pointer
  vorgangsweise: *) in einer do-while-schleife artikelname mit aktuellem im
                    indexarray vergleichen
  parameter:     *article_name:   string mit dem namen des artikels
                 *article_offset: pointer auf die long-variable, in die der
                                  offset des artikels geschrieben werden soll
                 *article_length: pointer auf die long-variable, in die die
                                  lÑnge des artikels geschrieben werden soll
  rÅckgabe:      0, falls artikel nicht gefunden werden konnte
*/
{
  signed char foundflag   =0;
  long        arraycounter=0;

  do
  {
    if( !strcmp(article_name,index_names[arraycounter]) )
    {
      foundflag=-1;
      *article_offset=index_offsets[arraycounter];
      *article_length=index_lengths[arraycounter];
    }
    arraycounter++;
  } while((arraycounter<index_number)&&(!foundflag));

  return(foundflag);
}

long viewer_article_lines_nr_get(char *txtdata,
                                 long article_offset,long article_length)
/*
  aufgabe:       ermitteln der anzahl der textzeilen des angegebenen artikels
  vorgangsweise: *) setze anzahl der gefundenen ascii-13-zeichen auf 1 (weil
                    die letzte zeile des artikels, an deren ende ja nicht
                    unbedingt ein ascii-13-zeichen stehen mu·, mitgerechnet
                    werden mu·)
                 schleife mit variable counter von 0 bis article_length-1
                   *) vergleiche *(txtdata+article_offset+counter) mit 13
                      wenn gleich: erhîhe die anzahl der gefundenen
                      ascii-13-zeichen um 1
                 *) anzahl der gefundenen ascii-13-zeichen zurÅckgeben
*/
{
  long ascii13_number=1,
       counter;
  char *pointer      =txtdata+article_offset;

  for(counter=0;counter<article_length;counter++)
  {
    if(*pointer==13)
      ascii13_number++;
    pointer++;
  }

  return(ascii13_number);
}

long viewer_article_page_read(struct ADOK_SCREEN *vscreen,
                              char *txtdata,
                              long article_offset,
                              long article_length,
                              long article_page_act_offset,
                              struct ADOK_VIEWER_LINK *links_index,
                              int *links_nr)
/*
  aufgabe:       jede menge:
                 *) einlesen einer artikelseite und schreiben auf den virtual
                    screen
                 *) zurÅckgeben des offsets der nÑchsten textseite
                 *) anpassen der nummer der links und von links_index
  vorgangsweise: zuerst einmal die "sub-funktionen" (makros):

                 link-ende
                   *) ist link_flag ungleich 0?
                      wenn ja:
                        *) x_counter-1 in
                           links_index[*links_nr].x2 kopieren
                        *) y_counter+15 in
                           links_index[*links_nr].y2 kopieren
                        *) link_flag auf 0 setzen
                        *) *links_nr um 1 erhîhen

                 zeilenumbruch
                   link-ende
                   *) textline_act um 1 erhîhen
                   *) y_counter um 16 erhîhen
                   *) x_counter auf 0 setzen
                   *) offset_counter um 2 erhîhen (ascii 10 Åberspringen)
                   *) color_act auf 255 (standardfarbe) setzen

                 zeichen ausgeben
                   *) ist x_counter+7 gleich/grî·er (*vscreen).x?
                      wenn ja: zeilenumbruch, aber offset_counter nicht
                               erhîhen
                   *) in anbetracht des vorigen punkts folgendes nur dann
                      ausfÅhren, wenn textline_act<20:
                      *) wenn link_flag, dann in VIEWER_UNDERLINE_COLOR
                         unterstreichen
                      *) font_char_print_screen(vscreen,font,
                         16,x_counter,y_counter,
                         *offset_counter,color_act)
                      *) x_counter um 8 erhîhen
                      *) offset_counter um 1 erhîhen

                 nun die "hauptfunktion":

                 *) texthintergrund auf vscreen kopieren
                 *) berechnen des ende des artikels im arbeitsspeicher
                    article_end=(long)txtdata+article_offset+article_length-2
                 *) offset_counter=
                    (long)txtdata+article_offset+article_page_act_offset-1
                 *) setzen der aktuellen x-position auf 0: x_counter=0
                 *) setzen der aktuellen y-position auf 0: y_counter=0
                 *) setzen der aktuellen textzeile auf 0:  textline_act=0
                 *) color_act auf 255 (standardfarbe) setzen
                 *) stringvariable string_temp mit 256 bytes reservieren
                 *) *links_nr auf 0 setzen
                 *) link_flag auf 0 setzen (flag, da· link abgearbeitet wird)
                 schleife, solange offset_counter<=article_end und
                 textline_act<20
                   *) ist das byte auf offset_counter gleich 13?
                      wenn ja: zeilenumbruch
                      wenn nein: ist das byte auf offset_counter gleich 248?
                        wenn ja: steuerzeichen auswerten
                          *) offset_counter um 1 erhîhen
                          *) ist das byte auf offset_counter grî·er/gleich 48,
                             aber kleiner/gleich 53? (ascii-zeichen "0"-"5")
                             wenn ja: farbe Ñndern
                               *) color_act auf <das byte auf offset_counter>
                                  plus 202 setzen
                               *) offset_counter um 1 erhîhen
                             wenn nein: weiter auswerten
                               *) das byte auf offset_counter und die nÑchsten
                                  beiden bytes auf string_temp kopieren
                               *) offset_counter um 3 erhîhen
                               *) string_temp auswerten
                                  wenn COL:
                                    *) das byte auf offset_counter und die
                                       nÑchsten beiden bytes auf string_temp
                                       kopieren
                                    *) offset_counter um 3 erhîhen
                                    *) string_temp mit atoi konvertieren und
                                       das ergebnis in color_act schreiben
                                  wenn ASC:
                                    *) das byte auf offset_counter und die
                                       nÑchsten beiden bytes auf string_temp
                                       kopieren
                                    *) offset_counter um 2 erhîhen
                                       (die dritte erhîhung wird vom
                                       zeichenausgabemakro Åbernommen)
                                    *) string_temp mit atoi konvertieren und
                                       dieses zeichen ausgeben
                                  wenn LNK:
                                    *) bis exklusiv zum nÑchsten 248 in
                                       string_temp einlesen und offset_counter
                                       dementsprechend erhîhen
                                    *) string_temp in
                                       links_index[*links_nr].name kopieren
                                    *) x_counter in links_index[*links_nr].x1
                                       kopieren
                                    *) y_counter in links_index[*links_nr].y1
                                       kopieren
                                    *) link_flag auf -1 setzen
                                  wenn LKE:
                                    link-ende
                                  andernfalls:
                                    *) offset_counter um 3 erniedrigen
                                       (steuercode ignorieren)
                        wenn nein: zeichen ausgeben
                 *) string_temp freigeben
                 *) rÅckgabe:   1+
                                offset_counter-((long)txtdata+article_offset)
*/
{
  long        article_end   =(long)txtdata+article_offset+article_length-2,
              offset_counter=(long)txtdata+article_offset+
                             article_page_act_offset-1;
  int         x_counter     =0,
              y_counter     =0,
              textline_act  =0,
              color_act     =5,
              tempbyte,
              tempcounter;
  char        *string_temp;
  signed char link_flag     =0;

  /*
    hier erledigt:
    - textbackground auf vscreen
    - farb-angaben
    - char-print-routinen-aufruf auf neue routine Ñndern
    - unterstreichen .. in textfarbe VIEWER_UNDERLINE_COLOR
  */

  #define VIEWER_ARTICLE_PAGE_READ_LINK_END                           \
    if(link_flag)                                                     \
    {                                                                 \
      links_index[*links_nr].x2=x_counter-1;                          \
      links_index[*links_nr].y2=VIEWER_TEXTWINDOW_Y1+y_counter+15;    \
      link_flag=0;                                                    \
      *links_nr+=1;                                                   \
    }

  #define VIEWER_ARTICLE_PAGE_READ_LINEFEED                           \
    VIEWER_ARTICLE_PAGE_READ_LINK_END;                                \
    textline_act++;                                                   \
    y_counter+=16;                                                    \
    x_counter=0;                                                      \
    offset_counter+=2;                                                \
    color_act=5

  #define VIEWER_ARTICLE_PAGE_READ_CHAR_PRINT                         \
    if(x_counter+7>=(*vscreen).x)                                     \
    {                                                                 \
      VIEWER_ARTICLE_PAGE_READ_LINEFEED;                              \
      offset_counter-=2;                                              \
    }                                                                 \
    if(textline_act<VIEWER_TEXTWINDOW_LINES)                          \
    {                                                                 \
      if(link_flag)                                                   \
      {                                                               \
        char *p=(*vscreen).mem+(*vscreen).x*(y_counter+13)+x_counter; \
        for(tempcounter=0;tempcounter<8;tempcounter++)                \
          *(p+tempcounter)=(VIEWER_UNDERLINE_COLOR+1)*16+*((vs_bkground.mem+vs_bkground.x*(VIEWER_TEXTWINDOW_Y1+y_counter+13)+x_counter+tempcounter)); \
      }                                                               \
      font_char_print_screen_crossfade(vscreen,font,16,               \
        x_counter,y_counter,tempbyte,color_act,&vs_bkground);         \
      x_counter+=8;                                                   \
      offset_counter++;                                               \
    }

  //background hineinkopieren
  vscreen_copy_part_source(vscreen,&vs_bkground,0,VIEWER_TEXTWINDOW_Y1);

  mymalloc((void **)&string_temp,256,0);
  *links_nr=0;

  while((offset_counter<=article_end)&&(textline_act<VIEWER_TEXTWINDOW_LINES))
  {
    tempbyte=*((char *)offset_counter);
    if(tempbyte==13)
    {
      VIEWER_ARTICLE_PAGE_READ_LINEFEED;
    }
    else
    {
      if(tempbyte==248)
      {
        //steuerzeichen auswerten
        offset_counter++;
        tempbyte=*((char *)offset_counter);
        if((tempbyte>=48)&&(tempbyte<=53))
        {
          color_act=tempbyte-48;
          offset_counter++;
        }
        else
        {
          for(tempcounter=0;tempcounter<3;tempcounter++)
          {
            string_temp[tempcounter]=*((char *)offset_counter);
            offset_counter++;
          }
          string_temp[3]='\0';
          if(!strcmp(string_temp,"COL"))
          {
            for(tempcounter=0;tempcounter<3;tempcounter++)
            {
              string_temp[tempcounter]=*((char *)offset_counter);
              offset_counter++;
            }
            string_temp[3]='\0';
            color_act=atoi(string_temp);
          }
          else
            if(!strcmp(string_temp,"ASC"))
            {
              for(tempcounter=0;tempcounter<3;tempcounter++)
              {
                string_temp[tempcounter]=*((char *)offset_counter);
                offset_counter++;
              }
              string_temp[3]='\0';
              tempbyte=atoi(string_temp);
              VIEWER_ARTICLE_PAGE_READ_CHAR_PRINT;
              offset_counter--;
            }
            else
              if(!strcmp(string_temp,"LNK"))
              {
                tempcounter=0;
                do
                {
                  string_temp[tempcounter]=*((char *)offset_counter);
                  offset_counter++;
                  tempcounter++;
                } while(string_temp[tempcounter-1]!=248);
                string_temp[tempcounter-1]='\0';
                strcpy(links_index[*links_nr].name,string_temp);
                links_index[*links_nr].x1=x_counter;
                links_index[*links_nr].y1=VIEWER_TEXTWINDOW_Y1+y_counter;
                link_flag=-1;
              }
              else
                if(!strcmp(string_temp,"LKE"))
                {
                   VIEWER_ARTICLE_PAGE_READ_LINK_END;
                }
                else
                  offset_counter-=3;
        }
      }
      else
      {
        VIEWER_ARTICLE_PAGE_READ_CHAR_PRINT;
      }
    }
  }

  free(string_temp);
  return(1+offset_counter-((long)txtdata+article_offset));
}

void viewer_article_print(char *article_offset_absolute,
                          long article_length)
/*
  aufgabe:       druckt einen artikel aus
  vorgangsweise: *) initialisieren von string_temp
                    (statische grî·e von 4 bytes)
                 *) setze string_temp[3] auf '\n'
                 *) setze counter auf 0
                 while-schleife solange counter kleiner article_length
                   *) setze tempbyte auf *(article_offset_absolute+counter)
                   *) ist tempbyte gleich 248?
                      wenn ja:
                        *) counter um 1 erhîhen
                        *) setze tempbyte auf
                           *(article_offset_absolute+counter)
                        *) tempbyte>=48 und tempbyte<=53?
                           wenn ja:
                             *) counter um 1 erhîhen
                           wenn nein:
                             *) aktuelles byte und die nÑchsten beiden in
                                string_temp schreiben
                             *) string_temp auswerten
                                wenn LKE:
                                  *) counter um 3 erhîhen
                                wenn COL:
                                  *) counter um 6 erhîhen
                                wenn LNK:
                                  *) counter solange erhîhen, bis
                                     *(article_offset_absolute+counter)
                                     gleich 248
                                  *) counter um 1 erhîhen
                                wenn ASC:
                                  *) angegebenes zeichen ausgeben
                                  *) counter um 6 erhîhen
                      wenn nein:
                        *) zeichen ausgeben:
                           printer_char_print(tempbyte)
                        *) counter um 1 erhîhen
                 *) zeilenumbruch ausgeben:
                    printer_char_print(13)
                    printer_char_print(10)
                 *) seitenvorschub ausgeben:
                    printer_char_print(12)
*/
{
  char string_temp[4],
       tempbyte;
  long counter       =0;

  while(counter<article_length)
  {
    tempbyte=*(article_offset_absolute+counter);
    if(tempbyte==248)
    {
      counter++;
      tempbyte=*(article_offset_absolute+counter);
      if((tempbyte>=48)&&(tempbyte<=53))
        counter++;
      else
      {
        string_temp[0]=tempbyte;
        string_temp[1]=*(article_offset_absolute+counter+1);
        string_temp[2]=*(article_offset_absolute+counter+2);
        if(!strcmp(string_temp,"LKE"))
          counter+=3;
        else
        {
          if(!strcmp(string_temp,"COL"))
            counter+=6;
          else
          {
            if(!strcmp(string_temp,"LNK"))
            {
              do
              {
                counter++;
              }
              while(248!=*(article_offset_absolute+counter));
              counter++;
            }
            else
            {
              if(!strcmp(string_temp,"ASC"))
              {
                string_temp[0]=*(article_offset_absolute+counter+3);
                string_temp[1]=*(article_offset_absolute+counter+4);
                string_temp[2]=*(article_offset_absolute+counter+5);
                printer_char_print(atoi(string_temp));
                counter+=6;
              }
            }
          }
        }
      }
    }
    else
    {
      printer_char_print(tempbyte);
      counter++;
    }
  }
  printer_char_print(13);
  printer_char_print(10);
  printer_char_print(12);
}

int viewer(char *txtdata,char *article_name,long instanz_nr,
           int statusbar_status_prev)
/*
  aufgabe:       anzeigen eines "hugi-artikels" aus dem speicherbereich
                 *txtdata, wobei der offset dieses artikels in der datei
                 *idxfile nachgeschlagen wird. weiters werden nummer der
                 instanz (0=indexfile, wird bei jedem aufruf einer neuen
                 viewer-instanz um 1 erhîht) sowie der aktuelle stand des
                 fortschrittsbalkens benîtigt, den der vorige artikel
                 "hinterlassen" hat
  vorgangsweise: sub-funktionen (makros):

                 kleine mouse-"initialisierung"
                 *) mouse_status_get(&mouse);
                 *) mouse_cursor_bkground_save(&mouse,&realscreen);
                 *) mouse_cursor_show(&mouse,&realscreen);

                 allgemeiner ausgabeteil
                 *) stand des fortschrittsbalkens ermitteln
                    -> statusbar_status_act
                 *) textviewer-hintergrund auf vs_page2 kopieren
                 *) aktuelle textseite auf vs_page2 schreiben
                    rÅckgabe ist pointer auf nÑchste textseite
                    -> in tabelle eintragen

                 hauptfunktion:

                 *) links_index initialisieren
                 *) finden des offsets und der byte-lÑnge des artikels im
                    index-file
                 *) anzahl der textseiten ermitteln
                    => tabelle mit den textseiten-offsets anlegen
                       (grî·e der tabelle je nach ermittelter anzahl)

                 ausgeben der ersten textseite
                 *) allgemeiner ausgabeteil
                 *) ist instanz_nr gleich 0? (index-file)
                    wenn ja:
                      *) vs_page2 auf vs_page1 kopieren
                      *) vs_page1 auf realscreen kopieren
                      *) fortschrittsbalken auf realscreen anzeigen
                      *) einfaden auf pal_text
                    wenn nein:
                      *) viewer_crossfade
                 *) stand des fortschrittsbalkens anpassen
                    statusbar_status_prev=statusbar_status_act

                 *) kleine mouse-"initalisierung"

                 hauptschleife
                   *) mausstatus abfragen
                   *) wenn maus bewegt, mauscursor anzeigen
                   *) wenn linke maustaste gedrÅckt
                      *) mouse.vs_bkground auf AKTUELLER mausposition ausgeben
                      *) loslassen der linken maustaste mittels mouse_wait
                         abwarten
                      *) mausposition auswerten
                        *) wenn innerhalb von VIEWER_BUTTON_BACK
                           *) wenn article_page_act ungleich 1
                              *) article_page_act um 1 erniedrigen
                              *) allgemeiner ausgabeteil
                              *) zurÅckscrollen mittels viewer_crossfade
                              *) stand des fortschrittsbalkens anpassen
                                 statusbar_status_prev=statusbar_status_act
                        *) andernfalls wenn innerhalb von
                           VIEWER_BUTTON_FORWARD
                           *) wenn article_page_act ungleich article_pages_nr
                              *) article_page_act um 1 erhîhen
                              *) allgemeiner ausgabeteil
                              *) weiterscrollen mittels viewer_crossfade
                              *) stand des fortschrittsbalkens anpassen
                                 statusbar_status_prev=statusbar_status_act
                        *) andernfalls wenn innerhalb von VIEWER_BUTTON_PRINT
                           *) artikel auf drucker ausgeben
                        *) andernfalls ÅberprÅfen, ob innerhalb von links
                           *) setze link_found_flag auf 0
                           *) setze counter auf 0
                           *) while-schleife, solange
                              counter kleiner links_nr und
                              link_found_flag gleich 0
                              *) wenn mauscursor innerhalb des links mit der
                                 nummer des schleifenzÑhlers, setze
                                 link_found_flag auf -1
                              *) counter um 1 erhîhen
                           *) wenn link_found_flag gleich -1
                              *) aktuelle mausposition - ohne mouse_status_get
                                 nochmals aufzurufen - in viewer_mouse_saved_x
                                 und viewer_mouse_saved_y abspeichern
                              *) viewer aufrufen:
                                 statusbar_status_prev=viewer(txtdata,idxfile,
                                 links_index[counter-1].name,
                                 instanz_nr+1,statusbar_status_act)
                              *) allgemeiner ausgabeteil
                              *) rÅckwÑrtsscrollen
                              *) stand des fortschrittsbalkens anpassen
                                 statusbar_status_prev=statusbar_status_act
                              *) abgespeicherte mausposition setzen:
                                 mouse_pos_set(viewer_mouse_saved_x,
                                              viewer_mouse_saved_y)
                      *) kleine mouse-"initialisierung" wiederholen

                 *) mouse.vs_bkground auf AKTUELLER mausposition ausgeben
                 *) loslassen der rechten maustaste mittels mouse_wait
                    abwarten
                 *) tabelle mit den textseiten-offsets freigeben
                 *) links_index freigeben
  parameter:     *txtdata:              pointer auf den speicherbereich, in
                                        den die artikeldaten-datei geladen
                                        wurde
                 *idxfile:              FILE-pointer auf die index-datei, in
                                        der sich namen und offsets der artikel
                                        befinden
                 *article_name:         ein string, der den namen des artikels
                                        enthÑlt
                 instanz_nr:            nummer der viewer-instanz
                                        (0=indexfile, von main aufgerufen;
                                        andernfalls, wie viele Åbergeordnete
                                        viewer-instanzen noch offen sind)
                 statusbar_status_prev: "geerbter" stand des fortschrittbalkens
  rÅckgabe:      aktueller stand des fortschrittbalkens
*/
{
  long                    article_offset,
                          article_length,
                          article_pages_nr,
                          *article_pages_offsets,
                          article_page_act,
                          counter;
  char                    *article_tempoffset_counter;
  struct ADOK_VIEWER_LINK *links_index;
  signed char             link_found_flag;
  int                     links_nr,
                          viewer_mouse_saved_x,
                          viewer_mouse_saved_y,
                          statusbar_status_act;

  #define VIEWER_MOUSE_INIT                                          \
    mouse_status_get(&mouse);                                        \
    mouse_cursor_bkground_save(&mouse,&realscreen);                  \
    mouse_cursor_show(&mouse,&realscreen)

  #define VIEWER_DISPLAY_COMMON                                      \
    /*-stand des fortschrittsbalkens ermitteln*/                     \
    statusbar_status_act=viewer_statusbar_status_get                 \
                           (article_pages_nr,article_page_act);      \
    /*-textviewer-hintergrund auf vs_page2 kopieren*/                \
    vscreen_copy_part_source(&vs_page2,&vs_bkground,0,VIEWER_TEXTWINDOW_Y1); \
    /*-erste textseite auf vs_page2 schreiben*/                      \
    /*hinweis: article_page_act hat in der index-tabelle den*/       \
    /*eintrag article_page_act-1. deshalb betreffen zugriffe auf*/   \
    /*article_pages_offsets[article_page_act] die nÑchste*/          \
    /*textseite.*/                                                   \
    article_pages_offsets[article_page_act]=                         \
      viewer_article_page_read(&vs_page2,                            \
                       txtdata,article_offset,article_length,        \
                       article_pages_offsets[article_page_act-1],    \
                       links_index,&links_nr);

  //-links_index initialisieren
  mymalloc((void **)&links_index,
           VIEWER_LINKS_NUMBER*sizeof(struct ADOK_VIEWER_LINK),1);

  //-artikel suchen, wenn nicht gefunden, kommentarlos zur vorigen instanz
  if(!viewer_article_find(article_name,&article_offset,&article_length))
    return(statusbar_status_prev);

  //-anzahl der textseiten ermitteln
  //anzahl der textzeilen ermitteln
  article_pages_nr=
    viewer_article_lines_nr_get(txtdata,article_offset,article_length);
  //in textseiten umrechnen
  if(0==article_pages_nr%VIEWER_TEXTWINDOW_LINES)
    article_pages_nr/=VIEWER_TEXTWINDOW_LINES;
  else
    article_pages_nr=1+article_pages_nr/VIEWER_TEXTWINDOW_LINES;
  //tabelle mit den textseiten-offsets anlegen und mit nullen fÅllen
  /*
    es wird deshalb um 1 eintrag mehr angelegt als benîtigt, weil der
    abschnitt, der sich um das vorwÑrtsblÑttern kÅmmert, IMMER den offset der
    nÑchsten textseite in die tabelle eintrÑgt, egal, ob es eine nÑchste
    textseite gibt oder nicht, weil schon die letzte textseite erreicht wurde.
  */
  mymalloc((void **)&article_pages_offsets,(article_pages_nr+1)*sizeof(long),0);
  //aktuelle textseite ist 1
  article_page_act=1;
  article_pages_offsets[0]=0;

  //+ausgeben der ersten textseite
  //-allgemeiner ausgabeteil
  VIEWER_DISPLAY_COMMON;
  /*-art des erscheinen des texts bestimmen:
     bei instanz_nr 0 (index-file) einfaden,
     bei hîherer instanz_nr vorwÑrtsscrollen
  */
  if(instanz_nr)
    viewer_crossfade(statusbar_status_act);
  else
  {
    vscreen_copy_full(&vs_page1,&vs_page2);
    vscreen_copy_part_dest(&realscreen,&vs_page1,0,VIEWER_TEXTWINDOW_Y1);
    viewer_statusbar_show(&realscreen,statusbar_status_act);
    pal8_fadeto(pal_text,1,sound_off);
  }
  //-fortschrittsbalken adjustieren
  statusbar_status_prev=statusbar_status_act;

  //-kleine mouse-"initalisierung"
  VIEWER_MOUSE_INIT;

  //+HAUPTSCHLEIFE
  do
  {
    //-auswerten des mausstatus
    mouse_status_get(&mouse);
    //-mauscursor bewegt? ja -> hintergrund saven, mauscursor anzeigen
    if(mouse.moved)
    {
      mouse_cursor_bkground_restore(&mouse,&realscreen);
      mouse_cursor_bkground_save(&mouse,&realscreen);
      mouse_cursor_show(&mouse,&realscreen);
    }
    //-linken mausbutton gedrÅckt? ja -> mausposition auswerten
    if(mouse.button_left)
    {
      vscreen_copy_part_dest(&realscreen,&(mouse.vs_bkground),
                             mouse.x,mouse.y);
      mouse_wait(&mouse,&(mouse.button_left));
      //back-scroll-button
      if((mouse.x>=VIEWER_BUTTON_BACK_X1)&&
         (mouse.x<=VIEWER_BUTTON_BACK_X2)&&
         (mouse.y>=VIEWER_BUTTON_BACK_Y1)&&
         (mouse.y<=VIEWER_BUTTON_BACK_Y2))
      {
        if(article_page_act>1)
        {
          article_page_act--;
          VIEWER_DISPLAY_COMMON;
          viewer_crossfade(statusbar_status_act);
          statusbar_status_prev=statusbar_status_act;
        }
      }
      else
      {
        //forward-scroll-button
        if((mouse.x>=VIEWER_BUTTON_FORWARD_X1)&&
           (mouse.x<=VIEWER_BUTTON_FORWARD_X2)&&
           (mouse.y>=VIEWER_BUTTON_FORWARD_Y1)&&
           (mouse.y<=VIEWER_BUTTON_FORWARD_Y2))
        {
          if(article_page_act<article_pages_nr)
          {
            article_page_act++;
            VIEWER_DISPLAY_COMMON;
            viewer_crossfade(statusbar_status_act);
            statusbar_status_prev=statusbar_status_act;
          }
        }
        else
        {
          //print-button
          if((mouse.x>=VIEWER_BUTTON_PRINT_X1)&&
             (mouse.x<=VIEWER_BUTTON_PRINT_X2)&&
             (mouse.y>=VIEWER_BUTTON_PRINT_Y1)&&
             (mouse.y<=VIEWER_BUTTON_PRINT_Y2))
          {
            viewer_article_print(txtdata+article_offset-1,article_length);
          }
          else
          {
            //links-felder durchsuchen
            link_found_flag=0;
            counter=0;
            while((counter<links_nr)&&(0==link_found_flag))
            {
              if((mouse.x>=links_index[counter].x1)&&
                 (mouse.x<=links_index[counter].x2)&&
                 (mouse.y>=links_index[counter].y1)&&
                 (mouse.y<=links_index[counter].y2))
                link_found_flag=-1;
              counter++;
            }
            if(link_found_flag)
            {
              viewer_mouse_saved_x=mouse.x;
              viewer_mouse_saved_y=mouse.y;
              statusbar_status_prev=
                viewer(txtdata,links_index[counter-1].name,
                       instanz_nr+1,statusbar_status_act);
              VIEWER_DISPLAY_COMMON;
              viewer_crossfade(statusbar_status_act);
              statusbar_status_prev=statusbar_status_act;
              mouse_pos_set(viewer_mouse_saved_x,viewer_mouse_saved_y);
            }
          }
        }
      }
      VIEWER_MOUSE_INIT;
    }
  }
  while(!mouse.button_right);

  //-mouse-"close"
  vscreen_copy_part_dest(&realscreen,&(mouse.vs_bkground),mouse.x,mouse.y);
  mouse_wait(&mouse,&(mouse.button_right));

  //-tabelle mit den textseiten-offsets freigeben
  free(article_pages_offsets);

  //-links_index freigeben
  free(links_index);

  //rÅckgabe: aktueller fortschrittsbalken-stand
  return(statusbar_status_act);
}

//-titel/schlu·bild-funktionen

void hugi_titlepicture_show()
{
  char temp;

  //titelbild laden
  pcx256_memload(pcxpic_t,&vs_bkground,pal);
  //bildschirm schwarz machen
  pal8_set(pal_black);
  //geladenes titelbild anzeigen
  vscreen_copy_full(&realscreen,&vs_bkground);
  //einfaden
  pal8_fadeto(pal,1,sound_off);
  //ein bi·chen warten
  for(temp=0;temp<150;temp++)
  {
    retrace_vertical_wait(sound_off);
  }
  //ausfaden
  pal8_fadeto(pal_black,1,sound_off);
}

void hugi_closingpicture_show()
{
  char temp;

  //schlu·bild laden
  pcx256_memload(pcxpic_c,&vs_bkground,pal);
  //bildschirm schwarz machen
  pal8_set(pal_black);
  //geladenes titelbild anzeigen
  vscreen_copy_full(&realscreen,&vs_bkground);
  //einfaden
  pal8_fadeto(pal,1,sound_off);
  //ein bi·chen warten
  for(temp=0;temp<150;temp++)
  {
    retrace_vertical_wait(sound_off);
  }
  //ausfaden
  pal8_fadeto(pal_black,1,sound_off);
}

void main(int argc,char **argv)
{
  FILE                  *savefile,          //zum absaven der textdaten-datei
                        *tmpfile;           //temporÑr (z.b. fÅr modulecheck)
  signed char           j,
                        k;
  long                  i,
                        display_rate,
                        temp,temp1;         //fÅr pal-init
  char                  temp2[3];           // "
  short                 mode;               //ausgewÑhlter bildschirmmodus

  if(!(strcmp(argv[1],"/e")))
  {
    savefile=fopen("hugi.dat","wb");
    for(i=0;i<TEXT_LEN;i++)
    {
      fputc(txtdata[i],savefile);
    }
    fclose(savefile);
    exit(0);
  }

  //-bildschirm-initialisierung teil 1: vesa-test
  VBE_Init();
  if(!VBE_Test())
  {
    printf("vbe 2.0 not detected\n");
  }
  //suche modus 640x480x256
  if(-1==(mode=VBE_FindMode(640,480,8)))
  {
    printf("error: screen mode not found. try using univbe!\n");
    exit(1);
  }
  //lfb vorhanden?
  if(lfb_detected=VBE_IsModeLinear(mode))
  {
    printf("linear frame buffer detected\n");
    realscreen.mem=VBE_GetVideoPtr(mode);
  }
  else
  {
    printf("linear frame buffer not detected\n");
    realscreen.mem=(char *)0xa0000;
  }

  //-mouse-initialisierung teil 1
  if(!mouse_init())
  {
    printf("error: mouse driver not found\n");
    exit(1);
  }

  imsinitstruct is;
  imsFillDefaults(is);
  is.bufsize=32768;
  is.reverb=25;
  is.chorus=50;
  if (!imsInit(is))
  {
    cputs("could not init sound\r\n");
    return;
  }

  xmodule xmod;
  mbinfile fil;
  fil.open(SONG_XM, SONG_XM_LEN, fil.openro);
  if (xmpLoadModule(xmod, fil))
  {
    cputs("could not load module\r\n");
    return;
  }
  fil.close();

  if (!xmpLoadSamples(xmod))
  {
    cputs("could not upload samples\r\n");
    return;
  }

  //-virtual-screen-initialisierung
  vscreen_new(&vs_bkground,640,480);
  vscreen_new(&vs_page1,640,VIEWER_TEXTWINDOW_Y2-VIEWER_TEXTWINDOW_Y1);
  vscreen_new(&vs_page2,640,VIEWER_TEXTWINDOW_Y2-VIEWER_TEXTWINDOW_Y1);
  vscreen_new(&(mouse.vs_cursor),10,20);
  vscreen_new(&(mouse.vs_bkground),10,20);

  //-paletten-initialisierung, laden des pcx-cursors
  mymalloc((void **)&pal_saved,768,0);
  pal8_get(pal_saved);
  mymalloc((void **)&pal_black,768,1);
  //pcx-palette
  mymalloc((void **)&pal,768,0);
  //add. pals
  mymalloc((void **)&pal_bkpic,768,0);
  mymalloc((void **)&pal_text,768,0);
  //cursor laden
  pcx256_memload(pcxcursor,&(mouse.vs_cursor),NULL);

  //-font-initialisierung
  mymalloc((void **)&font,4096,0);
  if(!font_load("hugi.fnt",16,font))
  {
    printf("error: font data not found\n");
    exit(1);
  }

  //-bildschirm-initialisierung teil 2: bildschirmmodus setzen
  VBE_SetMode(mode,1,1);
  //realscreen-"virtual screen" initialisieren
  realscreen.size=((realscreen.x=640)*(realscreen.y=480));

  //-titlepic anzeigen
  hugi_titlepicture_show();

  //-hintergrundbild laden
  pcx256_memload(pcxpic1,&vs_bkground,pal);

  //-statusbar-color setzen
  pal[VIEWER_STATUSBAR_COLOR*3+0]=63;
  pal[VIEWER_STATUSBAR_COLOR*3+1]=0;
  pal[VIEWER_STATUSBAR_COLOR*3+2]=0;

  //-mouse-colors setzen
  pal[VIEWER_MOUSE_COLOR_MAIN*3+0]=63;
  pal[VIEWER_MOUSE_COLOR_MAIN*3+1]=63;
  pal[VIEWER_MOUSE_COLOR_MAIN*3+2]=63;
  pal[VIEWER_MOUSE_COLOR_BORDER*3+0]=0;
  pal[VIEWER_MOUSE_COLOR_BORDER*3+1]=0;
  pal[VIEWER_MOUSE_COLOR_BORDER*3+2]=0;

  //-pal_bkpic, pal_text initialisieren
  memcpy(pal_bkpic,pal,768);
  memcpy(pal_text ,pal,768);

  //-zusÑtzliche palettenfarben setzen
  //pal_bkpic: bkpic-farben
  for(temp=0;temp<16;temp++)
  {
    for(temp1=16;temp1<=96;temp1+=16)
    {
      pal_bkpic[(temp+temp1)*3+0]=pal[temp*3+0];
      pal_bkpic[(temp+temp1)*3+1]=pal[temp*3+1];
      pal_bkpic[(temp+temp1)*3+2]=pal[temp*3+2];
    }
  }

  //pal_text: text-farben
  for(temp=1;temp<=6;temp++)
  {
    switch(temp)
    {
      case 6: temp2[0]=63; temp2[1]=63; temp2[2]=63; break; //farbe wei·
      case 5: temp2[0]=63; temp2[1]=63; temp2[2]=0;  break; //farbe gelb
      case 4: temp2[0]=63; temp2[1]=0;  temp2[2]=0;  break; //farbe rot
      case 3: temp2[0]=0;  temp2[1]=0;  temp2[2]=63; break; //farbe blau
      case 2: temp2[0]=0;  temp2[1]=63; temp2[2]=63; break; //farbe zyan
      case 1: temp2[0]=0;  temp2[1]=63; temp2[2]=0;  break; //farbe grÅn
    }
    for(temp1=1;temp1<16;temp1++)
    {
      pal_text[(temp*16+temp1)*3+0]=temp2[0];
      pal_text[(temp*16+temp1)*3+1]=temp2[1];
      pal_text[(temp*16+temp1)*3+2]=temp2[2];
    }
  }

  //bildschirm schwarz machen
  pal8_set(pal_black);
  //geladenes hintergrundbild anzeigen
  vscreen_copy_full(&realscreen,&vs_bkground);

  //-mouse-initialisierung fÅr den spezifischen bildschirmmodus
  mouse_init();
  mouse_radius_x_set(0,629);
  mouse_radius_y_set(0,459);
  mouse_pos_set     (0,0);

  //-musik-start
  xmpPlayModule(xmod);

  //-textviewer-instanz mit hinweis auf index-artikel aufrufen
  viewer(txtdata,"index",0,0);
  //ausfaden
  pal8_fadeto(pal_black,1,sound_off);

  //-musik-schlu·
  xmpStopModule();

  //-schlu·pic anzeigen
  hugi_closingpicture_show();

  //-bildschirm-close
  pal8_fadeto(pal_black,1,sound_off);
  VBE_SetMode (3,0,1);
  VBE_Done();

  //-paletten-freigabe
  pal8_set(pal_saved);
  free(pal_saved);
  free(pal);
  free(pal_bkpic);
  free(pal_text);
  free(pal_black);

  //-virtual-screen-freigabe
  vscreen_kill(&vs_bkground);
  vscreen_kill(&vs_page1);
  vscreen_kill(&vs_page2);
  vscreen_kill(&(mouse.vs_cursor));
  vscreen_kill(&(mouse.vs_bkground));

  //-mit restlichen pointern aufrÑumen
  free(font);

  //-ims-close
  xmpFreeModule(xmod);
  imsClose();
}
