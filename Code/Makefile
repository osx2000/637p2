
FILENAME = trump

SAOLFILE = $(FILENAME).saol
SASLFILE = $(FILENAME).sasl
MP4FILE  = $(FILENAME).mp4

INFILE = 
OUTFILE = output.wav

##
## Compiler optimization and debug options. Mac OS X users 
## should replace "gcc" with "cc".
## 

CC = gcc
OPT = -O3
CFLAGS = $(OPT)

SFRONT = ~/source/637p2/sfront/bin/sfront
#SFRONT = ~/CS/637/sfront/bin/sfront
PLAYER = play
CMP = cmp

OUTMODE = -aout $(OUTFILE)
INMODE = 

## for INFILE/OUTFILE selections that need libraries

IOLINK = 

## for INFILE/OUTFILE std selection:  > foo < bar

REDIRECT = 

##
## makes a WAV file
##
 
$(OUTFILE): $(SAOLFILE) $(SASLFILE)
	$(SFRONT) $(OUTMODE) $(INMODE) -orc $(SAOLFILE) -sco $(SASLFILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)

## tests bitstream creation code, by creating an MP4 file, decoding
## it, and comparing audio out with audio created by original ASCII
## files. doesn't work with stdin/stdout.

mp4test	: $(OUTFILE)
	rm -rf $(MP4FILE) sa.c
	mv $(OUTFILE) safe
	$(SFRONT)  -orc $(SAOLFILE) -sco $(SASLFILE) -bitout $(MP4FILE)
	$(SFRONT) $(OUTMODE) $(INMODE) -bit $(MP4FILE)
	$(CC) $(CFLAGS) sa.c -lm $(IOLINK) -o sa
	./sa $(REDIRECT)
	$(CMP) $(OUTFILE) safe

## tests a new sfront against an old one. assume safe is the audio
## created by old sfront. 

compare	: $(OUTFILE)
	$(CMP) $(OUTFILE) safe

safe	: $(OUTFILE)
	cp $(OUTFILE) safe

## for performance testing
##

timing	: 
	/usr/bin/time -p ./sa $(REDIRECT)

##
##
##

## only works on HPUX, for WAV files

play  : $(OUTFILE)
	$(PLAYER) $(OUTFILE)

clean: 
	rm -rf sa.c sa audio a*.wav $(MP4FILE) $(OUTFILE) *.info *~ safe



