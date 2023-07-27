                         _
                     j$$$^ü5,_
                    j$$$'   j$$,                                  Ú Ä  ÄÄ  Ä Ä
                    `?$'   j°°$'                                     °°°°°°°
                  _,  `+, j$$$'   Ú Ä  _  Ä   ÄÄ   Ä    Ä   ÄÄ  Ä Ù  °     °
                 j$$$'+,_j$$$'    ³   j$$,                           °     °
                j$$$'   j$$$'        j°°$' °  °°   °    °°   °   °  °°  °  °
               j$$$'   j$$$'_,    ³ j$$$'                            °     °
              j$$$'   j$$$'j$$$'+,_j$$$'             _               °     °
             j$$$'   j$$$'j$$$'   j$$$' a d o_ k    j$$,  h u _g i   °°°°°°°
            j°°$'   j$$$'j$$$'   j$$$'_,           j$$$'
            `?$'   j$$$'j$$$'   j$$$'j$$$'+,_     j$$$'    _
 Ä Ä ÄÄÄ Ä ÄÄÄ`+,_j$$$'j$$$'ÄÄ j$$$'j$$$'ÄÄ j$+. j$$$'ÄÄÄ j$$, ÄÄÄÄÄ Ä  ÄÄ  Ä
   °°°°°°°       ~`?$'j$$$'   j$$$'j$$$'   j$$$'j$$$'    j°°$'         _cro#00
   °     °            `?$'   j$$$'j$$$'   j$$$'j$$$'    j$$$'
   °     °              `+,_j$$$'j$$$'   j$$$'j$$$',_,@j$$$'
   °     °                 ~`?$'j°°$'   j$$$'j$$$'   ü?$ü^'
   °  °  °°  °  °°°  °  ° ° °  °`?$' ° j$$$'j$$$'    j$$;
   °     °                       ³`+,_j$$$'j$$$'    j$$$'
   °     °                           ~`?$'j$$$'    j$$$'
   °°°°°°°                       ³        `?$'    j$$$'
 Ä Ä  ÄÄÄ Ä  ÄÄ  Ä   ÄÄÄ  Ä    ÄÄÙ                `?$'

 subject: h11src.zip

 heeyah!

 this zip.file contains:
 - submissives vesa vbe 2.0 application core
 - bin2asm by peter quiring
 - tools for compilation by adok
 - hugi11.data by hellfire, dendrite, antony, smash, adok
 - hugi11.src by adok & salami

 i've decided to finally release the source.code of hugi11 on request. in
 fact it's heavily outdated by now: it works under dos and requires vbe 2.0,
 things that were standard in 1998 but with which you would make a fool
 of yourself if you made this requirements of your programs today. it may
 be interesting, though, for new programmers, to see how the article data
 is structured, how the pictures are handled, etc.

 you need watcom c++ to compile it. we used version 10.5 - it may work with
 other versions, too, but we didn't test it.

 moreover, you need indoor music system. i didn't include it in this pack
 for copyright reasons (the author just wants the original zip.file to be
 spread, including many extra files, which would've blown up the size of this
 zip pretty much). version 0.6 of ims was used in hugi11. perhaps it also
 works with other versions. but - no warranty.

 i also used pmwlite to get the resulting .exe smaller. if you don't have it,
 it doesn't really matter. the .exe will just be uncompressed, that's all.

 the main source file is hugi.cpp. look at it if you want to know how the
 interface, especially control, itself is handled.

 the file const11.h is very important - it contains constants which define
 certain parts of the interface such as the number of lines displayed per
 page. all these constants were stored in this file to make it possible for
 the programmer (i.e. me) to change the configuration easily, after just
 re-running the compiler. (the hugi engine we use nowadays allows
 manipulation of the configuration without re-compiling, which is even more
 developer-friendly - you need not have the source code to create an issue.
 so this .h def technique is pretty outdated, too.)

 ad*.h contain some of my personal routines i used. (btw, professional
 programmers would compile them as libs instead of just writing the functions
 in the header-file. in this way they don't have to be recompiled every time
 you recompile a program that uses them. but as the routines are small and
 our computers are fast...)

 the *.asm files are used in order to incorporate the data into the final
 .exe.

 even when you have the necessary ims libs and watcom compiler, it may be a
 bit difficult to compile the interface, so i'll explain it step by step:

 1. you have to convert all the data files into a form that can be used by
    an assembler. i used peter quiring's bin2asm for this purpose; it's
    included in this pack.

    the converted data files must have the following filenames:
    - closing picture (pcx, 640x480, 256c)   : pcxpic_c.inc
    - title picture   -         "        -   : pcxpic_t.inc
    - background gfx  -         "        -   : pcxpic1.inc
    - cursor          (pcx, 10x20, 256c)     : cursor.inc
    - font            (8x16 biosfont format) : font.inc
    - module          (xm)                   : song.inc
    - articles                               : texts.inc

    note: give each file a different label or remove the labels manually
    using a text-editor afterwards. otherwise you'll get problems linking
    the interface.

 2. make sure that the sizes of the original (not converted!) articles file
    and the original song file are correctly entered in const11.h!

 3. create the index header file (hugidx11.h, as defined in const11.h).
    you can use the tools index.exe and index_h.exe to accomplish this.
    first run " index hugi.dat hugi11.idx ", then
              " index_h hugi11.idx hugidx11.h " in order to create the
    index header file.

 4. compile the whole thing by typing " c c ". the parameter c triggers
    compilation of all data files (*.asm). without it, just the actual
    code is re-compiled.

 you see: it was very complicated to get an article into hugi: after
 formatting and including it into the data file, you had to re-convert the
 data file, enter the correct size in the config header, generate the new
 index and re-compile the whole thing. it's much easier, "editor-friendlier"
 now with chris dragan's engine (but the same applies to street raider's
 engine, too) - you just insert the formatted article in a data container
 file and include a link to it, that's it.

 about the format of hugi.dat (the article data file): i guess it's
 pretty obvious. between two characters with the ascii code 254, the internal
 name of the subsequent article is written. it's exactly this information
 that is evaluated by index.exe and stored in the index file - so in theory,
 you could replace these article names by whitespace or any sort of hidden
 messages. the character with the ascii code 248 toggles command mode.
 if a number between 0 and 5 follows, the according text-color (defined in
 hugi.cpp) is selected. it applies to the rest of the text-line, unless the
 color is changed again in the same line. 5 stands for the default color.
 the three other commands are ASC, LNK and LKE: ASC outputs the character
 with the subsequent ascii code, which must be in decimal format and have
 exactly 3 digits (e.g. " øASC248 " would display the command-mode-toggle
 symbol). LNK creates an internal link - e.g. if " øLNKblablaøRead me!øLKE "
 stands in the text-data file, you'll be able to enter the article named
 "blabla" by clicking "Read me!". it's obvious that LKE's purpose is to
 close a link-area. this is its own purpose, at least its own valid purpose.

 the structure of the background picture is pretty critical because, as
 you know if you've seen hugi11 recently, hugi11 uses crossfading - in a
 palettized 256-color mode. there are 6 possible text colors. if you consider
 transparency (i.e. the missing of a text-pixel at a position), you have
 7 possible states of the top layer of the text-window. now if the bottom
 layer (background behind the text) featured 256 colors, crossfading would
 be impossible in this screen mode. therefore it is restricted to 16 colors.
 7*16=112 - the remaining 144 palette entries can be used for the other
 parts of the screen. find out which palette entry is for which area by
 loading hugi11.pcx in a palette-orientated graphic program.

 in fact i don't remember what made me a limit of 16 colors, as a more
 comfortable limit (e.g. 20 colors, which would leave 116 colors for the
 rest of the screen) would have been possible, too. but i guess i chose
 16 because it's better to have little contrast behind the text so that
 it will be well-readable. moreover, in this way the graphician had more
 palette entries to use for the buttons and the hugi logo.

 okay. all right. i think that's about it.

 for the developer/editor, this hardcoded approach of a mag-interface is a
 pain in the ass to work with. but who cares? the readers didn't notice it,
 for them it was all right. :)

 the hugi11 interface is still pretty nice in my opinion. it was absolutely
 okay in early 1998 considering the standard of diskmags. and i'm pleased if
 these codes help you learn something.

 *warning*: you'd better not try to execute this code under windows. it may
 crash your system! always boot in msdos-mode!

 ahh, and i hope you unpacked this file with the parameter -d as important
 data is in subdirectories... if you use pkunzip. okay, most people use
 winzip instead anyway...

                                                       adok^hugi, 21.09.2000

 contact: hugi@netway.at - www.hugi.de
