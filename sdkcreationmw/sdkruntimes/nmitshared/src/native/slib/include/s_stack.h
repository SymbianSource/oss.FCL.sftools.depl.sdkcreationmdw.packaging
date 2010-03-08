/*
 * $Id: s_stack.h,v 1.7 2003/01/02 22:01:51 slava Exp $
 *
 * Copyright (C) 2000-2003 by Oleg Levin & Slava Monich
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 *   1.Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer. 
 *   2.Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer 
 *     in the documentation and/or other materials provided with the 
 *     distribution. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING 
 * IN ANY WAY OUT OF THE USE OR INABILITY TO USE THIS SOFTWARE, EVEN 
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * The views and conclusions contained in the software and documentation 
 * are those of the authors and should not be interpreted as representing 
 * any official policies, either expressed or implied.
 */

#ifndef _OLEG_STACK_H_
#define _OLEG_STACK_H_

#include "s_def.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* data types */
typedef struct _Stack Stack;
typedef void *        SElement;
typedef const void *  SElementC;

/* stack itself */
struct _Stack {
    int size;                   /* number of objects in stack */
    int alloc;                  /* number of allocated slots */
    SElement * stack;           /* bottom of stack */
};

/* exported functions */

extern Stack *  STACK_Create    P_((int n));
extern Bool     STACK_Init      P_((Stack * s, int n));

extern void     STACK_Delete    P_((Stack * s));
extern void     STACK_Destroy   P_((Stack * s));
extern void     STACK_Clear     P_((Stack * s));

extern Bool     STACK_Push      P_((Stack * s, SElement e));
extern SElement STACK_Pop       P_((Stack * s));
extern SElement STACK_Peek      P_((const Stack* s));

extern Bool     STACK_Contains  P_((const Stack * s, SElementC e));

/* macros */
#define STACK_IsEmpty(s) (STACK_Size(s) == 0)
#define STACK_Size(s)    ((s)->size)

#ifdef __cplusplus
} /* end of extern "C" */
#endif  /* __cplusplus */

#endif /* _OLEG_STACK_H_ */
