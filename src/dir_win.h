/********************************************************************/
/*                                                                  */
/*  dir_win.h     Directory access using the windows capabilitys.   */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/dir_win.h                                       */
/*  Changes: 1993, 1994, 2007  Thomas Mertes                        */
/*  Content: Directory access using _findfirst and _findnext.       */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in unix.                                                */
/*                                                                  */
/********************************************************************/

#include "windows.h"


struct dirent {
    char *d_name;
  };

typedef struct {
    HANDLE dir_handle;
    WIN32_FIND_DATAA find_record;
    int first_element;
    struct dirent dir_entry;
  } DIR;

struct wdirent {
    wchar_t *d_name;
  };

typedef struct {
    HANDLE dir_handle;
    WIN32_FIND_DATAW find_record;
    int first_element;
    struct wdirent dir_entry;
  } WDIR;


#ifdef ANSI_C

DIR *opendir (char *);
struct dirent *readdir (DIR *);
int closedir (DIR *);
WDIR *wopendir (wchar_t *);
struct wdirent *wreaddir (WDIR *);
int wclosedir (WDIR *);

#else

DIR *opendir ();
struct dirent *readdir ();
int closedir ();
WDIR *wopendir ();
struct wdirent *wreaddir ();
int wclosedir ();

#endif
