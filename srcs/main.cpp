#include "application.hpp"
#include <signal.h>

Application application;

/// logging
Tintin_reporter reporter;

void sig_handler(int sig)
{
#ifdef MATTDEAMONCLIENT
  if (sig == SIGINT) application.sigint();
  else if (sig == SIGWINCH) application.sigwinch();
#else
  reporter.signal(strsignal(sig));
#endif
  if (sig == SIGSEGV || sig == SIGTERM || sig == SIGQUIT) application.stop();
}

int main(int ac, char **av) {
    for (int n = 0; n < SIGRTMAX; n++) signal(n, sig_handler);
    application.initWithArg(ac, av);
    application.start();
    return (0);
}
