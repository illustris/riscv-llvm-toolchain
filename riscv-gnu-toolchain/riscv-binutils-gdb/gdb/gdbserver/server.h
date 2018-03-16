/* Common definitions for remote server for GDB.
   Copyright (C) 1993-2017 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef SERVER_H
#define SERVER_H

#include "common-defs.h"

gdb_static_assert (sizeof (CORE_ADDR) >= sizeof (void *));

#ifdef __MINGW32CE__
#include "wincecompat.h"
#endif

#include "version.h"

#if !HAVE_DECL_STRERROR
#ifndef strerror
extern char *strerror (int);	/* X3.159-1989  4.11.6.2 */
#endif
#endif

#if !HAVE_DECL_PERROR
#ifndef perror
extern void perror (const char *);
#endif
#endif

#if !HAVE_DECL_VASPRINTF
extern int vasprintf(char **strp, const char *fmt, va_list ap);
#endif
#if !HAVE_DECL_VSNPRINTF
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

#ifdef IN_PROCESS_AGENT
#  define PROG "ipa"
#else
#  define PROG "gdbserver"
#endif

#include "buffer.h"
#include "xml-utils.h"
#include "regcache.h"
#include "gdb_signals.h"
#include "target.h"
#include "mem-break.h"
#include "gdbthread.h"
#include "inferiors.h"
#include "environ.h"

/* Target-specific functions */

void initialize_low ();

/* Public variables in server.c */

extern ptid_t cont_thread;
extern ptid_t general_thread;

extern int server_waiting;
extern int pass_signals[];
extern int program_signals[];
extern int program_signals_p;

extern int disable_packet_vCont;
extern int disable_packet_Tthread;
extern int disable_packet_qC;
extern int disable_packet_qfThreadInfo;

extern char *own_buf;

extern int run_once;
extern int multi_process;
extern int report_fork_events;
extern int report_vfork_events;
extern int report_exec_events;
extern int report_thread_events;
extern int non_stop;

/* True if the "swbreak+" feature is active.  In that case, GDB wants
   us to report whether a trap is explained by a software breakpoint
   and for the server to handle PC adjustment if necessary on this
   target.  Only enabled if the target supports it.  */
extern int swbreak_feature;

/* True if the "hwbreak+" feature is active.  In that case, GDB wants
   us to report whether a trap is explained by a hardware breakpoint.
   Only enabled if the target supports it.  */
extern int hwbreak_feature;

extern int disable_randomization;

#if USE_WIN32API
#include <winsock2.h>
typedef SOCKET gdb_fildes_t;
#else
typedef int gdb_fildes_t;
#endif

#include "event-loop.h"

/* Functions from server.c.  */
extern void handle_v_requests (char *own_buf, int packet_len,
			       int *new_packet_len);
extern int handle_serial_event (int err, gdb_client_data client_data);
extern int handle_target_event (int err, gdb_client_data client_data);

/* Get rid of the currently pending stop replies that match PTID.  */
extern void discard_queued_stop_replies (ptid_t ptid);

/* Returns true if there's a pending stop reply that matches PTID in
   the vStopped notifications queue.  */
extern int in_queued_stop_replies (ptid_t ptid);

#include "remote-utils.h"

#include "utils.h"
#include "debug.h"
#include "gdb_vecs.h"

/* Maximum number of bytes to read/write at once.  The value here
   is chosen to fill up a packet (the headers account for the 32).  */
#define MAXBUFBYTES(N) (((N)-32)/2)

/* Buffer sizes for transferring memory, registers, etc.   Set to a constant
   value to accomodate multiple register formats.  This value must be at least
   as large as the largest register set supported by gdbserver.  */
#define PBUFSIZ 16384

/* Definition for an unknown syscall, used basically in error-cases.  */
#define UNKNOWN_SYSCALL (-1)

/* Definition for any syscall, used for unfiltered syscall reporting.  */
#define ANY_SYSCALL (-2)

/* After fork_inferior has been called, we need to adjust a few
   signals and call startup_inferior to start the inferior and consume
   its first events.  This is done here.  PID is the pid of the new
   inferior and PROGRAM is its name.  */
extern void post_fork_inferior (int pid, const char *program);

/* Get the gdb_environ being used in the current session.  */
extern gdb_environ *get_environ ();

extern target_waitstatus last_status;
extern ptid_t last_ptid;
extern unsigned long signal_pid;

#endif /* SERVER_H */