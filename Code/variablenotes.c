// y = y1 + (y2-y1) * (x - x1)/(x2 - x1) 

---LOCAL main:
int   harms             // # of harmonics
int   nbreaks           // # of breaks

---LOCAL envelope:
int   found             // boolean for brkpts check
int   x                 // index of interpolated point x1<=x<=x2
int   x1                // left x-coord of current working segment
int   y1                // left y-coord of current working segment ~ f(x1)
int   x2                // right x-coord of current working segment
int   x2                // right x-coord of current working segment
int   lastbr            // to keep track of previous brkpt *need different strategy*

---GLOBAL:
int   aSize             // # of frames
int   brk               // current break point
int   brc               // breakpoint counter

float actual[]          // a[]  size 'aSize' = frames
float working[]         // w[]  size 'aSize' = frames
int   brkpts[]          // b[]  size 'nbreaks'   
