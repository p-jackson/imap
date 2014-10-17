#include "command_printer.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <Windows.h>

using std::vector;
using std::string;

string toUnixEnding(std::string line) {
  boost::replace_last(line, "\r\n", "\n");
  return line;
}

bool isLogin(const vector<string>& tokens) {
  return !tokens.empty() && tokens[0] == "LOGIN";
}

CommandPrinter::CommandPrinter(std::ostream& out) : m_out{ out } {
}

string CommandPrinter::join(vector<string> tokens) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);

  const auto line = m_builder.join(tokens);

  if (!isLogin(tokens))
    m_out << toUnixEnding(line);
  else {
    if (tokens.size() > 2)
      tokens[2] = "********";
    const auto prefix = getPrefix(line);
    m_out << prefix << " " << boost::join(tokens, " ") << "\n";
  }

  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);

  return line;
}

vector<string> CommandPrinter::split(string line) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
  m_out << toUnixEnding(line) << "\n";
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);

  return m_builder.split(std::move(line));
}

string CommandPrinter::getPrefix(string line) {
  return m_builder.getPrefix(move(line));
}

vector<string> CommandPrinter::removePrefix(vector<string> tokens) {
  return m_builder.removePrefix(move(tokens));
}
