/******************************************************************************    
 *				monan.h
 *
 * 			includes and defines for the monan routines
 *
 *	jwb	04/06/92	change P definition from fprintf to printf
 *	jwb	08/26/93	insert NHARMIN define
 *	jwb	11/01/94	change BRTHRESH to float value
 *	tjm	04/10/96        add convert_3D_to_2D()
 *	jwb	06/03/96	color_on, color global variables
 *****************************************************************************/
#include <math.h>
#include <stdio.h>
#include "g_raph.h"
#include "macro.h"
#include "header.h"
#define P printf
#define BRTHRESH 100.
#define NHARMIN 5					     /* jwb 8/26/93 */

/* function type declarations  */
void plotat(),plotaa(),plotbt(),plotft(),plotfi();
void plabel(double,double);				/* jwb 4/27/94 */


void convert_3D_to_2D(
 	float x,
	float y,
	float z,
	float *x2d,
	float *y2d,
	float yaw,
	float pitch,
	float roll,
	float size_factor,
        int perspective);

/*    global variables:     */
extern HEADER header;
extern int nhar, nhar1, nchans, npts;
extern float *cmag, *dfr, *newmag, *newfr, *finalfr, *phase, *br, *time, tl, dt, fa, smax;
extern double ampscale;
extern char filnam[80];
extern char elabel[40];					/* jwb 12/20/95 */
extern int extra_label;					/* jwb 12/20/95 */
extern int color_on, color; 				/* jwb 06/03/96 */
