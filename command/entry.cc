#include "command.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <imap/connection.h>
#include <iostream>

using namespace std;

struct Unknown {
  static string name() { return "has_no_runnable_name"; }
  bool run(istream&, ostream& out, imap::Connection&, vector<string> args) {
    out << "Unknown command: \"" << args[0] << "\". Try \"help\".\n";
    return false;
  }
};

vector<Command> g_commands;

void registerCommand(Command c) {
  g_commands.push_back(move(c));
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

string getFilePart(const string& path) {
  vector<string> parts;
  boost::split(parts, path, boost::is_any_of("/\\"));
  return parts.back();
}

imap::Connection initConnection(int argc, char** argv) {
  if (argc < 3) {
    cerr << "Host and port weren't specified.\n";
    cerr << "  Usage: " << getFilePart(argv[0]) << " [host] [port]\n";
    return {};
  }

  cout << "Connecting...\n";
  try {
    auto port = boost::lexical_cast<unsigned short>(argv[2]);
    auto connection = imap::Connection{ argv[1], port };
    cout << "Connected.\n";
    return connection;
  }
  catch (exception& e) {
    cerr << "ERROR: " << e.what() << "\n";
    return {};
  }
}

int main(int argc, char** argv) {
  initCommands();

  auto connection = initConnection(argc, argv);
  if (!connection.isOpen())
    return 1;

  string line;
  bool quit = false;

  // Get the first response from the server.
  findCommand("read").run(cin, cout, connection, {});

  while (!quit && getline(cin, line)) {
    vector<string> args;
    boost::split(args, line, boost::is_space(), boost::token_compress_on);

    auto command = findCommand(args[0]);

    try {
      quit = command.run(cin, cout, connection, move(args));
    }
    catch (exception& e) {
      cerr << "ERROR: " << e.what() << "\n";
    }
  }
}
