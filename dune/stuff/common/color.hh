#ifndef DUNE_COMMON_COLOR_HH
#define DUNE_COMMON_COLOR_HH

#include <sstream>
#include <assert.h>
#include <iostream>

#ifdef __GNUC__
#include <cxxabi.h>
#endif
#include <string>

#include <dune/stuff/common/string.hh>

namespace Dune {
namespace Stuff {
namespace Common {

/**
 * @brief namespace to define color constants that can be
 * used to print colored text in an output stream.
 *
 * * \todo this could go into libdune-stuff
 * @warning Some color codes might be unsupported by your terminal.
 */
namespace Color {
// foreground colors
static const char* const black     = "\033[30m";
static const char* const red       = "\033[31m";
static const char* const green     = "\033[32m";
static const char* const brown     = "\033[33m";
static const char* const blue      = "\033[34m";
static const char* const purple    = "\033[35m";
static const char* const cyan      = "\033[36m";
static const char* const lightgray = "\033[37m";
// light foreground colors
static const char* const darkgray    = "\033[1;30m";
static const char* const lightred    = "\033[1;31m";
static const char* const lightgreen  = "\033[1;32m";
static const char* const yellow      = "\033[1;33m";
static const char* const lightblue   = "\033[1;34m";
static const char* const lightpurple = "\033[1;35m";
static const char* const lightcyan   = "\033[1;36m";
static const char* const white       = "\033[1;37m";

// background colors
static const char* const bblack     = "\033[40m";
static const char* const bred       = "\033[41m";
static const char* const bgreen     = "\033[42m";
static const char* const bbrown     = "\033[43m";
static const char* const bblue      = "\033[44m";
static const char* const bpurple    = "\033[45m";
static const char* const bcyan      = "\033[46m";
static const char* const blightgray = "\033[47m";
// light background colors
static const char* const bdarkgray    = "\033[1;40m";
static const char* const blightred    = "\033[1;41m";
static const char* const blightgreen  = "\033[1;42m";
static const char* const byellow      = "\033[1;43m";
static const char* const blightblue   = "\033[1;44m";
static const char* const blightpurple = "\033[1;45m";
static const char* const blightcyan   = "\033[1;46m";
static const char* const bwhite       = "\033[1;47m";
// modifiers
static const char* const normal       = "\033[0m";
static const char* const bold         = "\033[1m";
static const char* const italic       = "\033[2m";
static const char* const underline    = "\033[4m";
static const char* const blink        = "\033[5m";
static const char* const reverse      = "\033[7m";
static const char* const enditalic    = "\033[22m";
static const char* const endunderline = "\033[24m";
static const char* const endblink     = "\033[25m";
static const char* const endreverse   = "\033[27m";

/**
 * @brief Chooses a color from a 256 color map for a foreground color.
 *
 * @param i The color number between 0 and 255.
 * @returns A string describing a color code.
 */
std::string color(int i)
{
  return "\033[38;5;" + Dune::Stuff::Common::String::convertTo(i) + "m";
}

/**
 * @brief Chooses a color from a 256 color map for a background color.
 *
 * @param i The color number between 0 and 255.
 * @returns A string describing a color code.
 */
std::string backcolor(int i)
{
  return "\033[38;5;" + Dune::Stuff::Common::String::convertTo(i) + "m";
}

// maybe you want to choose your own color
int templateColorChooser(int i)
{
  return i % 256;
}

/**
 * @brief Highlights templates depending on the "template"-level.
 *
 * @param str The string containing the template string
 * @param maxlevel The maximal template-level the string is reduced to.
 * @returns A colored template string.
 */
std::string highlightTemplate(std::string str, int maxlevel = 10000)
{
  if (maxlevel < 0)
    maxlevel     = 0;
  int startindex = 0;
  int level = 0;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '<') {
      level++;
      std::string dummy = "\033[38;5;" + Dune::Stuff::Common::String::convertTo(templateColorChooser(level)) + "m";
      str.insert(i, dummy);
      i += dummy.size();
      if (level == maxlevel)
        startindex = i + 1;
    } else if (str[i] == '>') {
      level--;
      std::string dummy = "\033[38;5;" + Dune::Stuff::Common::String::convertTo(templateColorChooser(level)) + "m";
      str.insert(++i, dummy);
      if (level + 1 == maxlevel) {
        int size = i - startindex - 1;
        str.erase(startindex, size);
        i = startindex + 1;
      }
      i += dummy.size();
    }
  }
  str += "\033[38;5;0m";
  return str;
} // highlightTemplate

/**
 * @brief A simple function highlighting a whole string in a specified foreground color.
 *
 * @param str The string you want to highlight.
 * @param colornr A color number from a 256 color map between 0 and 255.
 * @returns The highlighted string.
 */
std::string highlightString(std::string str, int colornr = 0)
{
  return "\033[38;5;" + Dune::Stuff::Common::String::convertTo(colornr % 256) + "m" + str + "\033[38;5;0m";
}

/**
 * @brief Highlights a substring of another string in a specified color.
 *
 * @param str The string where you want to highlight substrings.
 * @param substr The sub string you want to highlight in str.
 * @param colornr A color number from a 256 color map between 0 and 255.
 * @returns The highlighted string.
 */
std::string highlightSearchString(std::string str, std::string substr, int colornr = 0)
{
  int index = str.find(substr, 0);

  while (index != int(std::string::npos)) {
    std::string dummy  = "\033[38;5;" + Dune::Stuff::Common::String::convertTo(colornr % 256) + "m";
    std::string dummy2 = "\033[38;5;0m";
    str.insert(index, dummy);
    str.insert(index + substr.size() + dummy.size(), dummy2);
    index = str.find(substr, index + dummy.size() + substr.size() + dummy2.size());
  }
  return str;
} // highlightSearchString

} // end of namespace Color

} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // end of DUNE_COMMON_COLOR_HH

/** Copyright (c) 2012, Stefan Girke
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/