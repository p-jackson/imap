#ifndef IMAP_COMMAND_BUILDER_H
#define IMAP_COMMAND_BUILDER_H

#include <string>
#include <vector>

namespace imap {

  class DefaultCommandBuilder {
    unsigned int m_nextNum;
    std::string m_nextPrefix;

  public:
    DefaultCommandBuilder();
    std::string join(std::vector<std::string> tokens);
    std::vector<std::string> split(std::string line);
    std::string getPrefix(std::string line);
    std::vector<std::string> removePrefix(std::vector<std::string> tokens);
  };

}

#endif