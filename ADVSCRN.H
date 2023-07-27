//adok's virtual-screen-routinen

#define __ADVSCRN__
#ifndef __ADALLOC__
  #include "adalloc.h"
#endif

#include "vesavbe.h"

//struktur der virtual-screens
struct ADOK_SCREEN
{
  int  x;
  int  y;
  char *mem;
  long size;
};

signed char        lfb_detected;
struct ADOK_SCREEN realscreen;

void vscreen_new(struct ADOK_SCREEN *vscreen,int x,int y)
/*
  aufgabe:       erzeugen eines neuen virtual-screens
  vorgangsweise: *) eintragen der x- und der y-grî·e
                 *) multiplikation von x und y
                    => speicherplatzbedarf bei 256 farben
                 *) reservierung des speichers, eintragen in mem
  parameter:     *vscreen: pointer auf die virtual-screen-struktur
                 x:        grî·e in x-richtung des virtual screens
                 y:        grî·e in y-richtung des virtual screens
*/
{
  char *mem;

  (*vscreen).x=x;
  (*vscreen).y=y;
  (*vscreen).size=(long)x*y;
  mymalloc((void**)&mem,(*vscreen).size,0);
  (*vscreen).mem=mem;
}

void vscreen_kill(struct ADOK_SCREEN *vscreen)
/*
  aufgabe:       entfernen eines virtual screens
  vorgangsweise: *) freigabe des speichers
                 *) Åberschreiben des pointers mit NULL (=> zugriff illegal)
  parameter:     *vscreen: pointer auf die virtual-screen-struktur
*/
{
  free((*vscreen).mem);
  (*vscreen).mem=NULL;  //sicherheitsma·nahme!
}

void vscreen_copy_full(struct ADOK_SCREEN *vs_dest,
                       struct ADOK_SCREEN *vs_source)
/*
  aufgabe:       kopie eines virtual screens / des real screens in einen
                 anderen, wobei der speicherplatzbedarf des quellscreens
                 angegeben werden mu·.
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) vs_source.x==vs_dest.x
                 *) vs_source.y<=vs_dest.y
  vorgangsweise: memcpy benutzen ;)
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
*/
{
  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
    memcpy((*vs_dest).mem,(*vs_source).mem,(*vs_source).size);
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      char *mem     =(*vs_source).mem,
           counter,
           max      =(*vs_dest).size>>16;
      int  rest     =(*vs_dest).size&0xffff;

      for(counter=0;counter<max;counter++)
      {
        VBE_SetBank(counter);
        memcpy((char*)0xa0000,mem,65536);
        mem+=65536;
      }
      VBE_SetBank(max);
      memcpy((char*)0xa0000,mem,rest);
    }
    else
    {
      char *mem     =(*vs_dest).mem,
           counter,
           max      =(*vs_source).size>>16;
      int  rest     =(*vs_source).size&0xffff;

      for(counter=0;counter<max;counter++)
      {
        VBE_SetBank(counter);
        memcpy(mem,(char*)0xa0000,65536);
        mem+=65536;
      }
      VBE_SetBank(max);
      memcpy(mem,(char*)0xa0000,rest);
    }
  }
}

void vscreen_copy_part(struct ADOK_SCREEN *vs_dest,
                       struct ADOK_SCREEN *vs_source,
                       int x1,int y1,int x2,int y2,
                       int x_left,int y_top)
/*
  aufgabe:       kopie des durch x1/y1 (links-oben) und x2/y2 (rechts-unten)
                 angegebenen ausschnitts von vs_source nach vs_dest, wobei die
                 linke obere ecke in vs_dest, von der aus ausgegeben wird,
                 durch x_left/y_top angegeben wird
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) x2-x1+x_left<(*vs_dest).x
                 *) y2-y1+y_top<(*vs_dest).y
  vorgangsweise: *) pointer auf die erste zu beschreibende zeile in *vs_dest
                    berechnen: pointer_dest=(*vs_dest).x*y_top+x_left
                 *) pointer auf die erste zu lesende zeile in *vs_source
                    berechnen: pointer_source=(*vs_source).x*y1+x1
                 *) zeilenlÑnge des zu schreibenden bereichs berechnen:
                    x_len=1+x2-x1
                 *) spaltenlÑnge des zu schreibenden bereichs berechnen:
                    y_len=1+y2-y1
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<y_len:
                   *) mit memcpy() x_len bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erhîhen
                   *) pointer_source um (*vs_source).x erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x1:         linke x-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 y1:         obere y-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 x2:         rechte x-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 y2:         untere y-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 x_left:     linke x-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden,
                 a) um einen ausschnitt eines virtual screens auf eine
                    beliebige position in einem anderen virtual screens zu
                    bringen
                 b) um einen ausschnitt eines virtual screens auf denselben
                    virtual screen, aber auf eine etwas weiter oben gelegene
                    position zu kopieren, wobei quell- und zielbild einander
                    "Åberschneiden"
*/
{
  int  y_counter     =0,
       x_len         =1+x2-x1,
       y_len         =1+y2-y1;
  long pointer_source=(*vs_source).x*y1+x1,
       pointer_dest  =(*vs_dest).x*y_top+x_left;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<y_len)
    {
      memcpy((*vs_dest).mem+pointer_dest,
             (*vs_source).mem+pointer_source,
             x_len);
      pointer_dest+=(*vs_dest).x;
      pointer_source+=(*vs_source).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_dest_temp>>16);
          *((char*)0xa0000+(pointer_dest_temp&0xffff))=
            *((*vs_source).mem+pointer_source_temp);
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          *((*vs_dest).mem+pointer_dest_temp)=
            *((char*)0xa0000+(pointer_source_temp&0xffff));
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
  }
}

void vscreen_copy_part_transparent(struct ADOK_SCREEN *vs_dest,
                                   struct ADOK_SCREEN *vs_source,
                                   int x1,int y1,int x2,int y2,
                                   int x_left,int y_top,
                                   int transparent_color)
/*
  aufgabe:       kopie des durch x1/y1 (links-oben) und x2/y2 (rechts-unten)
                 angegebenen ausschnitts von vs_source nach vs_dest, wobei die
                 linke obere ecke in vs_dest, von der aus ausgegeben wird,
                 durch x_left/y_top angegeben wird und transparente stellen
                 in vs_source nicht gezeichnet werden
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) x2-x1+x_left<(*vs_dest).x
                 *) y2-y1+y_top<(*vs_dest).y
  vorgangsweise: *) pointer auf die erste zu beschreibende zeile in *vs_dest
                    berechnen: pointer_dest=(*vs_dest).x*y_top+x_left
                 *) pointer auf die erste zu lesende zeile in *vs_source
                    berechnen: pointer_source=(*vs_source).x*y1+x1
                 *) zeilenlÑnge des zu schreibenden bereichs berechnen:
                    x_len=1+x2-x1
                 *) spaltenlÑnge des zu schreibenden bereichs berechnen:
                    y_len=1+y2-y1
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<y_len:
                   *) mit memcpy() x_len bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erhîhen
                   *) pointer_source um (*vs_source).x erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x1:         linke x-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 y1:         obere y-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 x2:         rechte x-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 y2:         untere y-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 x_left:     linke x-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden,
                 a) um einen ausschnitt eines virtual screens auf eine
                    beliebige position in einem anderen virtual screens zu
                    bringen
                 b) um einen ausschnitt eines virtual screens auf denselben
                    virtual screen, aber auf eine etwas weiter oben gelegene
                    position zu kopieren, wobei quell- und zielbild einander
                    "Åberschneiden"
*/
{
  int  x_counter     =0,
       y_counter     =0,
       temp,
       x_len         =1+x2-x1,
       y_len         =1+y2-y1;
  long pointer_source=(*vs_source).x*y1+x1,
       pointer_dest  =(*vs_dest).x*y_top+x_left;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<y_len)
    {
      for(x_counter=0;x_counter<x_len;x_counter++)
      {
        temp=*((*vs_source).mem+pointer_source);
        if(temp!=transparent_color)
          *((*vs_dest).mem+pointer_dest)=temp;
        pointer_dest++;
        pointer_source++;
      }
      pointer_dest+=(*vs_dest).x-x_len;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          if(*((*vs_source).mem+pointer_source_temp))
          {
            VBE_SetBank(pointer_dest_temp>>16);
            *((char*)0xa0000+(pointer_dest_temp&0xffff))=
              *((*vs_source).mem+pointer_source_temp);
          }
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          if(*((char*)0xa0000+(pointer_source_temp&0xffff)))
          {
            *((*vs_dest).mem+pointer_dest_temp)=
              *((char*)0xa0000+(pointer_source_temp&0xffff));
          }
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
  }
}

void vscreen_copy_part_reverse(struct ADOK_SCREEN *vs_dest,
                               struct ADOK_SCREEN *vs_source,
                               int x1,int y1,int x2,int y2,
                               int x_left,int y_top)
/*
  aufgabe:       kopie des durch x1/y1 (links-oben) und x2/y2 (rechts-unten)
                 angegebenen ausschnitts von vs_source nach vs_dest, wobei die
                 linke obere ecke in vs_dest, von der aus ausgegeben wird,
                 durch x_left/y_top angegeben wird
                 der unterschied zum normalen vscreen_copy_part ist, da· hier
                 bei der untersten zeile zu kopieren angefangen wird. siehe
                 hinweis
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) x2-x1+x_left<(*vs_dest).x
                 *) y2-y1+y_top<(*vs_dest).y
  vorgangsweise: *) zeilenlÑnge des zu schreibenden bereichs berechnen:
                    x_len=1+x2-x1
                 *) spaltenlÑnge des zu schreibenden bereichs berechnen:
                    y_len=1+y2-y1
                 *) pointer auf die erste zu beschreibende zeile in *vs_dest
                    berechnen:
                    pointer_dest=(*vs_dest).x*(y_top+y_len-1)+x_left
                 *) pointer auf die erste zu lesende zeile in *vs_source
                    berechnen: pointer_source=(*vs_source).x*(y1+y_len-1)+x1
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<y_len:
                   *) mit memcpy() x_len bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erniedrigen
                   *) pointer_source um (*vs_source).x erniedrigen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x1:         linke x-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 y1:         obere y-koordinate des bereichs in vs_source, von
                             dem gelesen werden soll
                 x2:         rechte x-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 y2:         untere y-koordinate des bereichs in vs_source,
                             von dem gelesen werden soll
                 x_left:     linke x-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden,
                 a) um einen ausschnitt eines virtual screens auf eine
                    beliebige position in einem anderen virtual screens zu
                    bringen
                 b) um einen ausschnitt eines virtual screens auf denselben
                    virtual screen, aber auf eine etwas weiter unten gelegene
                    position zu kopieren, wobei quell- und zielbild einander
                    "Åberschneiden"
*/
{
  int  y_counter     =0,
       x_len         =1+x2-x1,
       y_len         =1+y2-y1;
  long pointer_source=(*vs_source).x*(y1+y_len-1)+x1,
       pointer_dest  =(*vs_dest).x*(y_top+y_len-1)+x_left;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<y_len)
    {
      memcpy((*vs_dest).mem+pointer_dest,
             (*vs_source).mem+pointer_source,
             x_len);
      pointer_dest-=(*vs_dest).x;
      pointer_source-=(*vs_source).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_dest_temp>>16);
          *((char*)0xa0000+(pointer_dest_temp&0xffff))=
            *((*vs_source).mem+pointer_source_temp);
        }
        pointer_dest-=(*vs_dest).x;
        pointer_source-=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<y_len)
      {
        for(x_counter=0;x_counter<x_len;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          *((*vs_dest).mem+pointer_dest_temp)=
            *((char*)0xa0000+(pointer_source_temp&0xffff));
        }
        pointer_dest-=(*vs_dest).x;
        pointer_source-=(*vs_source).x;
        y_counter++;
      }
    }
  }
}

void vscreen_copy_part_dest(struct ADOK_SCREEN *vs_dest,
                            struct ADOK_SCREEN *vs_source,
                            int x_left,int y_top)
/*
  aufgabe:       kopie des kompletten *vs_source nach *vs_dest ab
                 position x_left/y_top in *vs_dest
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) vs_source.x<=vs_dest.x
                 *) vs_source.y<=vs_dest.y
                 *) (*vs_source).x+x_left<(*vs_dest).x
                 *) (*vs_source).y+y_top<(*vs_dest).y
  vorgangsweise: *) pointer auf die erste zu beschreibende zeile in *vs_dest
                    berechnen: pointer_dest=(*vs_dest).x*y_top+x_left
                 *) pointer_source auf 0 setzen
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<(*vs_source).y:
                   *) mit memcpy() (*vs_source).x bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erhîhen
                   *) pointer_source um (*vs_source).x erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x_left:     linke x-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden, um ein sprite bzw.
                 mauscursor (=vs_source) auf den bildschirm (=vs_dest) zu
                 bringen
*/
{
  int  y_counter     =0;
  long pointer_source=0,
       pointer_dest  =(*vs_dest).x*y_top+x_left;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<(*vs_source).y)
    {
      memcpy((*vs_dest).mem+pointer_dest,
             (*vs_source).mem+pointer_source,
             (*vs_source).x);
      pointer_dest+=(*vs_dest).x;
      pointer_source+=(*vs_source).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<(*vs_source).y)
      {
        for(x_counter=0;x_counter<(*vs_source).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_dest_temp>>16);
          *((char*)0xa0000+(pointer_dest_temp&0xffff))=
            *((*vs_source).mem+pointer_source_temp);
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<(*vs_source).y)
      {
        for(x_counter=0;x_counter<(*vs_source).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          *((*vs_dest).mem+pointer_dest_temp)=
            *((char*)0xa0000+(pointer_source_temp&0xffff));
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
  }
}

void vscreen_copy_part_dest_transparent(struct ADOK_SCREEN *vs_dest,
                                        struct ADOK_SCREEN *vs_source,
                                        int x_left,int y_top,
                                        int transparent_color)
/*
  aufgabe:       kopie des kompletten *vs_source nach *vs_dest ab
                 position x_left/y_top in *vs_dest; farbe transparent_color in
                 *vs_source wird nicht geschrieben
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) vs_source.x<=vs_dest.x
                 *) vs_source.y<=vs_dest.y
                 *) (*vs_source).x+x_left<(*vs_dest).x
                 *) (*vs_source).y+y_top<(*vs_dest).y
  vorgangsweise: *) pointer auf die erste zu beschreibende zeile in *vs_dest
                    berechnen: pointer_dest=(*vs_dest).x*y_top+x_left
                 *) pointer_source auf 0 setzen
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<(*vs_source).y:
                   for-schleife von x_counter von 0 bis <(*vs_source).x
                     *) setze temp auf *((*vs_source).mem+pointer_source)
                     *) wenn temp!=transparent_color - byte schreiben
                        *((*vs_dest).mem+pointer_dest)=temp
                     *) pointer_dest um 1 erhîhen
                     *) pointer_source um 1 erhîhen
                   *) pointer_dest um ((*vs_dest).x-(*vs_source).x) erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x_left:     linke x-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_dest, in
                             den kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden, um ein sprite bzw.
                 mauscursor (=vs_source) auf den bildschirm (=vs_dest) zu
                 bringen
*/
{
  int  x_counter,
       y_counter     =0,
       temp;
  long pointer_source=0,
       pointer_dest  =(*vs_dest).x*y_top+x_left;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<(*vs_source).y)
    {
      for(x_counter=0;x_counter<(*vs_source).x;x_counter++)
      {
        temp=*((*vs_source).mem+pointer_source);
        if(temp!=transparent_color)
          *((*vs_dest).mem+pointer_dest)=temp;
        pointer_dest++;
        pointer_source++;
      }
      pointer_dest+=(*vs_dest).x-(*vs_source).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      while(y_counter<(*vs_source).y)
      {
        for(x_counter=0;x_counter<(*vs_source).x;x_counter++)
        {
          temp=*((*vs_source).mem+pointer_source);
          if(temp!=transparent_color)
          {
            VBE_SetBank(pointer_dest>>16);
            *((char*)0xa0000+(pointer_dest&0xffff))=temp;
          }
          pointer_dest++;
          pointer_source++;
        }
        pointer_dest+=(*vs_dest).x-(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      while(y_counter<(*vs_source).y)
      {
        for(x_counter=0;x_counter<(*vs_source).x;x_counter++)
        {
          VBE_SetBank(pointer_source>>16);
          temp=*((char*)0xa0000+(pointer_source&0xffff));
          if(temp!=transparent_color)
          {
            *((*vs_dest).mem+pointer_dest)=temp;
          }
          pointer_dest++;
          pointer_source++;
        }
        pointer_dest+=(*vs_dest).x-(*vs_source).x;
        y_counter++;
      }
    }
  }
}

//! ab hier anpassen

void vscreen_copy_part_source(struct ADOK_SCREEN *vs_dest,
                              struct ADOK_SCREEN *vs_source,
                              int x_left,int y_top)
/*
  aufgabe:       kopie eines teils von *vs_source, der bei x_left/y_top
                 beginnt und die breite (*vs_dest).x und die hîhe (*vs_dest).y
                 hat, nach *vs_dest
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) vs_source.x>=vs_dest.x
                 *) vs_source.y>=vs_dest.y
                 *) (*vs_dest).x+x_left<(*vs_source).x
                 *) (*vs_dest).y+y_top<(*vs_source).y
  vorgangsweise: *) pointer auf die erste zu lesende zeile in *vs_source
                    berechnen: pointer_source=(*vs_source).x*y_top+x_left
                 *) pointer_dest auf 0 setzen
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<(*vs_dest).y:
                   *) mit memcpy() (*vs_dest).x bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erhîhen
                   *) pointer_source um (*vs_source).x erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x_left:     linke x-koordinate des bereichs in vs_source, der
                             in vs_dest kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_source, der
                             in vs_dest kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden, um einen
                 bildschirmausschnitt (bildschirm=vs_source) als sprite
                 (=vs_dest) abzusaven
*/
{
  int  y_counter     =0;
  long pointer_source=(*vs_source).x*y_top+x_left,
       pointer_dest  =0;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<(*vs_dest).y)
    {
      memcpy((*vs_dest).mem+pointer_dest,
             (*vs_source).mem+pointer_source,
             (*vs_dest).x);
      pointer_dest+=(*vs_dest).x;
      pointer_source+=(*vs_source).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<(*vs_dest).y)
      {
        for(x_counter=0;x_counter<(*vs_dest).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_dest_temp>>16);
          *((char*)0xa0000+(pointer_dest_temp&0xffff))=
            *((*vs_source).mem+pointer_source_temp);
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<(*vs_dest).y)
      {
        for(x_counter=0;x_counter<(*vs_dest).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          *((*vs_dest).mem+pointer_dest_temp)=
            *((char*)0xa0000+(pointer_source_temp&0xffff));
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
  }
}

void vscreen_copy_part_source_transparent(struct ADOK_SCREEN *vs_dest,
                                          struct ADOK_SCREEN *vs_source,
                                          int x_left,int y_top,
                                          int transparent_color)
/*
  aufgabe:       kopie eines teils von *vs_source, der bei x_left/y_top
                 beginnt und die breite (*vs_dest).x und die hîhe (*vs_dest).y
                 hat, nach *vs_dest, wobei transparente stellen ausgelassen
                 werden
  wichtig:       voraussetzung fÅr das anwenden dieser funktion ist:
                 *) vs_source.x>=vs_dest.x
                 *) vs_source.y>=vs_dest.y
                 *) (*vs_dest).x+x_left<(*vs_source).x
                 *) (*vs_dest).y+y_top<(*vs_source).y
  vorgangsweise: *) pointer auf die erste zu lesende zeile in *vs_source
                    berechnen: pointer_source=(*vs_source).x*y_top+x_left
                 *) pointer_dest auf 0 setzen
                 *) y_counter auf 0 setzen
                 ab hier wird geloopt, solange y_counter<(*vs_dest).y:
                   *) mit memcpy() (*vs_dest).x bytes von
                      (*vs_source).mem+pointer_source nach
                      (*vs_dest).mem+pointer_dest kopieren
                   *) pointer_dest um (*vs_dest).x erhîhen
                   *) pointer_source um (*vs_source).x erhîhen
                   *) y_counter um 1 erhîhen
  parameter:     *vs_dest:   pointer auf ziel-screen
                 *vs_source: pointer auf quelle-screen
                 x_left:     linke x-koordinate des bereichs in vs_source, der
                             in vs_dest kopiert werden soll
                 y_top:      obere y-koordinate des bereichs in vs_source, der
                             in vs_dest kopiert werden soll
  hinweis:       diese funktion kann dazu verwendet werden, um einen
                 bildschirmausschnitt (bildschirm=vs_source) als sprite
                 (=vs_dest) abzusaven
*/
{
  int  x_counter     =0,
       temp,
       y_counter     =0;
  long pointer_source=(*vs_source).x*y_top+x_left,
       pointer_dest  =0;

  if((lfb_detected)||
     (((*vs_dest).mem!=realscreen.mem)&&((*vs_source).mem!=realscreen.mem)))
  {
    while(y_counter<(*vs_dest).y)
    {
      for(x_counter=0;x_counter<(*vs_dest).x;x_counter++)
      {
        temp=*((*vs_source).mem+pointer_source);
        if(temp!=transparent_color)
          *((*vs_dest).mem+pointer_dest)=temp;
        pointer_dest++;
        pointer_source++;
      }
      pointer_source+=(*vs_source).x-(*vs_dest).x;
      y_counter++;
    }
  }
  else
  {
    if((*vs_dest).mem==realscreen.mem)
    {
      int  x_counter;

      while(y_counter<(*vs_dest).y)
      {
        for(x_counter=0;x_counter<(*vs_dest).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_dest_temp>>16);
          if(*((*vs_source).mem+pointer_source_temp))
          {
            *((char*)0xa0000+(pointer_dest_temp&0xffff))=
              *((*vs_source).mem+pointer_source_temp);
          }
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
    else
    {
      int  x_counter;

      while(y_counter<(*vs_dest).y)
      {
        for(x_counter=0;x_counter<(*vs_dest).x;x_counter++)
        {
          long pointer_dest_temp  =pointer_dest+x_counter;
          long pointer_source_temp=pointer_source+x_counter;

          VBE_SetBank(pointer_source_temp>>16);
          if(*((char*)0xa0000+(pointer_source_temp&0xffff)))
          {
            *((*vs_dest).mem+pointer_dest_temp)=
              *((char*)0xa0000+(pointer_source_temp&0xffff));
          }
        }
        pointer_dest+=(*vs_dest).x;
        pointer_source+=(*vs_source).x;
        y_counter++;
      }
    }
  }
}
