%!PS-Adobe-3.0 EPSF-3.0
%%Title: EAGLE Drawing /Volumes/Data/Seafile/Projects & Development/Eagle/Projects/ITEST Eagle/Encoders/mag_encoder_Rev3_single_sided_1.brd
%%Creator: EAGLE
%%Pages: 1
%%BoundingBox: 0 0 62 95
%%EndComments

% Coordinate transfer:

/EU { 254 div 0.072 mul } def
/inch { 72 mul } def

% Linestyle:

1 setlinecap
1 setlinejoin

% Drawing functions:

/l {  % draw a line
   /lw exch def
   /y2 exch def
   /x2 exch def
   /y1 exch def
   /x1 exch def
   newpath
   x1 EU y1 EU moveto
   x2 EU y2 EU lineto
   lw EU setlinewidth
   stroke
   } def

/h {  % draw a hole
   /d  exch def
   /y  exch def
   /x  exch def
   d 0 gt {
     newpath
     x EU y EU d 2 div EU 0 360 arc
     currentgray dup
     1 exch sub setgray
     fill
     setgray
     } if
   } def

/b {  % draw a bar
   /an exch def
   /y2 exch def
   /x2 exch def
   /y1 exch def
   /x1 exch def
   /w2 x2 x1 sub 2 div EU def
   /h2 y2 y1 sub 2 div EU def
   gsave
   x1 x2 add 2 div EU y1 y2 add 2 div EU translate
   an rotate
   newpath
   w2     h2     moveto
   w2 neg h2     lineto
   w2 neg h2 neg lineto
   w2     h2 neg lineto
   closepath
   fill
   grestore
   } def

/c {  % draw a circle
   /lw exch def
   /rd exch def
   /y  exch def
   /x  exch def
   newpath
   lw EU setlinewidth
   x EU y EU rd EU 0 360 arc
   stroke
   } def

/a {  % draw an arc
   /lc exch def
   /ae exch def
   /as exch def
   /lw exch def
   /rd exch def
   /y  exch def
   /x  exch def
   lw rd 2 mul gt {
     /rd rd lw 2 div add 2 div def
     /lw rd 2 mul def
     } if
   currentlinecap currentlinejoin
   lc setlinecap 0 setlinejoin
   newpath
   lw EU setlinewidth
   x EU y EU rd EU as ae arc
   stroke
   setlinejoin setlinecap
   } def

/p {  % draw a pie
   /d exch def
   /y exch def
   /x exch def
   newpath
   x EU y EU d 2 div EU 0 360 arc
   fill
   } def

/edge { 0.20710678119 mul } def

/o {  % draw an octagon
   /an exch def
   /dy exch def
   /dx exch def
   /y  exch def
   /x  exch def
   gsave
   x EU y EU translate
   an dx dy lt { 90 add /dx dy /dy dx def def } if rotate
   newpath
      0 dx 2 div sub EU                    0 dy edge  add EU moveto
      0 dx dy sub 2 div sub dy edge sub EU 0 dy 2 div add EU lineto
      0 dx dy sub 2 div add dy edge add EU 0 dy 2 div add EU lineto
      0 dx 2 div add EU                    0 dy edge  add EU lineto
      0 dx 2 div add EU                    0 dy edge  sub EU lineto
      0 dx dy sub 2 div add dy edge add EU 0 dy 2 div sub EU lineto
      0 dx dy sub 2 div sub dy edge sub EU 0 dy 2 div sub EU lineto
      0 dx 2 div sub EU                    0 dy edge  sub EU lineto
   closepath
   fill
   grestore
   } def

47500 15000 47500 255000 0 l
147500 255000 147500 15000 0 l
147500 15000 47500 15000 0 l
47500 255000 147500 255000 0 l
116935 214995 128935 226995 180.0 b
91935 214995 103935 226995 180.0 b
66935 214995 78935 226995 180.0 b
122935 220995 8000 h
97935 220995 8000 h
72935 220995 8000 h
