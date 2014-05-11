#include "command.h"

#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace std;

struct Unknown {
  static string name() { return "has_no_runnable_name"; }
  bool run(istream&, ostream& out, vector<string> args) {
    out << "Unknown command: \"" << args[0] << "\". Try \"help\".\n";
    return false;
  }
};

std::vector<Command> g_commands;

void registerCommand(Command c) {
  g_commands.push_back(std::move(c));
}

Command findCommand(const string& name) {
  auto command = find_if(begin(g_commands), end(g_commands), [&](const Command& command) {
    return command.matches(name);
  });

  if (command == end(g_commands))
    return Command{ Unknown{} };
  else
    return *command;
}

int main() {
  initCommands();

  string line;
  bool quit = false;

  while (!quit && getline(cin, line)) {
    vector<string> args;
    boost::split(args, line, boost::is_space(), boost::token_compress_on);

    auto command = findCommand(args[0]);

    quit = command.run(cin, cout, move(args));
  }
}
