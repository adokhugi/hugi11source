//adok's druckerfunktionen

#define __ADPRINT__

void printer_char_print(char ascii)
/*
  aufgabe:       senden eines zeichens (ascii/steuercode) an drucker
  vorgangsweise: *) auf ende des busy-zustands warten
                 *) zeichen an drucker senden
                 *) strobe senden
                 *) auf acknowledgement warten
  parameter:     ascii: zu sendendes zeichen
*/
{
  while(0==(inp(0x379)&128));
  outp(0x378,ascii);
  outp(0x37a,inp(0x37a)|1);
  outp(0x37a,inp(0x37a)&(~1));
  while(1==(inp(0x379)&64));
}