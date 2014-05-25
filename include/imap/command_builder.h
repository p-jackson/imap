#ifndef IMAP_COMMAND_BUILDER_H
#define IMAP_COMMAND_BUILDER_H

#include <string>

namespace imap {

  class CommandBuilder {
    unsigned int m_nextNum;
    std::string m_nextPrefix;

  public:
    CommandBuilder();
    std::string makeLine(const std::string& line);
  };

}

#endif