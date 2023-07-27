//adok's pcx-loader

#define __ADPCX__
#ifndef __ADVSCRN__
  #include "advscrn.h"
#endif
#ifndef __ADPAL__
  #include "adpal.h"
#endif

signed char pcx256_load(char *file_name,struct ADOK_SCREEN *vscreen,char *pal)
/*
  aufgabe:       laden einer pcx256-datei in einen virtuellen screen
  wichtig:       es wird vorausgesetzt, da·
                 *) es sich um eine pcx-datei mit 256 farben handelt
                 *) (*vscreen).x der breite und
                 *) (*vscreen).y der hîhe der pcx-datei entsprechen
                 (der header der pcx-datei wird erst gar nicht ausgewertet!!!)
  vorgangsweise: *) setzen von vs_pointer auf 0
                 *) îffnen der pcx-datei
                 *) Åberspringen des headers (=die ersten 128 bytes)
                 hier wird nun geloopt, solange vs_pointer<(*vscreen).size
                   *) ein byte einlesen und in temp speichern
                   *) ist (temp&192)==192 ?
                      - wenn ja:
                          *) anzahl (number_to_write) ist temp&63
                          *) byte einlesen und in temp speichern
                          *) memset((*vscreen).mem+vs_pointer,temp,
                                    number_to_write);
                          *) vs_pointer um number_to_write erhîhen
                      - wenn nein:
                               *) *((*vscreen).mem+vs_pointer)=temp
                               *) vs_pointer um 1 erhîhen
                 *) ein byte (farbpalettenbeginn-kennung) Åberspringen
                 *) die folgenden 768 bytes einlesen und in *pal schreiben:
                    durchlaufe temp von 0 bis 767
                      *) *(pal+temp)=fgetc(file_handle)>>2
                 *) schlie·en der pcx-datei
  parameter:     file_name: dateiname der pcx-datei
                 *vscreen:  pointer auf den zu beschreibenden virtual screen
                 *pal:      pointer auf die zu beschreibenden palettendaten
                            (NULL, wenn palette nicht gesetzt werden soll)
*/
{
  long vs_pointer=0;
  int  temp;
  char number_to_write;
  FILE *file_handle;

  //datei îffnen
  if(!(file_handle=fopen(file_name,"rb")))
    return(0);

  //header Åberspringen
  fseek(file_handle,128,SEEK_SET);

  //bilddaten einlesen
  while(vs_pointer<(*vscreen).size)
  {
    temp=fgetc(file_handle);
    if(192==(temp&192))
    {
      number_to_write=temp&63;
      temp=fgetc(file_handle);
      memset((*vscreen).mem+vs_pointer,temp,number_to_write);
      vs_pointer+=number_to_write;
    }
    else
    {
      *((*vscreen).mem+vs_pointer)=temp;
      vs_pointer++;
    }
  }

  //palettendaten einlesen
  if(NULL!=pal)
  {
    fgetc(file_handle);
    for(temp=0;temp<768;temp++)
      *(pal+temp)=fgetc(file_handle)>>2;
  }

  //datei schlie·en, ende
  fclose(file_handle);
  return(-1);
}