#ifndef logging_hpp
#define logging_hpp
#include <iostream>
#include <string>

class Logging
{
  // returns msg string for easy throwing
  public:
    static std::string Log    (std::string const & msg);
    static std::string Log    (char const * const msg);

    static std::string Warn   (std::string const & msg);
    static std::string Warn   (char const * const msg);

    static std::string Error  (std::string const & msg);
    static std::string Error  (char const * const msg);

    static std::string Debug  (std::string const & msg);
    static std::string Debug  (char const * const msg);
  private:
    static void write(const char* msg, const char* color = reset);
    constexpr static char const
      *red { "\e[31m" }, 
      *yellow { "\e[33m" },
      *blue { "\e[36m" },
      *reset { "\e[0m" };

    constexpr static bool const verbose { true };
    constexpr static bool const add_new_line { true };
};
#endif