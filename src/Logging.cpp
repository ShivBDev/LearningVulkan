#include "Logging.h"

std::string Logging::Log    (std::string const & msg) { Log(msg.c_str()); return msg; }
std::string Logging::Log    (char const * const msg)  { write(msg); return msg; }

std::string Logging::Warn   (std::string const & msg) { Warn(msg.c_str()); return msg; }
std::string Logging::Warn   (char const * const msg)  { write(msg, yellow); return msg; }

std::string Logging::Error  (std::string const & msg) { Error(msg.c_str()); return msg; }
std::string Logging::Error  (char const * const msg)  { write(msg, red); return msg; }

std::string Logging::Debug  (std::string const & msg) { Debug(msg.c_str()); return msg; }
std::string Logging::Debug  (char const * const msg)  { write(msg, blue); return msg; }

void Logging::write(const char* msg, const char* color) {
  if(color == blue && !verbose) { return; }
  printf("%s%s%s%s", color, msg, reset, add_new_line ? "\n" : "");
}