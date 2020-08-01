/********************************************************************/
/*                                                                  */
/*  itf_rtl.c     Primitive actions for the interface type.         */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/itf_rtl.c                                       */
/*  Changes: 2012  Thomas Mertes                                    */
/*  Content: Primitive actions for the interface type.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "itf_rtl.h"



#ifdef ANSI_C

rtlInterfacetype itfCreate (const rtlInterfacetype interface_from)
#else

rtlInterfacetype itfCreate (interface_from)
rtlInterfacetype interface_from;
#endif

  { /* itfCreate */
    if (interface_from->usage_count != 0) {
      interface_from->usage_count++;
    } /* if */
    return interface_from;
  } /* itfCreate */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype itfToHeap (listtype arguments)
#else

objecttype itfToHeap (arguments)
listtype arguments;
#endif

  {
    objecttype modu_from;
    objecttype result;

  /* itfToHeap */
    modu_from = arg_1(arguments);
    printf("itfToHeap: ");
       trace1(modu_from);
       printf("\n");
    if (CATEGORY_OF_OBJ(modu_from) == INTERFACEOBJECT) {
      result = take_reference(modu_from);
    } else if (CATEGORY_OF_OBJ(modu_from) == STRUCTOBJECT) {
      if (TEMP2_OBJECT(modu_from)) {
        if (!ALLOC_OBJECT(result)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, modu_from, sizeof(objectrecord));
          CLEAR_TEMP2_FLAG(result);
          result->value.structvalue = take_struct(modu_from);
          modu_from->value.structvalue = NULL;
        } /* if */
      } else {
        result = modu_from;
      } /* if */
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    result = bld_interface_temp(result);
    printf("itfToHeap --> ");
       trace1(result);
       printf("\n");
    return result;
  } /* itfToHeap */
#endif



#ifdef ANSI_C

rtlInterfacetype itfToInterface (rtlStructtype *stru_arg)
#else

rtlInterfacetype itfToInterface (stru_arg)
rtlStructtype *stru_arg;
#endif

  {
    rtlInterfacetype result;

  /* itfToInterface */
    result = *stru_arg;
    *stru_arg = NULL;
    return result;
  } /* itfToInterface */
