#
# Copyright (c) 2002, Vienna University of Technology, Real-Time Systems Group.
# All rights reserved.
# 
# Contact: sem182@vmars.tuwien.ac.at
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 

#*******************************************************************************
#                                                                              *
#     Author:      G. Leber                                                    *
#                  Technische Universitaet Wien                                *
#                  Institut fuer Technische Informatik E182/1                  *
#                  Treitlstrasse 3                                             *
#                  1040 Wien                                                   *
#                  Tel.: (0222) 58801 / 8176                                   *
#     File:        Makefile						       *
#     Version:     1.9							       *
#     Date:        6/27/94						       *
#                                                                              *
#******************************************************************************/

PREFIX  ?=	/usr
LIB	=	$(DESTDIR)$(PREFIX)/lib
INCLUDE	=	$(DESTDIR)$(PREFIX)/include/
MAN3	=	$(DESTDIR)$(PREFIX)/share/man/man3

OBJS	=	sem182.o msem182.o

#CC	=	c89	-std -check
CC = gcc
#CFLAGS	=	-I.
CFLAGS = -c -I.

#INSTALL	=	installbsd
INSTALL = install

all: libsem182.a

libsem182.a: $(OBJS)
	ar cru libsem182.a $?
	ranlib libsem182.a

.PHONY:
clean:
	rm -f libsem182.a $(OBJS)

install: all
	mkdir -p $(MAN3) $(LIB) $(INCLUDE)
	$(INSTALL) -c -m 644 sem182.h $(INCLUDE)
	$(INSTALL) -c -m 644 msem182.h $(INCLUDE)
	$(INSTALL) -c -m 644 libsem182.a $(LIB)
	$(INSTALL) -c -m 644 sem182.3 $(MAN3)
	-rm -f $(MAN3)/P.3
	-rm -f $(MAN3)/V.3
	-rm -f $(MAN3)/seminit.3
	-rm -f $(MAN3)/semgrab.3
	-rm -f $(MAN3)/semrm.3
	ln -s sem182.3 $(MAN3)/P.3
	ln -s sem182.3 $(MAN3)/V.3
	ln -s sem182.3 $(MAN3)/seminit.3
	ln -s sem182.3 $(MAN3)/semgrab.3
	ln -s sem182.3 $(MAN3)/semrm.3
	$(INSTALL) -c -m 644 msem182.3 $(MAN3)
	-rm -f $(MAN3)/mP.3
	-rm -f $(MAN3)/mV.3
	-rm -f $(MAN3)/mseminit.3
	-rm -f $(MAN3)/msemgrab.3
	-rm -f $(MAN3)/msemrm.3
	ln -s msem182.3 $(MAN3)/mP.3
	ln -s msem182.3 $(MAN3)/mV.3
	ln -s msem182.3 $(MAN3)/mseminit.3
	ln -s msem182.3 $(MAN3)/msemgrab.3
	ln -s msem182.3 $(MAN3)/msemrm.3

uninstall:
	rm -f $(INCLUDE)/sem182.h  $(INCLUDE)/msem182.h $(LIB)/libsem182.a \
		$(MAN3)/sem182.3 $(MAN3)/P.3 $(MAN3)/V.3 $(MAN3)/seminit.3 \
		$(MAN3)/semgrab.3 $(MAN3)/semrm.3 $(MAN3)/msem182.3 \
		$(MAN3)/mP.3 $(MAN3)/mV.3 $(MAN3)/mseminit.3 \
		$(MAN3)/msemgrab.3 $(MAN3)/msemrm.3

