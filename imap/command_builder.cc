#include <imap/command_builder.h>

namespace imap {

  CommandBuilder::CommandBuilder()
    : m_nextNum{ 1 },
      m_nextPrefix{ "A" }
  {
  }
  
  std::string CommandBuilder::makeLine(const std::string& line) {
    std::string prefix = m_nextPrefix + std::to_string(m_nextNum++) + " ";
    return prefix + line + "\r\n";
  }

}