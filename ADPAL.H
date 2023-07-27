//adok's pal-routinen

#define __ADPAL__

void pal8_get(char *pal)
/*
  aufgabe:       auslesen der aktuellen 256-farben-farbpalette und schreiben
                 in einen array
  vorgangsweise: *) setzen von port 0x3c7 auf 0
                 schleife von 0 bis 767
                   *) den inhalt des elements der palette mit der nummer des
                      schleifenz„hlers von port 0x3c9 lesen und in den array
                      schreiben
  parameter:     *pal: pointer auf die zu beschreibenden palettendaten
*/
{
  int counter;

  outp(0x3c7,0);
  for(counter=0;counter<=767;counter++)
    *(pal+counter)=inp(0x3c9);
}

void pal8_set(char *pal)
/*
  aufgabe:       setzen einer 256-farben-farbpalette, welche in einem array
                 gespeichert wurde
  vorgangsweise: *) setzen von port 0x3c8 auf 0
                 schleife von 0 bis 767
                   *) den inhalt des elements der palette mit der nummer des
                      schleifenz„hlers auf port 0x3c9 schreiben
  parameter:     *pal: pointer auf die palettendaten
*/
{
  int counter;

  outp(0x3c8,0);
  for(counter=0;counter<=767;counter++)
    outp(0x3c9,*(pal+counter));
}

#ifdef __ADVRET__
  void pal8_fadeto(char *pal,int speed,signed char sound_off)
  /*
    aufgabe:       fadet den bildschirm zu einer bestimmten farbpalette ber
    vorgangsweise: *) kopieren der aktuellen farbpalette in einen tempor„ren
                      speicherbereich
                      pal8_get(pal_temp)
                   do-while-schleife, solange pal_changed wird
                     *) setzen des pal_changed-flags auf 0
                     *) counter von 0 bis 767 durchlaufen lassen
                        *) ist pal_temp[counter] kleiner als pal[counter]?
                           wenn ja:
                             *) setze pal_changed auf -1
                             *) pal_temp[counter] um speed erh”hen
                             *) wenn pal_temp[counter] nun gr”áer als
                                pal[counter], dann pal_temp[counter] auf
                                pal[counter] setzen
                           wenn nein:
                             *) ist pal_temp[counter] gr”áer als pal[counter]?
                             wenn ja:
                               *) setze pal_changed auf -1
                               *) pal_temp[counter] um speed erniedrigen
                               *) wenn (signed char)pal_temp[counter] nun
                                  kleiner als pal[counter], dann
                                  pal_temp[counter] auf pal[counter] setzen
                     *) auf vertikalen retrace warten
                     *) farbpalette setzen: pal8_set(pal_temp)
    parameter:     *pal:      palettendaten, auf die gefadet werden soll
                   speed:     zahl, um die die palettendaten pro durchlauf
                              erh”ht werden sollen
                   sound_off: l„uft midas oder nicht?
  */
  {
    char        pal_temp[768];
    signed char pal_changed;
    int         counter;

    pal8_get(pal_temp);

    do
    {
      pal_changed=0;
      for(counter=0;counter<768;counter++)
        if(pal_temp[counter]<pal[counter])
        {
          pal_changed=-1;
          pal_temp[counter]+=speed;
          if(pal_temp[counter]>pal[counter])
            pal_temp[counter]=pal[counter];
        }
        else
          if(pal_temp[counter]>pal[counter])
          {
            pal_changed=-1;
            pal_temp[counter]-=speed;
            if((signed char)pal_temp[counter]<pal[counter])
              pal_temp[counter]=pal[counter];
          }
      retrace_vertical_wait(sound_off);
      pal8_set(pal_temp);
    } while(pal_changed);
  }
#endif