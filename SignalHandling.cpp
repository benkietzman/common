/* -*- c++ -*- */
///////////////////////////////////////////
// SignalHandling
// -------------------------------------
// file       : SignalHandling.cpp
// author     : Ben Kietzman
// begin      : 2010-03-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file SignalHandling.cpp
* \brief SignalHandling Functions
*/
// {{{ includes
#include "SignalHandling"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ sethandles()
    void sethandles(void (*function)(int))
    {
      #if defined SIGINT
      signal( SIGINT,     function);  /* interrupt (rubout) */
      #endif
      #if defined SIGQUIT
      signal( SIGQUIT,    function);  /* quit (ASCII FS) */
      #endif
      #if defined SIGILL
      signal( SIGILL,     function);  /* illegal instruction (not reset when caught) */
      #endif
      #if defined SIGTRAP
      signal( SIGTRAP,    function);  /* trace trap (not reset when caught) */
      #endif
      #if defined SIGEMT
      signal( SIGEMT,     function);  /* EMT instruction */
      #endif
      #if defined SIGFPE
      signal( SIGFPE,     function);  /* floating point exception */
      #endif
      #if defined SIGKILL
      signal( SIGKILL,    function);  /* kill (cannot be caught or ignored) */
      #endif
      #if defined SIGBUS
      signal( SIGBUS,     function);  /* bus error */
      #endif
      #if defined SIGSEGV
      signal( SIGSEGV,    function);  /* segmentation violation */
      #endif
      #if defined SIGSYS
      signal( SIGSYS,     function);  /* bad argument to system call */
      #endif
      #if defined SIGPIPE
      signal( SIGPIPE,    function);  /* write on a pipe with no one to read it */
      #endif
      #if defined SIGALRM
      signal( SIGALRM,    function);  /* alarm clock */
      #endif
      #if defined SIGTERM
      signal( SIGTERM,    function);  /* software termination signal from kill */
      #endif
      #if defined SIGCHLD
      signal( SIGCHLD,    function);  /* child status change alias (POSIX) */
      #endif
      #if defined SIGPWR
      signal( SIGPWR,     function);  /* power-fail restart */
      #endif
      #if defined SIGWINCH
      signal( SIGWINCH,   function);  /* window size change */
      #endif
      #if defined SIGURG
      signal( SIGURG,     function);  /* urgent socket condition */
      #endif
      #if defined SIGPOLL
      signal( SIGPOLL,    function);  /* pollable event occured */
      #endif
      #if defined SIGSTOP
      signal( SIGSTOP,    function);  /* stop (cannot be caught or ignored) */
      #endif
      #if defined SIGTSTP
      signal( SIGTSTP,    function);  /* user stop requested from tty */
      #endif
      #if defined SIGCONT
      signal( SIGCONT,    function);  /* stopped process has been continued */
      #endif
      #if defined SIGTTIN
      signal( SIGTTIN,    function);  /* background tty read attempted */
      #endif
      #if defined SIGTTOU
      signal( SIGTTOU,    function);  /* background tty write attempted */
      #endif
      #if defined SIGVTALRM
      signal( SIGVTALRM,  function);  /* virtual timer expired */
      #endif
      #if defined SIGPROF
      signal( SIGPROF,    function);  /* profiling timer expired */
      #endif
      #if defined SIGXCPU
      signal( SIGXCPU,    function);  /* exceeded cpu limit */
      #endif
      #if defined SIGXFSZ
      signal( SIGXFSZ,    function);  /* exceeded file size limit */
      #endif
      #if defined SIGWAITING
      signal( SIGWAITING, function);  /* process's lwps are blocked */
      #endif
      #if defined SIGLWP
      signal( SIGLWP,     function);  /* special signal used by thread library */
      #endif
      #if defined SIGFREEZE
      signal( SIGFREEZE,  function);  /* special signal used by CPR */
      #endif
      #if defined SIGTHAW
      signal( SIGTHAW,    function);  /* special signal used by CPR */
      #endif
      #if defined SIGCANCEL
      signal( SIGCANCEL,  function);  /* thread cancellation signal used by libthread */
      #endif
      #if defined SIGLOST
      signal( SIGLOST,    function);  /* resource lost (eg, record-lock lost) */
      #endif
    }
    // }}}
    // {{{ sigstring()
    string sigstring(string &strSignal, int nSignal)
    {
      switch (nSignal)
      {
        #if defined  SIGINT
        case SIGINT     : strSignal = "SIGINT";     break; /* interrupt (rubout) */
        #endif
        #if defined  SIGQUIT
        case SIGQUIT    : strSignal = "SIGQUIT";    break; /* quit (ASCII FS) */
        #endif
        #if defined  SIGILL
        case SIGILL     : strSignal = "SIGILL";     break; /* illegal instruction (not reset when caught) */
        #endif
        #if defined  SIGTRAP
        case SIGTRAP    : strSignal = "SIGTRAP";    break; /* trace trap (not reset when caught) */
        #endif
        #if defined  SIGABRT
        case SIGABRT    : strSignal = "SIGABRT";    break; /* used by abort, replace SIGIOT in the future */
        #endif
        #if defined  SIGFPE
        case SIGFPE     : strSignal = "SIGFPE";     break; /* floating point exception */
        #endif
        #if defined  SIGKILL
        case SIGKILL    : strSignal = "SIGKILL";    break; /* kill (cannot be caught or ignored) */
        #endif
        #if defined  SIGBUS
        case SIGBUS     : strSignal = "SIGBUS";     break; /* bus error */
        #endif
        #if defined  SIGSEGV
        case SIGSEGV    : strSignal = "SIGSEGV";    break; /* segmentation violation */
        #endif
        #if defined  SIGSYS
        case SIGSYS     : strSignal = "SIGSYS";     break; /* bad argument to system call */
        #endif
        #if defined  SIGPIPE
        case SIGPIPE    : strSignal = "SIGPIPE";    break; /* write on a pipe with no one to read it */
        #endif
        #if defined  SIGALRM
        case SIGALRM    : strSignal = "SIGALRM";    break; /* alarm clock */
        #endif
        #if defined  SIGTERM
        case SIGTERM    : strSignal = "SIGTERM";    break; /* software termination signal from kill */
        #endif
        #if defined  SIGCHLD
        case SIGCHLD    : strSignal = "SIGCHLD";    break; /* child status change alias (POSIX) */
        #endif
        #if defined  SIGPWR
        case SIGPWR     : strSignal = "SIGPWR";     break; /* power-fail restart */
        #endif
        #if defined  SIGWINCH
        case SIGWINCH   : strSignal = "SIGWINCH";   break; /* window size change */
        #endif
        #if defined  SIGURG
        case SIGURG     : strSignal = "SIGURG";     break; /* urgent socket condition */
        #endif
        #if defined  SIGPOLL
        case SIGPOLL    : strSignal = "SIGPOLL";    break; /* pollable event occured */
        #endif
        #if defined  SIGSTOP
        case SIGSTOP    : strSignal = "SIGSTOP";    break; /* stop (cannot be caught or ignored) */
        #endif
        #if defined  SIGTSTP
        case SIGTSTP    : strSignal = "SIGTSTP";    break; /* user stop requested from tty */
        #endif
        #if defined  SIGCONT
        case SIGCONT    : strSignal = "SIGCONT";    break; /* stopped process has been continued */
        #endif
        #if defined  SIGTTIN
        case SIGTTIN    : strSignal = "SIGTTIN";    break; /* background tty read attempted */
        #endif
        #if defined  SIGTTOU
        case SIGTTOU    : strSignal = "SIGTTOU ";   break; /* background tty write attempted */
        #endif
        #if defined  SIGVTALRM
        case SIGVTALRM  : strSignal = "SIGVTALRM";  break; /* virtual timer expired */
        #endif
        #if defined  SIGPROF
        case SIGPROF    : strSignal = "SIGPROF";    break; /* profiling timer expired */
        #endif
        #if defined  SIGXCPU
        case SIGXCPU    : strSignal = "SIGXCPU";    break; /* exceeded cpu limit */
        #endif
        #if defined  SIGXFSZ
        case SIGXFSZ    : strSignal = "SIGXFSZ";    break; /* exceeded file size limit */
        #endif
        #if defined  SIGWAITING
        case SIGWAITING : strSignal = "SIGWAITING"; break; /* process's lwps are blocked */
        #endif
        #if defined  SIGLWP
        case SIGLWP     : strSignal = "SIGLWP";     break; /* special signal used by thread library */
        #endif
        #if defined  SIGFREEZE
        case SIGFREEZE  : strSignal = "SIGFREEZE";  break; /* special signal used by CPR */
        #endif
        #if defined  SIGTHAW
        case SIGTHAW    : strSignal = "SIGTHAW";    break; /* special signal used by CPR */
        #endif
        #if defined  SIGCANCEL
        case SIGCANCEL  : strSignal = "SIGCANCEL";  break; /* thread cancellation signal used by libthread */
        #endif
        #if defined  SIGLOST
        case SIGLOST    : strSignal = "SIGLOST";    break; /* resource lost (eg, record-lock lost) */
        #endif
        default         : strSignal = "UNKNOWN";    break;
      }

      return strSignal;
    }
    // }}}
    // {{{ sigdummy()
    void sigdummy(int nSignal)
    {
      if (nSignal == SIGCHLD)
      {
        struct sigaction action;
        action.sa_handler = SIG_IGN;
        sigaction(nSignal, &action, NULL);
      }
      signal(nSignal, sigdummy);
    }
    // }}}
  }
}
