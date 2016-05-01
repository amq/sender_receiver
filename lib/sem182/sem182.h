/**
 * Copyright (c) 2002, Vienna University of Technology, Real-Time Systems Group.
 * All rights reserved.
 *
 * Contact: sem182@vmars.tuwien.ac.at
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 **/


/*******************************************************************************
*                                                                              *
*     Author:      G. Leber                                                    *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8176                                   *
*     File:        sem182.h						       *
*     Version:     2.2							       *
*     Date:        6/27/94						       *
*                                                                              *
*******************************************************************************/

#ifndef SEM182_H
#define SEM182_H

#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif

#include <sys/types.h>

/* sem182.c */
int semrm P_((int semid));
int seminit P_((key_t key, int semperm, int initval));
int semgrab P_((key_t key));
int V P_((int semid));
int P P_((int semid));

#undef P_

#endif /* SEM182_H */
