########################################################################
# Makefile                                                             #
#                                                                      #
# For wfs control client 					       #
#                                                                      #
########################################################################
#                                                                      #
#		THE CHARA ARRAY					       #
#     Center for High Angular Resolution Astronomy                     #
# Georgia State University, Atlanta GA 30303-3083, U.S.A.              #
#                                                                      #
#                                                                      #
# Telephone: 1-626-796-5405                                            #
# Fax      : 1-626-796-6717                                            #
# email    : theo@chara.gsu.edu                                        #
# WWW      : http:www.chara.gsu.edu 		                       #
#                                                                      #
# (C) This source code and its associated executable                   #
# program(s) are copyright.                                            #
#                                                                      #
########################################################################
#                                                                      #
# Author : Theo ten Brummelaar                                         #
# Date   : October 2002 					       #
########################################################################

#
# Some definitions
#

CC = gcc
CFLAGS = -g -O2 -Wall -DUSE_GTK `chara-gtk-cflags`
LFLAGS =  -L/usr/X11/lib -lgtkclient -lclient -lcfitsio -lsockman -lchara -lcfitsio -lnsimpleX -lX11 -lrwfits -lnrc -lsla -lz `chara-gtk-libs`
LIBDIR = /usr/local/lib
INCDIR = /usr/local/include
HELPDIR= /usr/local/lib/ui_help
BINDIR=/usr/local/bin
LIBDIR=/usr/local/lib
INCDIR=/usr/local/include
ETCDIR=/ctrscrut/chara/etc
LIBNAME=clockclient

#
# The targets
#

all: wfsgtk

#
# The user interface
#

wfsgtk: wfsgtk.o messages.o andor.o wfs.o display.o
	${CC} -o wfsgtk wfsgtk.o messages.o andor.o wfs.o display.o $(LFLAGS)

wfsgtk.o: wfsgtk.c wfsgtk.h
	${CC} ${CFLAGS} -c wfsgtk.c

messages.o: messages.c wfsgtk.h
	${CC} ${CFLAGS} -c messages.c

andor.o: andor.c wfsgtk.h
	${CC} ${CFLAGS} -c andor.c

wfs.o: wfs.c wfsgtk.h
	${CC} ${CFLAGS} -c wfs.c

display.o: display.c wfsgtk.h
	${CC} ${CFLAGS} -c display.c

#
# Cleanup
#

clean:
	rm -rf core wfsgtk wfsgtk.o messages.o andor.o wfs.o display.o

#
# install
#

install: wfsgtk 
	cp wfsgtk $(BINDIR)
	chmod a+x $(BINDIR)/wfsgtk

uninstall:
	rm -f $(BINDIR)/wfsgtk
