//adok's mouse-routinen

#define __ADMOUSE__

struct ADOK_MOUSE
{
  int x;
  int y;
  int x_old;
  int y_old;
  signed char button_left;
  signed char button_right;
  signed char button_middle;
  signed char moved;
  #ifdef __ADVSCRN__
    struct ADOK_SCREEN vs_cursor;
    struct ADOK_SCREEN vs_bkground;
  #endif
};

signed short mouse_init(void);
/*
  aufgabe:       maus-initalisierung
  vorgangsweise: int 33h, fkt. 0 aufrufen
  rÅckgabe:      0, wenn mi·lungen
*/
#pragma aux mouse_init = \
  "xor ax,ax" \
  "int 0x33" \
modify [ax] \
value  [ax];

void mouse_radius_x_set(int x_start,int x_end);
/*
  aufgabe:       setzen des radius in x-richtung, wo sich die maus bewegen
                 darf
  vorgangsweise: int 33h, fkt. 7 aufrufen
  parameter:     x_start: linke x-koordinate
                 x_end:   rechte x-koordinate
*/
#pragma aux mouse_radius_x_set = \
  "mov ax,7" \
  "int 0x33" \
modify [ax] \
parm   [cx] [dx];

void mouse_radius_y_set(int y_start,int y_end);
/*
  aufgabe:       setzen des radius in y-richtung, wo sich die maus bewegen
                 darf
  vorgangsweise: int 33h, fkt. 8 aufrufen
  parameter:     y_start: obere y-koordinate
                 y_end:   untere y-koordinate
*/
#pragma aux mouse_radius_y_set = \
  "mov ax,8" \
  "int 0x33" \
modify [ax] \
parm   [cx] [dx];

void mouse_pos_set(int x,int y);
/*
  aufgabe:       setzen der position des mauscursors
  vorgangsweise: int 33h, fkt. 4 aufrufen
  parameter:     x: x-koordinate
                 y: y-koordinate
*/
#pragma aux mouse_pos_set = \
  "mov ax,4" \
  "int 0x33" \
modify [ax] \
parm   [cx] [dx];

void mouse_status_get(struct ADOK_MOUSE *mouse)
/*
  aufgabe:       abfragen des mausstatus (x-koordinate, y-koordinate, linke,
                 rechte und mittlere taste) und schreiben in einen
                 ADOK_MOUSE-struct
                 vorher wird alte position abgespeichert
                 in (*mouse).moved wird geschrieben, ob sich die position
                 geÑndert hat
  vorgangsweise: int 33h, fkt. 3 - reicht das zur beschreibung der
                 vorgangsweise?
  parameter:     mouse: ADOK_MOUSE-struct
*/
{
  union REGS regs;

  //sichern der alten mauscursorposition
  (*mouse).x_old=(*mouse).x;
  (*mouse).y_old=(*mouse).y;

  //int 33h, fkt. 3 callen
  regs.w.ax=3;
  int386(0x33,&regs,&regs);

  //schreiben der position
  (*mouse).x=regs.w.cx;
  (*mouse).y=regs.w.dx;

  //maus bewegt?
  if(((*mouse).x!=(*mouse).x_old)||((*mouse).y!=(*mouse).y_old))
    (*mouse).moved=-1;
  else
    (*mouse).moved=0;

  //schreiben der buttons
  if((regs.w.bx&1)==1)
    (*mouse).button_left=-1;
  else
    (*mouse).button_left=0;
  if((regs.w.bx&2)==2)
    (*mouse).button_right=-1;
  else
    (*mouse).button_right=0;
  if((regs.w.bx&4)==4)
    (*mouse).button_middle=-1;
  else
    (*mouse).button_middle=0;
}

void mouse_wait(struct ADOK_MOUSE *mouse,signed char *flag)
/*
  aufgabe:       fragt so lange den mausstatus ab, bis *flag==0
  vorgangsweise: while-Schleife bis (*flag)==0, in der mouse_status_get()
                 aufgerufen wird
  parameter:     *mouse: pointer auf maus-daten
                 *flag:  pointer auf das abzufragende flag
  hinweis:       kann benutzt werden, um das programm anzuhalten, bis eine
                 bestimmte maustaste gedrÅckt wurde. z.b.:
                 mouse_wait(&mouse,&(mouse.button_right));
*/
{
  while(*flag)
    mouse_status_get(mouse);
}

void mouse_cursor_show_vga(void);
/*
  aufgabe:       mauscursor in nicht-svga-modi darstellen
*/
#pragma aux mouse_cursor_show_vga = \
  "mov ax,1" \
  "int 0x33" \
modify [ax];

void mouse_cursor_hide_vga(void);
/*
  aufgabe:       mauscursor in nicht-svga-modi verstecken
*/
#pragma aux mouse_cursor_hide_vga = \
  "mov ax,2" \
  "int 0x33" \
modify [ax];

void mouse_status_convert_textmode(struct ADOK_MOUSE *mouse)
/*
  aufgabe:       konvertiert mausposition auf textmodus-format
  vorgangsweise: *) x durch 8 teilen
                 *) y durch 8 teilen
  parameter:     *mouse:   pointer auf maus-daten
*/
{
  (*mouse).x/=8;
  (*mouse).y/=8;
}

#ifdef __ADVSCRN__
  void mouse_cursor_show(struct ADOK_MOUSE *mouse,struct ADOK_SCREEN *vscreen)
  /*
    aufgabe:       zeigt mauscursor auf aktueller x/y-position auf virtual
                   oder real screen an
    vorgangsweise: benutzen von vscreen_copy_part_dest_transparent
    parameter:     *mouse:   pointer auf maus-daten
                   *vscreen: pointer auf virtual screen
  */
  {
    vscreen_copy_part_dest_transparent(vscreen,&((*mouse).vs_cursor),
                                       (*mouse).x,(*mouse).y,0);
  }

  void mouse_cursor_bkground_save
  (struct ADOK_MOUSE *mouse,struct ADOK_SCREEN *vscreen)
  /*
    aufgabe:       speichert hintergrund auf aktueller x/y-position des
                   mauscursors auf virtual oder real screen in
                   (*mouse).bkground ab
    vorgangsweise: benutzen von vscreen_copy_part_source
    parameter:     *mouse:   pointer auf maus-daten
                   *vscreen: pointer auf virtual screen
  */
  {
    vscreen_copy_part_source(&((*mouse).vs_bkground),vscreen,
                             (*mouse).x,(*mouse).y);
  }

  void mouse_cursor_bkground_restore
  (struct ADOK_MOUSE *mouse,struct ADOK_SCREEN *vscreen)
  /*
    aufgabe:       kopiert (*mouse).bkground auf alte x/y-position des
                   mauscursors auf virtual oder real screen
    vorgangsweise: benutzen von vscreen_copy_part_dest
    parameter:     *mouse:   pointer auf maus-daten
                   *vscreen: pointer auf virtual screen
  */
  {
    vscreen_copy_part_dest(vscreen,&((*mouse).vs_bkground),
                           (*mouse).x_old,(*mouse).y_old);
  }
#endif