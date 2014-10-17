#include <imap/default_command_builder.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace std;

namespace imap {

  DefaultCommandBuilder::DefaultCommandBuilder()
    : m_nextNum{ 1 },
      m_nextPrefix{ "A" }
  {
  }
  
  string DefaultCommandBuilder::join(vector<string> tokens) {
    const auto prefix = m_nextPrefix + to_string(m_nextNum++) + " ";
    return prefix + boost::join(tokens, " ") + "\r\n";
  }

  vector<string> DefaultCommandBuilder::split(string line) {
    vector<string> result;
    boost::split(result, line, boost::is_space());
    return result;
  }

  string DefaultCommandBuilder::getPrefix(std::string line) {
    const auto tokens = split(std::move(line));
    if (tokens.empty())
      return {};
    else
      return tokens[0];
  }

  vector<string> DefaultCommandBuilder::removePrefix(vector<string> tokens) {
    tokens.erase(tokens.begin());
    return std::move(tokens);
  }
  
}