/* read10xbsys.c 
  Program to read TENEX BSYS archive tapes
  A modification of the read20 program

  last changed: 10 August 1990 
  Dragana Pavlovic, SUMEX, Stanford University

  Copyright (c) 1990, Stanford University
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include "read10xbsys.h"

#define MAXBLKS 16             /* There is no real blocking.  The records can
				 only be read one by one.  However, there
				 is a weird effect when maxblks is 1 so might
				 as well leave it at 16 */

				/* 5 bytes per 36-bit word */
				/* 515 word logical blocks */
#define TAPEBLK 515*5


#define MAXDIR 50       /* allowed chars for the name of the directory*/
#define MAXFIL 100 /* allowed chars for the name of the file */
#define MAXTTL 150     /* allowed chars for total name and directory */
#define MAXNM1 50     /*allowed chars for part of filename before the . */
#define MAXNM2 50    /* allowed chars for part of filename after the . */

/* #define TAPE "/dev/nrmt8"  */      /* Default input tape */
/* #define RTAPE "/dev/rmt8"  */     /* rewind input tape if so specified */
#define TAPE "/dev/nrmt0"        /* Default input tape */
#define RTAPE "/dev/rmt0"       /* rewind input tape if so specified */

int  fdTape;                    /* File handle for Dumper-20 format tape */
char tapebuffer[MAXBLKS][TAPEBLK];
char *tapeblock;	        /* One logical record from tape */
FILE *FP = 0;                   /* Output file handle on extracts */


int  bytesize;          /* Number of bits/byte in current file */
int  numbytes;          /* Number of bytes in current file */

int  logrecord;         /* Current logical record in file */
int Index;              /* index into the tapebuffer */
int  eofs;              /* How many EOF's seen */
int  maxblks;		/* Actual max blocking factor to try */
extern int  errno;
extern void exit();

char pattern[MAXTTL];      /* Filename match pattern */
char filname[MAXFIL];       
char dirname[MAXDIR];       

char wholename[MAXTTL];
char name1[MAXNM1];
char name2[MAXNM2];
char dir[MAXDIR];

int dirret = 0;

main(argc, argv)
int argc;
char *argv[];
{
	char *tape = TAPE;              /* Pathname for tape device/file */
	int rc;

	/* Do switch parsing */
	while (argc>1 && argv[1][0] == '-'){
	  switch(argv[1][1]){
	  case 's':
	    (void)printf("Skipping to the next file\n");
	    (void)system("mt fsf");
	    break;
	  case 'r':
	    tape = RTAPE;
	    break;
	  default:
	    (void)printf("Unknown flag %s\n", argv[1]);
	    exit(1);
	    break;
	  }
	  argc--;  argv++;
	}

	if (argc == 3) {
	  (void)strcpy(dirname,argv[1]);
	  (void)strcpy(filname,argv[2]);
	  (void)sprintf(pattern, "<%s>%s", argv[1], argv[2]);
	  FP = fopen(filname, "w");
	  if (FP == 0) {
	    (void)printf("Couldn't open the file to which to save: %s\n", filname);
	    exit(1);
	  }
	}
	else if (argc == 2) {
	  (void)strcpy(dirname,argv[1]);
	  dirret = 1;
	  (void)sprintf(pattern, "<%s>", argv[1]);
	}
	else {
	  (void)sprintf(pattern," ");
	}
  maxblks = MAXBLKS;
  fdTape = open(tape, 0);         /* Open tape for read */
  if (fdTape == -1) {
    (void)printf("Couldn't open 'tape' file %s\n", tape);
    exit(1);
  }
  
  rc = 0;
  eofs = 0;
  logrecord = 0;

	bytesize = 7; 
	numbytes = TAPEBLK; 

  for ( ; ; )             /* Loop till end of tape */
    {
      /*** Read a record ***/
      
      while (1)
	{
	  if (rc == 0) {
	    rc = read(fdTape, (char *)tapebuffer, (unsigned)TAPEBLK * maxblks);
	    Index =0;
	    if (rc == 0) {
	      if (eofs) {
		(void)printf("End of Tape\n");
		exit(0);
	      }
	      (void)printf("End of Saveset\n");
	      if (dirret == 1) {
		if (FP != 0 && strcmp(wholename, pattern) == 0) {
		  (void)fclose(FP);
		}
	      }
	      else if (FP != 0 && strcmp(wholename, pattern) == 0) {
		(void)fclose(FP);
		exit(0);
	      }
	      eofs++;
	      logrecord = 0;
	      continue;
	    }
	  }

	  eofs = 0;
	  tapeblock = &tapebuffer[Index][0];
	  logrecord++;

/* the directory at the beginning of the tape is the only file 320 long */    

	  if (rc == 320 && logrecord == 1) {
	    (void)printf("Skipping over the directory\n");
	  }
/* each file consists of n+1 record.  The first record has the file info */
	  else if (logrecord == 1) {
	    doHeader(tapeblock);
	  }
	  else if (strcmp(pattern, wholename) == 0) {
	    doBody(tapeblock);
	  }
	  rc = 0;
	  break;
	}
    }
}

/* parse out the info from the first record of the file */
doHeader(block)
char* block;
{
  int x,y;/* max is 5 bytes per word */
  static char buf[(512*5)+1];         /* A page of characters */
  int ct;
  
  switch (bytesize) {
  case 7:      
    /* int maxperblock = 512*5;  */
    break;
  default:     
    return;
  }
  
  ct = TAPEBLK;

  for (x = 0; x < 68; x++) {
    if (bytesize == 7 ) {
      if (x == 24 || x == 40 || x == 67) {
	getstring(block, buf, x, ct);
	buf[ct] = 0;
	switch(x) {
	case 24:        /*the first part of the file name */
	  (void)strcpy(name1, buf);
	  for (y = 0; y < strlen(name1); y++) {
	    if (name1[y] > 64 && name1[y] < 91) { /* only cap. letters */
	      name1[y]= name1[y]+32;
	    }
	  }
	  break;
	case 40:         /* the second part of file name */
	  (void)strcpy(name2, buf);
	  for (y = 0; y < strlen(name2); y++) {
	    if (name2[y] > 64 && name2[y] < 91) { /* only cap. letters */
	      name2[y]= name2[y]+32;
	    }
	  }
	  break;
	case 67:   /* the directory name */
	  (void)strcpy(dir, buf);
	  for (y = 0; y < strlen(dir); y++) {
	    if (dir[y] > 64 && dir[y] < 91) { /* only cap. letters */
	      dir[y]= dir[y]+32;
	    }
	  }
	  break;
	}
      }
    }
  }
  if (dirret == 1) {
    (void)sprintf(wholename, "<%s>", dir);
    if (strlen(name2) != 0) {
      (void)printf("<%s>%s.%s\n", dir, name1, name2);
    }
    else {
      (void)printf("<%s>%s\n", dir, name1);
    }
    if (strcmp(wholename, pattern) == 0) {
      if (strlen(name2) != 0) {
	(void)sprintf(filname, "%s.%s", name1, name2);
      }
      else {
	(void)sprintf(filname, "%s", name1);
      }
      (void)printf("Extracting\n");
      FP = fopen(filname, "w");
      if (FP == 0) {
	(void)printf("Couldn't open the file to which to save: %s\n", filname);
	exit(1);
      }
    }
  }
  else if (strlen(name2) != 0) {
    (void)sprintf(wholename,"<%s>%s.%s", dir, name1, name2);
    (void)printf("%s\n", wholename);
  }
  else {
    (void)sprintf(wholename,"<%s>%s", dir,name1);
    (void)printf("%s\n", wholename);
  }

}


doBody(block)
char* block;
{
  int x;/* max is 5 bytes per word */
  static char buf[(512*5)+1];         /* A page of characters */
  int ct;

  for (x = 0; x < (512*5)+1; x++) {
    buf[x] = '\057';
  }
  switch (bytesize) {
  case 7:      
    /* int maxperblock = 512*5; */
    break;
  default:     
    return;
  }

  
  ct = TAPEBLK;

  if (bytesize == 7 ) {
    getstring(block, buf, 2, ct);
    buf[ct] = 0;
    x = strlen(buf);
/* at the very end of the retrieved text, there is a ctrl-\ ctrl-B garbage*/
    while(buf[x] != '\034' && x !=0) {
      x--;
    }
    /* wipe out the garbage if it is there. so far have not encountered a
       case when it wasn't */
    if (buf[x] == '\034' && buf[x+1] == '\002') {
      buf[x] = '\0';
    }
    (void)fprintf(FP,"%s", buf);
  }
}

/* this stuff is from the read20 program */
int   masks[32] =       /* bitmasks for different length fields */
{              0x00000001, 0x00000003, 0x00000007,
   0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
   0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
   0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
   0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
   0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
   0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
   0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
   0xffffffff
};

getfield(block, wordoff, bitoff, bitlen)
char *block;            /* Tape block record */
int wordoff;            /* 36-bit word offset */
int bitoff;             /* Bit offset of field (from msb) */
int bitlen;             /* Bit length of field */
{
	char *p;                /* Used to point into record */
	int w32;           /* First 32 bits of the 36 bit word */
	int   w4;               /* Last 4 bits of the 36 bit word */
	int  w = 0;            /* the word to return */


				/* First, the "illegal" kludge */
	if (bitoff == 0 && bitlen == 36)
	{       bitoff = 4;
		bitlen = 32;

	}
	if (bitlen > 32) (void)printf("I can't get that large a field!\n");

	/* A PDP-10 (or 20) 36-bit word is laid out with the first 32 bits
	   as the first 4 bytes and the last 4 bits are the low order 4 bits
	   of the 5th byte.   The high 4 bits of that byte should be zero */

	p = block + (5*wordoff);        /* Get ptr to word of interest */
	w32 = *p++ & 0377;                      /* First byte */
	w32 = (w32 << 8) | (*p++ & 0377);       /* 2nd */
	w32 = (w32 << 8) | (*p++ & 0377);       /* 3rd */
	w32 = (w32 << 8) | (*p++ & 0377);       /* 4th */
	w4  = *p;                               /* 5th */
	if (w4 > 017) (void)printf("Not a PDP-10 tape!  w4=%o\n", w4);
	

	/* Get the field right justified in the word "w".
	   There are three cases that I have to handle:
	      [1] field is contained in w32
	      [2] field crosses w32 and w4
	      [3] field is contained in w4
	*/

	if (bitoff+bitlen <= 32)        /* [1] field is contained in w32 */
	{
		w = w32 >> (32 - (bitoff+bitlen));
	}
	else if (bitoff <= 32)          /* [2] field crosses boundary */
	{
		w =  (w32 << (bitoff+bitlen-32))
		   | (w4  >> (36 - (bitoff+bitlen)));
	}
	else                            /* [3] field is contained in w4 */
	{
		w = w4 >> (36 - (bitoff+bitlen));
	}
	w = w & masks[bitlen-1];          /* Trim to proper size */
	return(w);
}


getstring(block, s, wordoff, max)
char *block;            /* Tape block */
char *s;                /* Destination string buffer */
int  wordoff;           /* 36-bit offset from start of tape block */
int  max;               /* Max number of characters to xfer into s */
{
	register int i;         /* Counter for five characters per word */
	int ct = 0;             /* Number of characters loaded so far */
	char *orig = s;         /* Save for debugging */

	while (ct < max)
	{
		for (i = 0; i < 5; i++)
		{
			*s = getfield(block, wordoff, i*7, 7);
			if (*s == 0) return;
			s++;
		}
		wordoff++;
		ct += 5;
	}
}


/* Unix day 0 in Tenex format = DayBaseDelta 0117213 */
