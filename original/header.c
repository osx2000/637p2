/****************************************************************************** 
 *  Functions to read and write header    
 *    data for analysis files            
 *                                      
 *  Rob Maher    April 1987            
 *                                    
 *    use:                           
 *           HEADER headptr;        
 *              int fd;            
 *                                
 *        fd = open("file",0);   
 *          --or--                 
 *        fd = creat("file",0644);
 *                               
 *     rdat(fd,&headptr);       
 *     wdat(fd,&headptr);      
 *
 *   Changes:                        
 *	02/19/91 jwb	On NeXT: eliminated calloc() declaration.
 *	08/12/94 jwb	For DEC Alpha: insert byte swapping code.
 *	11/05/95 jwb	Insert code to make sure header is of correct length.
 *	09/24/96 jwb	Header correct length code revised.
 *****************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#define P printf

char *gstring();

/* #ifdef __alpha */						/* jwb 8/12/94 */
  byteswap2(short*);					/* jwb 8/12/94 */
  byteswap4(int*);					/* jwb 8/12/94 */
  int i, *data;						/* jwb 8/12/94 */
/* #endif */

/* Function to read header structure from file */
void rdat(int fd,HEADER *head_data) 			/* jwb 11/05/95 */
     /* fd is the file descriptor */
{
  int nobytes;						/* jwb 11/05/95 */
  
  if((nobytes = lseek(fd, 0L, 2)) < 10*sizeof(int))	/* jwb 09/24/96 */
  {
    P("Analysis file header is invalid. Abort\n");	/* jwb 09/24/96 */
    fflush(stdout);					/* jwb 11/05/95 */
    exit(1);						/* jwb 11/05/95 */
  }
  lseek(fd, 0L, 0);					/* jwb 11/05/95 */
  head_data->performer= gstring(fd);
  head_data->instrument= gstring(fd);
  head_data->date= gstring(fd);
  head_data->pitch= gstring(fd);
  head_data->dyn= gstring(fd);
  head_data->vibra= gstring(fd);
  head_data->part= gstring(fd);
  head_data->type= gstring(fd);
  head_data->comments= gstring(fd);
  head_data->andate= gstring(fd);
  read(fd,&head_data->interpval,sizeof(float));
  read(fd,&head_data->sr,sizeof(float));
  read(fd,&head_data->tl,sizeof(float));
  read(fd,&head_data->smax,sizeof(float));
  read(fd,&head_data->fa,sizeof(float));
  read(fd,&head_data->dt,sizeof(float));
  read(fd,&head_data->fftlen,sizeof(int));
  read(fd,&head_data->nhar,sizeof(int));
  read(fd,&head_data->nchans,sizeof(int));
  if( read(fd,&head_data->npts,sizeof(int)) <=0 )
  {  P("File read error in rdat()\n");
     exit(1);
  }
  /* #ifdef __alpha */						/* jwb 8/12/94 */
  /* swap the int & float bytes after reading them in */
  data = (int*)&(head_data->interpval);			/* jwb 8/12/94 */
  for(i=0;i<10;i++) byteswap4(data+i);			/* jwb 8/12/94 */
  /* #endif */

}  /*  end rdat()  */

/* function to write header structure data to file */
void wdat(fd,head_data) int fd; HEADER *head_data;
{
  /* #ifdef __alpha */                                         /* jwb 8/12/94 */
  /* swap the int & float bytes before writing them out */
  data = (int*)&(head_data->interpval);                 /* jwb 8/12/94 */
  for(i=0;i<10;i++) byteswap4(data+i);                  /* jwb 8/12/94 */
  /* #endif */

  write(fd,head_data->performer,1+strlen(head_data->performer));
  write(fd,head_data->instrument,1+strlen(head_data->instrument));
  write(fd,head_data->date,1+strlen(head_data->date));
  write(fd,head_data->pitch,1+strlen(head_data->pitch));
  write(fd,head_data->dyn,1+strlen(head_data->dyn));
  write(fd,head_data->vibra,1+strlen(head_data->vibra));
  write(fd,head_data->part,1+strlen(head_data->part));
  write(fd,head_data->type,1+strlen(head_data->type));
  write(fd,head_data->comments,1+strlen(head_data->comments));
  write(fd,head_data->andate,1+strlen(head_data->andate));
  write(fd,&head_data->interpval,sizeof(float));
  write(fd,&head_data->sr,sizeof(float));
  write(fd,&head_data->tl,sizeof(float));
  write(fd,&head_data->smax,sizeof(float));
  write(fd,&head_data->fa,sizeof(float));
  write(fd,&head_data->dt,sizeof(float));
  write(fd,&head_data->fftlen,sizeof(int));
  write(fd,&head_data->nhar,sizeof(int));
  write(fd,&head_data->nchans,sizeof(int));
  if( write(fd,&head_data->npts,sizeof(int)) <=0 ){
     P("File write error in wdat()\n");
     exit(1);
    }

  /* #ifdef __alpha */                                          /* jwb 8/12/94 */
  /* swap the int & float bytes back again for internal use */
  for(i=0;i<10;i++) byteswap4(data+i);                  /* jwb 8/12/94 */
  /* #endif */

} /*  end function wdat()  */

/******************************************** 
 ** function to acquire string from a file ** 
 ** -->returns pointer to allocated buffer ** 
 ********************************************/
char *gstring(fd) int fd;
{
   int i; char buf; char *string;

   /** count chars in string **/
    i=0;
    do
    { i++;
      read(fd,&buf,sizeof(char));
    } while(buf != 0);

    /* create string buffer */
    string = (char *)calloc(i,sizeof(char));

    /* seek to head of string and read it in */
    lseek(fd,-1*i,1);
    read(fd,string,i*sizeof(char));
    return(string);
} /* end of function gstring */

/* #ifdef __alpha */
byteswap2(short* dat)
{
  unsigned char *byte, temp;
  byte = (unsigned char *)dat;
  temp = byte[1]; byte[1] = byte[0]; byte[0] = temp;
}

byteswap4(int* dat)
{
  unsigned char *byte, temp;
  byte = (unsigned char *)dat;
  temp = byte[3]; byte[3] = byte[0]; byte[0] = temp;
  temp = byte[2]; byte[2] = byte[1]; byte[1] = temp;
}
/* #endif */

/********************** END OF FILE header.c *********************************/
