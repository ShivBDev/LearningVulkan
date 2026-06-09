#include "Logging.hpp"

std::string Logging::Log    (std::string const & msg) { return Log(msg.c_str()); }
std::string Logging::Log    (char const * const msg)  { write(msg); return msg; }

std::string Logging::Warn   (std::string const & msg) { return Warn(msg.c_str()); }
std::string Logging::Warn   (char const * const msg)  { write(msg, yellow); return msg; }

std::string Logging::Error  (std::string const & msg) { return Error(msg.c_str()); }
std::string Logging::Error  (char const * const msg)  { write(msg, red); return msg; }

std::string Logging::Debug  (std::string const & msg) { return Debug(msg.c_str()); }
std::string Logging::Debug  (char const * const msg)  { write(msg, blue); return msg; }

void Logging::write(const char* msg, const char* color) {
  if(color == blue && !verbose) { return; }
  printf("%s%s%s%s", color, msg, reset, add_new_line ? "\n" : "");
}