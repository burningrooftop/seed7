/********************************************************************/
/*                                                                  */
/*  cmd_win.c     Command functions which call the Windows API.     */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  File: seed7/src/cmd_win.c                                       */
/*  Changes: 2010, 2012 - 2013  Thomas Mertes                       */
/*  Content: Command functions which call the Windows API.          */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"


#ifndef PATH_MAX
#define PATH_MAX 2048
#endif



#if defined OS_STRI_WCHAR && !defined USE_WMAIN
#ifdef DEFINE_COMMAND_LINE_TO_ARGV_W
#define CommandLineToArgvW MyCommandLineToArgvW
/**
 *  Special handling of backslash characters for CommandLineToArgvW.
 *  CommandLineToArgvW reads arguments in two modes. Inside and
 *  outside quotation mode. The following rules apply when a
 *  backslash is encountered:
 *  - 2n backslashes followed by a quotation mark produce
 *    n backslashes and a switch from inside to outside quotation
 *    mode and vice versa. In this case the quotation mark is not
 *    added to the argument.
 *  - (2n) + 1 backslashes followed by a quotation mark produce
 *    n backslashes followed by a quotation mark. In this case the
 *    quotation mark is added to the argument and the quotation mode
 +    is not changed.
 *  - n backslashes not followed by a quotation mark simply produce
 *    n backslashes.
 */
static void processBackslash (const_os_striType *sourcePos, os_striType *destPos)

  {
    memSizeType backslashCount;
    memSizeType count;

  /* processBackslash */
    backslashCount = 1;
    (*sourcePos)++;
    while (**sourcePos == '\\') {
      backslashCount++;
      (*sourcePos)++;
    } /* while */
    if (**sourcePos == '"') {
      /* Backslashes in the result: backslashCount / 2   */
      for (count = backslashCount >> 1; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
      if (backslashCount & 1) {
        /* Add a quotation mark (") to the result.       */
        **destPos = '"';
        (*destPos)++;
        /* Stay in current quotation mode                */
        (*sourcePos)++;
      } else {
        /* Ignore the quotation mark (").                */
        /* Switch from inside to outside quotation mode  */
        /* and vice versa.                               */
      } /* if */
    } else {
      /* N backslashes not followed by a quotation mark  */
      /* simply produce n backslashes.                   */
      for (count = backslashCount; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
    } /* if */
  } /* processBackslash */



/**
 *  Parse commandLine and generate an array of pointers to the arguments.
 *  The parameter w_argc and the returned array of pointers (w_argv)
 *  correspond to the parameters argc and argv of main().
 *  The rules to recognize the first argument (the command) are
 *  different from the rules to recognize the other (normal) arguments.
 *  Arguments can be quoted or unquoted. Normal arguments (all except
 *  the first argument) can consist of quoted and unquoted parts. The
 *  quoted and unquoted parts that are concatenated to form one argument.
 *  To handle quoted and unquoted parts the function works with two
 *  modes: Inside and outside quotation mode.
 *  @param w_argc Address to which the argument count is copied.
 *  @return an array of pointers to the arguments of commandLine.
 */
static os_striType *CommandLineToArgvW (const_os_striType commandLine, int *w_argc)

  {
    size_t command_line_size;
    const_os_striType sourcePos;
    os_striType destPos;
    os_striType destBuffer;
    memSizeType argumentCount;
    os_striType *w_argv;

  /* CommandLineToArgvW */
    command_line_size = os_stri_strlen(commandLine);
    argumentCount = 0;
    w_argv = (os_striType *) malloc(command_line_size * sizeof(os_striType *));
    if (w_argv != NULL) {
      sourcePos = commandLine;
      while (*sourcePos == ' ') {
        sourcePos++;
      } /* while */
      if (*sourcePos == 0) {
        w_argv[0] = NULL;
      } else {
        if (unlikely(!os_stri_alloc(destBuffer, command_line_size))) {
          free(w_argv);
          w_argv = NULL;
        } else {
          /* Set pointer to first char of first argument */
          w_argv[0] = destBuffer;
          argumentCount = 1;
          destPos = destBuffer;
          if (*sourcePos == '"') {
            sourcePos++;
            while (*sourcePos != '"' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } else {
            while (*sourcePos != ' ' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } /* if */
          if (*sourcePos != 0) {
            do {
              sourcePos++;
            } while (*sourcePos == ' ');
            if (*sourcePos != 0) {
              /* Terminate the current argument */
              *destPos = 0;
              destPos++;
              /* Set pointer to first char of next argument */
              w_argv[argumentCount] = destPos;
              argumentCount++;
            } /* if */
          } /* if */
          while (*sourcePos != 0) {
            /* printf("source char: %d\n", *sourcePos); */
            if (*sourcePos == '"') {
              /* Inside quotation mode */
              sourcePos++;
              while (*sourcePos != '"' && *sourcePos != 0) {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } /* while */
              if (*sourcePos == '"') {
                /* Consume the terminating quotation mark */
                sourcePos++;
              } /* if */
            } /* if */
            if (*sourcePos != ' ' && *sourcePos != 0) {
              /* Outside quotation mode */
              do {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } while (*sourcePos != ' ' && *sourcePos != '"' && *sourcePos != 0);
            } /* if */
            if (*sourcePos == ' ') {
              do {
                sourcePos++;
              } while (*sourcePos == ' ');
              if (*sourcePos != 0) {
                /* Terminate the current argument */
                *destPos = 0;
                destPos++;
                /* Set pointer to first char of next argument */
                w_argv[argumentCount] = destPos;
                argumentCount++;
              } /* if */
            } /* if */
          } /* while */
          /* Terminate the last argument */
          *destPos = 0;
          w_argv[argumentCount] = NULL;
        } /* if */
      } /* if */
    } /* if */
    *w_argc = argumentCount;
    return w_argv;
  } /* CommandLineToArgvW */



void freeUtf16Argv (os_striType *w_argv)

  { /* freeUtf16Argv */
    if (w_argv != NULL) {
      os_stri_free(w_argv[0]);
      free(w_argv);
    } /* if */
  } /* freeUtf16Argv */

#else



void freeUtf16Argv (os_striType *w_argv)

  { /* freeUtf16Argv */
    LocalFree(w_argv);
  } /* freeUtf16Argv */
#endif



os_striType *getUtf16Argv (int *w_argc)

  {
    os_striType commandLine;
    os_striType *w_argv;

  /* getUtf16Argv */
    commandLine = GetCommandLineW();
    w_argv = CommandLineToArgvW(commandLine, w_argc);
    return w_argv;
  } /* getUtf16Argv */
#endif



/**
 *  Get the absolute path of the executable of the current process.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the absolute path of the current process.
 */
striType getExecutablePath (const const_striType arg_0)

  {
    os_charType buffer[PATH_MAX];
    errInfoType err_info = OKAY_NO_ERROR;
    striType executablePath;

  /* getExecutablePath */
    if (unlikely(GetModuleFileNameW(NULL, buffer, PATH_MAX) == 0)) {
      raise_error(FILE_ERROR);
      executablePath = NULL;
    } else {
      executablePath = cp_from_os_path(buffer, &err_info);
      if (unlikely(executablePath == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return executablePath;
  } /* getExecutablePath */



#if USE_GET_ENVIRONMENT
os_striType *getEnvironment (void)

  {
    os_striType envBuffer;
    os_striType currPos;
    memSizeType length;
    memSizeType numElems = 0;
    memSizeType currIdx = 0;
    os_striType *env;

  /* getEnvironment */
    logFunction(printf("getEnvironment()"););
    envBuffer = GetEnvironmentStringsW();
    if (envBuffer == NULL) {
      env = NULL;
    } else {
      /* printf("envBuffer: \"" FMT_S_OS "\"\n", envBuffer); */
      currPos = envBuffer;
      do {
        length = os_stri_strlen(currPos);
        currPos = &currPos[length + 1];
        numElems++;
      } while (length != 0);
      /* printf("numElems: " FMT_U_MEM "\n", numElems); */
      env = (os_striType *) malloc(numElems * sizeof(os_striType));
      if (env != NULL) {
        currPos = envBuffer;
        do {
          env[currIdx] = currPos;
          length = os_stri_strlen(currPos);
          currPos = &currPos[length + 1];
          currIdx++;
        } while (length != 0);
        env[currIdx - 1] = NULL;
        /* for (currIdx = 0; env[currIdx] != NULL; currIdx++) {
          printf("env[" FMT_U_MEM "]: \"" FMT_S_OS "\"\n", currIdx, env[currIdx]);
        } */
      } /* if */
      if (env == NULL || env[0] == NULL) {
        if (FreeEnvironmentStringsW(envBuffer) == 0) {
          logError(printf("getEnvironment: FreeEnvironmentStrings() failed.\n"););
        } /* if */
      } /* if */
    } /* if */
    return env;
  }  /* getEnvironment */



void freeEnvironment (os_striType *environment)

  { /* freeEnvironment */
    if (environment != NULL) {
      if (environment[0] != NULL) {
        if (FreeEnvironmentStringsW(environment[0]) == 0) {
          logError(printf("getEnvironment: FreeEnvironmentStrings() failed.\n"););
        } /* if */
      } /* if */
      free(environment);
    } /* if */
  } /* freeEnvironment */
#endif



#ifdef DEFINE_WGETENV
os_striType wgetenv (const const_os_striType name)

  {
    memSizeType value_size;
    os_striType value;

  /* wgetenv */
    value_size = GetEnvironmentVariableW(name, NULL, 0);
    if (value_size == 0) {
      value = NULL;
    } else {
      if (ALLOC_WSTRI(value, value_size - 1)) {
        if (GetEnvironmentVariableW(name, value, value_size) != value_size - 1) {
          FREE_OS_STRI(value);
          value = NULL;
        } /* if */
      } /* if */
    } /* if */
    return value;
  } /* wgetenv */
#endif



#ifdef DEFINE_WSETENV
int wsetenv (const const_os_striType name, const const_os_striType value,
    int overwrite)

  {
    int result;

  /* wsetenv */
    logFunction(printf("wsetenv(\"" FMT_S_OS "\", \"" FMT_S_OS "\", &d)\n",
                       name, value, overwrite););
    result = !SetEnvironmentVariableW(name, value);
    return result;
  } /* wsetenv */
#endif
