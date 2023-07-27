//adok's pcx-loader

#define __ADPCX__
#ifndef __ADVSCRN__
  #include "advscrn.h"
#endif
#ifndef __ADPAL__
  #include "adpal.h"
#endif

signed char pcx256_memload(unsigned char pcxpic1[],struct ADOK_SCREEN *vscreen,char *pal)
{
  long vs_pointer=0;
  int  temp;
  char number_to_write;
  long offset; 

//header Åberspringen
  offset=+128;

  //bilddaten einlesen
  while(vs_pointer<(*vscreen).size)
  {
//    temp=fgetc(file_handle);
    temp=pcxpic1[offset++];

    if(192==(temp&192))
    {
      number_to_write=temp&63;
//      temp=fgetc(file_handle);
      temp=pcxpic1[offset++];
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
//    fgetc(file_handle);
    temp=pcxpic1[offset++];
    for(temp=0;temp<768;temp++)
      *(pal+temp)=(pcxpic1[offset++]>>2);
  }

  //datei schlie·en, ende
//  fclose(file_handle);
  return(-1);
}
