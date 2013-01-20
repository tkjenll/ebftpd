#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <exception>
#include <sys/ptrace.h>
#include "ftp/listener.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"
#include "util/debug.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/error.hpp"

namespace signals
{

Handler Handler::instance;

void Handler::StartThread()
{
  instance.Start();
}

void Handler::StopThread()
{
  instance.Stop();
}

void Handler::Run()
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGTERM);
  
  int signo;
  while (true)
  {
    sigwait(&mask, &signo);
    switch (signo)
    {
      case SIGHUP   :
      {
        try
        {
          LoadConfig();
        }
        catch (const cfg::ConfigError& e)
        {
          logs::error << "Failed to load config: " + e.Message() << logs::endl;
        }
        try
        {
          text::Factory::Initalize();
        }
        catch (const text::TemplateError& e)
        {
          logs::error << "Templates failed to initalise: " << e.Message() << logs::endl;
        }
        break;
      }
      case SIGTERM  :
      case SIGQUIT  :
      case SIGINT   :
      {
        logs::debug << "Server interrupted!" << logs::endl;
        ftp::Listener::SetShutdown();
        return;
      }
    }
  }
}

void CrashHandler(int signo)
{
  std::stringstream ss;
  ss << "Critical error signal " << signo << " received, dumping backtrace: " << std::endl;

  util::debug::DumpBacktrace(ss, 2);
  
  std::string line; 
  while (std::getline(ss, line))
  {
    logs::error << line << logs::endl;
  }
  
  _exit(-1);
}

void TerminateHandler()
{
  static bool rethrown = false;
  std::stringstream ss;
  try
  {
    if (!rethrown++) throw;
  }
  catch (const std::exception& e)
  {
    ss << "Unhandled ";
    char exceptionType[1024];
    if (util::debug::Demangle(typeid(e).name(), exceptionType, sizeof(exceptionType)))
      ss << exceptionType;
    else
      ss << typeid(e).name();
    ss << " (" << e.what() << ") exception, dumping backtrace: " << std::endl;
  }
  catch (...)
  {
    ss << "Unhandled exception, dumping backtrace: " << std::endl;
  }
  
  

  util::debug::DumpBacktrace(ss, 2);
  
  std::string line; 
  while (std::getline(ss, line))
  {
    logs::error << line << logs::endl;
  }
  
  _exit(-1);
}

util::Error Initialise()
{
  std::set_terminate(TerminateHandler);

  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGABRT);
  
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = 0;
  sa.sa_handler = CrashHandler;
  if (sigaction(SIGSEGV, &sa, nullptr) < 0)
    return util::Error::Failure(errno);
  
  // allow interruption inside gdb
  if (ptrace(PTRACE_TRACEME, 0, NULL, 0) < 0 && errno == EPERM)
    sigdelset(&set, SIGINT);

  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) < 0)
    return util::Error::Failure(errno);  

  return util::Error::Success();
}

} /* signals namespace */
