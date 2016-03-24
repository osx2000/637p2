/******************************************************************************
		anread.c
	Routine to read in analysis file data into cmag and dfr arrays.
	Also computes rms (into cmag), br, and dfr arrays.

	Programmer:  James Beauchamp, 1987
	All Rights Reserved

Changes:
  	09/23/87  jwb	Modify for 'simple' rather than 'compressed' file 
			storage.
	06/27/92  jwb	Insert fabs() function for BR calculation.
	10/03/92  jwb	anwrite(): Insert code to guard against overwriting 
			existing file.
	08/26/93  jwb	anread(): If nhar eq. 0, don't compute rms, dfr.wt.av.; 
			instead, read in stored values.
			anwrite(): If nhar eq. 0, write rms and dfr.wt.av to 
			file.
	12/30/93  jwb	anwrite(): Accept default of nhar harmonics to write to
			file.	
	01/06-7/94 jwb	anread(), anwrite(): Install 'compact' form of data 
			using short ints for file storage.
	02/24/94  jwb   anread(), anwrite(): Modify to scale amplitudes 
			according the max. value of cmag array members.
	08/19/94  jwb	Port to Dec Alpha requires byte swap operations
			in both anread() and anwrite().
	10/12/94  jwb	Declare globals as static.
	10/24/94  jwb 	Change fabs to abs.
	11/11/94  jwb	Fix bug introduced on 8/19/94.
	01/04/95  jwb	anwrite(): Save npts and tl in header.
	04/19/95  jwb	anwrite(): Fix bug wrt Alpha writing of phase array.
	05/19/95  jwb	anwrite(): Exclude the possibility of negative cmag.
	11/05/95  jwb	anwrite(): Restrict nharw to less than sr/(2*fa).
	11/17/95  jwb	anread(): No. of harmonics question goes to stderr
	05/06/96  jwb	anread(): Add nhreq arg to function. if nhreq == 1 
			the program requests nhar from the user. Otherwise,
			it just takes nhar = header.nhar.
	09/24/96 jwb	anread(): Additional header listing print out.
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // open function
#include <unistd.h> // close function
#include "monan.h"
#define USH_MAX 65535.					    /* jwb 02/24/94 */
#define SH_MAX 32768.					    /* jwb 02/24/94 */
static char buffer[80];					    /* jwb 10/12/94 */
int anread(char *filnam, int nhreq)			    /* jwb 05/06/96 */
{
  int fnlen,fnbeg,filno,i,nbpb,nbph, k,k1,k1inc, narg;
  int unit, type;					    /* jwb 04/23/94 */
  int j, nhar2, fftlen, nitems;                             /* jwb 05/06/96 */
  unsigned short int *tempbufs;			            /* jwb 01/06/94 */
  float *tempbuf, cm, df, dfscale;			    /* jwb 01/06/94 */
  double sum1, sum2, sum3, sum4, sum5;

  if((unit = open(filnam,0)) == -1)  return(1);  /* file not available */

  rdat(unit,&header);

  P("\nRead file %s \nDate Recorded: %s\n",filnam,header.date);
  fnlen = strlen(filnam); fnbeg = fnlen - 10; filno = -1;
							    /* jwb 09/24/96: */
  P("Instrument: %s, Performer: %s\n", header.instrument, header.performer);
  P("Comments: \n%s\n",header.comments);
  dt = header.dt; tl = header.tl; fa = header.fa;
  P("Pitch is %s, dynamic is %s\n", header.pitch, header.dyn); /*jwb 12/29/93*/
  P("Base analysis frequency = %.2f Hz\nTime between frames = %f sec\n"
	  ,fa, dt);
  P("Original sample rate = %f\n", header.sr);
  P("Duration of sound is %5.3f\n",tl);
  P("Magfr format is type %s\n",header.type);
  P("Number of analysis frames is %d\n",header.npts);
  P("Number of harmonics per frame = %d\n", header.nhar);
  npts = header.npts;
pt:
  if(nhreq)						    /* jwb 05/06/96 */
  {
    
  //   fprintf(stderr, 					    
  //     "How many harmonics to read (default: %d)? ",	    
		// 			header.nhar);	    
  //   fgets(buffer, sizeof(buffer), stdin);
  //   nitems = sscanf(buffer, "%d", &nhar);
    
  //   if(nitems < 0) nhar = header.nhar; 		    
	 //  else if(nitems == 0) 
  //   { fprintf(stderr,"input not recognized\n"); goto pt;}
  //   else if(nitems < header.nhar) { nhar = nitems; }
  //   else { nhar = header.nhar; }

  //   if(nhar > header.nhar) 
  //   {
  //     fprintf(stderr,"only %d harmonics available\n", header.nhar);
  //     goto pt;
  //   }
  //   else if(nhar < 0)					    /* jwb 05/06/96 */
  //   {
  //     fprintf(stderr,
  //         "No. of harmonics must be non-negative\n");       /* jwb 05/06/96 */
  //     goto pt;						    /* jwb 05/06/96 */
  //   }
  //   P("Use nhar = %d\n", nhar);
  // }
  }
  nhar = header.nhar;

  nhar1 = nhar + 1; /* note: nhar may be less than header.nhar */
  nhar2 = 2*header.nhar;				    /* jwb 11/11/94 */
  fftlen = header.fftlen;                             	    /* jwb 08/19/94 */

  /*   allocate space for cmag, br, dfr, time, and tempbuf arrays  */
  cmag = (float *) calloc(npts*nhar1,sizeof(float));
  dfr = (float *) calloc(npts*nhar1,sizeof(float));
  br  = (float *) calloc(npts+1,sizeof(float));
  time = (float *) calloc(npts+1,sizeof(float));

  if(!strcmp(header.type,"full")) 	  type = 'f';
  if(!strcmp(header.type,"compressed")) type = 'c';
  if(!strcmp(header.type,"simple")) 	  type = 's';
  if(!strcmp(header.type,"compact")) 	  type = 'p';	    /* jwb 01/06/94 */

  if(type=='f')  /* "full" */
  {
    /* read in and save initial phase data    */
     phase = (float *) calloc(fftlen/2,sizeof(float));      /* jwb 8/19/94 */
     nbph = fftlen*sizeof(float)/2;
     if(read(unit,phase,nbph) != nbph) 
     {P("Cannot read initial phases\n");  return(1);}
     /* #ifdef __alpha */
     for(i=0;i<fftlen/2;i++) byteswap4((int*)(phase+i));    /* jwb 8/19/94 */
     /* #endif */
   /* allocate space to read one analysis frame  */
     nbpb = fftlen*sizeof(float);
     tempbuf = (float *) malloc(nbpb); 
  }
  else if((type=='c')||(type=='s')||(type=='p'))  	    /* jwb 01/06/94 */
		/* "compressed" or "simple" or "compact" data type */
  {
    /* read in and save initial phase data    */
    if(header.nhar >= 1)  /* normal original data */	    /* jwb 08/26/93 */
    {
      phase = (float *) calloc(header.nhar+1,sizeof(float));
      nbph = header.nhar*sizeof(float);
      if(read(unit,phase+1,nbph) != nbph) 
      { P("Cannot read initial phases\n");  return(1); }
      /* #ifdef __alpha */
     for(j=1;j<=nhar;j++) byteswap4((int*)(phase+j));      /* jwb 08/19/94 */
     /* #endif */
   /* allocate space to read one analysis frame (based on original nhar) */
     nbpb = nhar2*sizeof(float);			   /* jwb 08/19/94 */
     tempbuf = (float *) malloc(nbpb);		  	   /* jwb 08/26/93 */
    }
    else   /* header.nhar eq. 0 data */		  	   /* jwb 8/26/93 */
    { nbpb = 2*sizeof(float);
	tempbuf = (float *) malloc(nbpb);	  	   /* jwb 08/26/93 */
    }
  }
  else {P("Data type not recognized.  Cannot process."); return(1);}

  if((type=='f')||(type=='c'))  /*  "full" or "compressed"  */
     ampscale = 1./(fftlen*0.5*.54);
  else if(type=='p') ampscale = header.smax;		  /* jwb 02/24/94 */
  else	ampscale = 1;
  
  if(type=='p')  /* for short int files */ 		  /* jwb 01/07/94 */
  {
    tempbufs = (unsigned short *)tempbuf;		  /* jwb 01/07/94 */
    nbpb /= 2;						  /* jwb 01/07/94 */
    if(fa > 0) dfscale = fa/SH_MAX;			  /* jwb 02/24/94 */
    else if(fa == 0) dfscale = 1./8.;			  /* jwb 01/07/94 */
  }

    /* read in analysis data for npts frames and put in cmag & dfr arrays */
  for(i=0;i<npts;i++)  
  {
    if(read(unit,tempbuf,nbpb) != nbpb)
    { P("Cannot read frame %d\n",i);
      npts = i-2;
      break;
    }
    /* #ifdef __alpha */
    if(type=='p') /* compact */
      for(j=0;j<nhar2;j++) byteswap2((short*)(tempbufs+j));  /* jwb 08/19/94 */
    else /* full, compressed, or simple */
      for(j=0;j<nhar2;j++) byteswap4((int*)(tempbuf+j));     /* jwb 08/17/94 */
    /* #endif */
    if(nhar >= 1)	/* normal data */		     /* jwb 08/26/93 */
    {
      if(type=='f'){k1=4; k1inc = 4;} /* for type "full"  */
	else  {k1=0; k1inc = 2;} /* for type "compressed" or "simple"  */
	sum1 = sum2 = sum3 = sum4 = sum5 = 0.;
	narg = i*nhar1;
	for(k=1;k<=nhar;k++,k1 += k1inc)		     /* jwb 08/26/93 */
	{ 
	  narg++;
	  if(type!='p')					    /* jwb 01/06/94 */
	  {
	    cm = ampscale*tempbuf[k1];			    /* jwb 01/06/94 */ 
	    df = tempbuf[k1+1];				    /* jwb 01/06/94 */
	  }
	  else
	  {
	    cm = ampscale*tempbufs[k1];			    /* jwb 01/06/94 */
	    df = dfscale*k*(tempbufs[k1+1]-SH_MAX);	    /* jwb 02/24/94 */
	  }
	  cmag[narg] = cm;
        dfr[narg]  = df;
 	  sum1 += cm*cm;  sum4 += k*abs(cm);		    /* jwb 10/26/94 */ 
	  sum5 += abs(cm); 				    /* jwb 10/26/94 */
		/* limit dfr to NHARMIN harms: */
	  if(k<=NHARMIN) {sum2 += cm;  sum3 += cm*df/k;}    /* jwb 08/26/93 */
/*	  if((i==0)&&(k>0)) P("k=%d, cmag=%f, dfr=%f\n",
		      			     k,cmag[narg],dfr[narg]);
*/
      } 
	cmag[i*nhar1] = sqrt(sum1);  	  /* store the rms points  */
	if(sum2 > .0001) dfr[i*nhar1] = sum3/sum2;   
      else dfr[i*nhar1] = 0.;    	    /* store the df.wt.ave points  */
	br[i] = (sum4 + BRTHRESH)/(sum5 + BRTHRESH); /*store the br points*/
    } 
    else    /* nhar == 0 */
    { 
	if(type != 'p')					    /* jwb 01/06/94 */
	{
	  cmag[i] = tempbuf[0];		 		    /* jwb 08/26/93 */
	  dfr[i]  = tempbuf[1];				    /* jwb 08/26/93 */
	}	
	else  /* type == 'p' */   			    /* jwb 01/06/94 */
	{
	  cmag[i] = ampscale*tempbufs[0]; 		    /* jwb 02/24/94 */
	  dfr[i]  = dfscale*(tempbufs[1] - SH_MAX);	    /* jwb 01/06/94 */
	}
    }
    time[i] = i*dt;              		/* store the time points   */
  }  /* end for(i=0;i<npts ... loop */
  header.npts = npts;					    /* jwb 08/19/94 */
  free(tempbuf);
  return(0);
}

/******************************************************************************
 *
 *                   anwrite()
 *
 *         Routine to write mag fr analysis format file (.an file)
 *
 *                 J. Beauchamp    4/28/87
 *
 *****************************************************************************/
void anwrite()
{
  int i,j,k, fd, narg, nbytes, nhard, nharw;		/* jwb 11/05/95 */
  unsigned short int temp[2]; 				/* jwb 01/07/94 */
  float dfscale, ampscale, cmax;			/* jwb 02/24/94 */
  char outfile[40], resp[10];				/* jwb 10/03/92 */
  P("Write (possibly modified) data as a 'compact' format mag fr file\n\n");
  top:
  P("Give name for output file--"); scanf("%s%*c",outfile);
  if((fd = open(outfile,0)) != -1)			/* jwb 10/03/92 */
      { P("This file already exists! Do you wish to overwrite it?(y/n) "); 
        scanf("%s%*c",resp);				/* jwb 10/03/92 */
        if(resp[0] == 'n') goto top;			/* jwb 10/03/92 */
        close(fd);					/* jwb 10/03/92 */
      }							/* jwb 10/03/92 */
  fd = creat(outfile,0644);
  pt1:
  nhard = min((int)(header.sr/(2.*fa)),nhar);		/* jwb 11/05/95 */
  P("How many harmonics to write? (%d max, default): ",
		nhard);					/* jwb 11/05/95 */
  fgets(buffer, sizeof(buffer), stdin);			/* jwb 12/30/93 */
  if(sscanf(buffer, "%d", &nharw) != 1) nharw=nhard;	/* jwb 12/30/93 */
  if(nharw < 0)						/* jwb 12/30/93 */
  {
    P("non-negative number, please!\n");		/* jwb 12/30/93 */
    goto pt1;						/* jwb 12/30/93 */
  }
  if(nharw > nhard) 					/* jwb 11/05/95 */
  { 
    P("need a lower number!\n"); 			/* jwb 8/26/93 */
    goto pt1;						/* jwb 8/26/93 */
  }		
  nbytes = 2*sizeof(short);				/* jwb 01/07/94 */
  if(fa==0) dfscale = 8.;				/* jwb 01/07/94 */
  else      dfscale = SH_MAX/fa;			/* jwb 01/07/94 */
/*   find amplitude scale factor */
  cmax = 0.;						/* jwb 02/24/94 */
  { 
    for(i=0;i<npts;i++)					/* jwb 02/24/94 */
    {
      if(nharw > 0)					/* jwb 02/24/94 */
      {
        for(k=1;k<=nharw;k++)				/* jwb 02/24/94 */
          cmax = max(cmax,cmag[k + i*nhar1]);		/* jwb 02/24/94 */
      }
      else	/* nhar == 0 */				/* jwb 02/24/94 */
	cmax = max(cmax,cmag[i*nhar1]); /* rms case */	/* jwb 02/24/94 */
    }
  }
  ampscale    = USH_MAX/cmax;				/* jwb 02/24/94 */
  header.smax = 1./ampscale;				/* jwb 02/24/94 */
    
  if(nharw > 0) P("Write compact format with %d harmonics\n", nharw);	/* jwb 12/30/93 */
  if(nharw == 0) P("Write compact format with only rms and dfr.wt.ave data\n");/*12/30/93*/
/*  write header  */
  header.nhar = nharw;  header.type = "compact";
  header.npts = npts; header.tl = tl;			/* jwb 01/04/95 */
  wdat(fd,&header);
  /* write initial phases */
  if(nharw >= 1) 
  {
    /* #ifdef __alpha */
    for(j=1;j<=nharw;j++) byteswap4((int*)(phase+j));	    /* jwb 4/19/95 */
    /* #endif */
   write(fd,phase + 1,nharw*sizeof(float)); 	            /* jwb 8/26/93 */
  }
/* write mag, dfr values for nharw harmonics and npts frames */
  for(i=0;i<npts;i++)
  {
    if(nharw > 0)					/* jwb 08/26/93 */
    {
      for(k=1;k<=nharw;k++)
      {   narg = k + i*nhar1;	/* nhar1 is a separator; do not modify */
     	temp[0] = ampscale*max(cmag[narg],0.);		/* jwb 05/19/95 */  
	temp[1] = dfscale*dfr[narg]/k + SH_MAX;		/* jwb 01/07/94 */
	/* #ifdef __alpha */
        byteswap2((short*)temp); byteswap2((short*)temp+1);  /* jwb 8/19/94 */
	/* #endif */
       	write(fd,temp,nbytes);
  /*	if(i==0) P("k=%d, cmag=%f, dfr=%f\n",k,cmag[narg],dfr[narg]);  */
      }
    }
    else     /* write only rms, dfr.wt.ave to file */	/* jwb 08/26/93 */
    { 
      temp[0] = ampscale*cmag[i*nhar1];			/* jwb 02/24/94 */ 
      temp[1] = dfscale*dfr[i*nhar1]; 			/* jwb 01/07/94 */
      /* #ifdef __alpha */
      byteswap2((short*)temp); byteswap2((short*)temp+1);  /* jwb 8/19/94 */
      /* #endif */
      write(fd,temp,nbytes);
    }
  }
  ;
  P("Write file %s\n",outfile);
  close(fd);
}
