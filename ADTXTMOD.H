//adok's textmode-funktionen

#define __ADTXTMOD__

void textmode_enter(void);
/*
  aufgabe:       betreten/bildschirml”schen des textmodus 3
*/
#pragma aux textmode_enter = \
  "mov ax,3" \
  "int 0x10" \
modify [ax];

void textmode_char_print(char ascii,char attrib,char x,char y)
/*
  aufgabe:       ausgeben eines ascii-zeichens mit einem bestimmten
                 farbattribut an einer bestimmten stelle im bildschirm
  vorgangsweise: *) berechnen des offsets, wo das zeichen ausgegeben werden
                    soll:
                    offset=0xb8000+(80*y+x)<1
                 *) zeichen auf offset ausgeben
                    *offset=ascii
                 *) attribut ausgeben
                    *(offset+1)=attrib
  parameter:     *) ascii:  auszugebendes ascii-zeichen
                 *) attrib: farbattribut
                 *) x:      x-position
                 *) y:      y-position
*/
{
  long offset=0xb8000+160*(long)y+2*(long)x;

  *(char*)offset=ascii;
  *(char*)(offset+1)=attrib;
}

void textmode_string_print(char *string,char attrib,char x,char y)
/*
  aufgabe:       ausgeben eines strings mit einem bestimmten farbattribut ab
                 einer bestimmten stelle im bildschirm
  vorgangsweise: *) setze counter auf 0
                 schleife, bis string[counter] gleich '\0'
                   *) an textmode_char_print bergeben
                   *) x inkrementieren
                   *) counter inkrementieren
  parameter:     *) string: pointer auf auszugebenden string
                 *) attrib: farbattribut
                 *) x:      x-position
                 *) y:      y-position
*/
{
  int counter=0;

  while(string[counter]!='\0')
  {
    textmode_char_print(string[counter],attrib,x,y);
    x++;
    counter++;
  }
}

void cursor_hide(void);
/*
  aufgabe:       verstecken des textcursors
*/
#pragma aux cursor_hide = \
  "mov ah,1" \
  "mov cx,0xffff" \
  "int 0x10" \
modify [ax cx];

void cursor_show(void);
/*
  aufgabe:       darstellen des textcursors
*/
#pragma aux cursor_show = \
  "mov ah,1" \
  "mov cx,0x607" \
  "int 0x10" \
modify [ax cx];