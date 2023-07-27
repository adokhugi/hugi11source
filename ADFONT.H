//adok's font-routinen
//die font-dateien mssen im biosfont-format (breite 8, h”he beliebig) sein

#define __ADFONT__

signed char font_load(char *file_name,char font_height,char far *font_ram)
/*
  aufgabe:       laden einer font-datei in einen speicherbereich
  vorgangsweise: *) ”ffnen der font-datei
                 *) kopieren der ersten 256*font_height bytes aus der datei
                    in den speicherbereich
                 *) schlieáen der font-datei
  parameter:     *file_name:  string, der den dateinamen der font-datei
                              enth„lt
                 font_height: h”he des fonts (normalerweise 8, 14 oder 16)
                 *font_ram:   speicherbereich, in den der font geschrieben
                              werden soll
  rckgabe:      0, wenn laden des fonts miálungen
*/
{
  int i;
  for(i=0;i<(font_height<<8);i++)
  font_ram[i]=externfont[i];
  return(-1);
}

#ifdef __ADVSCRN__
  void font_char_print_screen(struct ADOK_SCREEN *vscreen,char *font_ram,
                              char font_height,
                              int x,int y,char ascii,char color)
  /*
    aufgabe:       anzeigen eines zeichens eines geladenen fonts in einem
                   virtual screen auf angegebener position in angegebener
                   farbe
    vorgangsweise: *) berechnen des offsets innerhalb von *font_ram, an dem
                      das gewnschte zeichen gespeichert ist:
                      offset=ascii*font_height
                   schleife von 0 bis 15 durchlaufen
                     *) char_line auf *(font_ram+offset) setzen
                     variable bit_number in schleife von 0 bis 7 durchlaufen
                       *) ist bit bit_number gesetzt?
                          temp=1<<bit_number;
                          if(temp==(char_line&temp)) ...
                          wenn ja: pixel setzen
                              *((*vscreen).mem+(*vscreen).x*y+x+7-bit_number)=
                              color
                     *) y um 1 erh”hen
                     *) offset um 1 erh”hen

    parameter:     *vscreen:    pointer auf den virtual screen, auf den
                                geschrieben werden soll
                   *font_ram:   pointer auf die font-daten
                   font_height: zeichenh”he
                   x:           x-koordinate, wo ausgegeben werden soll
                   y:           y-koordinate, wo ausgegeben werden soll
                   ascii:       zeichen, das ausgegeben werden soll
                   color:       farbe, in der das zeichen ausgegeben werden
                                soll
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
          *((*vscreen).mem+(*vscreen).x*y+x+7-bit_number)=color;
      }
      y++;
      offset++;
    }
  }
#endif
