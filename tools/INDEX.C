//hugi-internes index-erzeugprogramm

#include <stdio.h>

void main(int argc,char **argv)
{
  FILE *txtfile,
       *idxfile;
  long length,
       tempoffset;
  int  tempchar,
       i;

  //kommandozeile ok?
  if(argc!=3)
  {
    printf("error: zu viele/wenige kommandozeilenparameter\n"
           "syntax: index txtfile.ext idxfile.ext\n");
    exit(1);
  }

  //txtfile îffnen
  if(NULL==(txtfile=fopen(argv[1],"rb")))
  {
    printf("error: textfile nicht vorhanden\n");
    exit(1);
  }
  //idxfile erzeugen
  idxfile=fopen(argv[2],"wb");

  /*main: txtfile nach ˛ absuchen, dabei pro erfolglosem fgetc() length++
          wenn gefunden -> length in idxfile abspeichern, sofern length != 0
                           ˛ in idxfile schreiben
                           virtuellen filename bis zum nÑchsten ˛ einlesen
                                               in idxfile abspeichern
                           ˛ in idxfile schreiben
                           offset nach zweitem ˛ ermitteln
                                                 in idxfile abspeichern
                           length auf 0 setzen*/
  length=0;
  while(EOF!=(tempchar=fgetc(txtfile)))
    if(tempchar!=254)
      length++;
    else
    {
      if(length!=0) fwrite(&length,sizeof(long),1,idxfile);
      fputc(254,idxfile);
      while(254!=(tempchar=fgetc(txtfile)))
        fputc(tempchar,idxfile);
      fputc(254,idxfile);
      tempoffset=ftell(txtfile)+1;
      fwrite(&tempoffset,sizeof(long),1,idxfile);
      length=0;
    }
  fwrite(&length,sizeof(long),1,idxfile);

  //txtfile, idxfile schlie·en
  fclose(txtfile);
  fclose(idxfile);

  //testweises auslesen von idxfile
  idxfile=fopen(argv[2],"rb");
  i=0;
  while(EOF!=(tempchar=fgetc(idxfile)))
    if(254==tempchar)
    {
      while(254!=(tempchar=fgetc(idxfile)))
        printf("%c",tempchar);
      fread(&tempoffset,sizeof(long),1,idxfile);
      fread(&length,sizeof(long),1,idxfile);
      printf(": offset %ld length %ld\n",tempoffset,length);
      //fÅr "schîne" ausgabe sorgen
      i++;
      if(i==23)
      {
        i=0;
        printf("taste drÅcken..\n");
        getch();
      }
    }
  fclose(idxfile);
}