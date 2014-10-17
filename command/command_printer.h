#ifndef IMAP_COMMAND_COMMAND_PRINTER_H
#define IMAP_COMMAND_COMMAND_PRINTER_H

#include <imap/default_command_builder.h>

#include <string>
#include <iosfwd>

class CommandPrinter {
  imap::DefaultCommandBuilder m_builder;
  std::ostream& m_out;

public:
  CommandPrinter(std::ostream& out);

  CommandPrinter& operator=(const CommandPrinter&) = delete;

  std::string join(std::vector<std::string> tokens);
  std::vector<std::string> split(std::string line);
  std::string getPrefix(std::string line);
  std::vector<std::string> removePrefix(std::vector<std::string> tokens);
};

#endif