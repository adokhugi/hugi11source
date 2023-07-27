//adok's vertical-retrace-funktionen

#define __ADVRET__

#ifdef __midasdll_h
  //frame-counter fr midas-bildschirm-sync
  volatile frame_counter=0;

  void MIDAS_CALL preVR()
  /*
    aufgabe:       ver„ndern des frame-counters beim abspielen von musik vor
                   jedem vertikalen retrace, sofern midas benutzt wird
    vorgangsweise: inkrementieren der globalen volatile-variable frame_counter
    hinweis:       diese funktion muá mit MIDASsetTimerCallbacks zum
                   vor-dem-vertikalen-retrace-aufrufen eingetragen werden
  */
  {
    frame_counter++;
  }
#endif

void retrace_vertical_wait(signed char sound_off)
/*
  aufgabe:       abwarten des vertikalen retraces
                 funktioniert sowohl mit midas als auch ohne
  vorgangsweise: wenn midas (sound_off==0)
                   warten, bis sich frame_counter ver„ndert hat
                 andernfalls (sound_off!=0)
                   *) eventuell ablaufenden vertikalen retrace zuende warten
                   *) auf einen neuen vertikalen retrace warten
  parameter:     sound_off: soll midas bercksichtigt werden oder nicht?
  hinweis:       diese funktion wird benutzt, um ein flackern w„hrend des
                 ver„nderns des grafikkarten-status zu verhindern
*/
{
#ifdef __midasdll_h
  if(sound_off)
  {
#endif
    while (inp(0x3da)&8);
    while (!(inp(0x3da)&8));
#ifdef __midasdll_h
  }
  else
  {
    long temp=frame_counter;
    while(temp==frame_counter);
  }
#endif
}