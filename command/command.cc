#include "command.h"

#include <imap/commands.h>
#include <imap/connection.h>
#include <imap/default_command_builder.h>

#include <boost/algorithm/string/join.hpp>
#include <fstream>
#include <Windows.h>

using namespace std;
using imap::Connection;

struct Quit {
  static string name() { return "quit"; }
  bool run(istream&, ostream&, Connection&, vector<string>) {
    return true;
  }
};

struct Read {
  static string name() { return "read"; }
  bool run(istream&, ostream&, Connection& conn, vector<string>) {
    conn.readLine().get();
    return false;
  }
};

struct Write {
  static string name() { return "write"; }
  bool run(istream& in, ostream&, Connection& conn, vector<string>) {
    string line;
    getline(in, line);
    conn.send(line).get();
    return false;
  }
};

struct Login {
  static string name() { return "login"; }
  bool run(istream&, ostream& out, Connection& conn, vector<string> args) {
    string username, password;
    tie(username, password) = getCredentials(args);

    if (username.empty()) {
      out << "No username given\n";
      return false;
    }
    if (password.empty()) {
      out << "No password given\n";
      return false;
    }

    imap::login(conn, username, password).get();

    return false;
  }

  static pair<string, string> getCredentials(vector<string>& args) {
    if (args.size() >= 3)
      return make_pair(args[1], args[2]);

    auto file = ifstream{ "login.txt" };
    if (!file)
      return {};

    string username, password;
    getline(file, username);
    getline(file, password);
    return make_pair(username, password);
  }
};

struct Help {
  static string name() { return "help"; }
  bool run(istream&, ostream& out, Connection&, vector<string>) {
    out << "Supported commands:\n";
    out << "  help\n";
    out << "    prints this message\n";
    out << "  read\n";
    out << "    wait from a single line from the server\n";
    out << "  write\n";
    out << "    prompts for a line which is sent to the server (don't provide a command ID)\n";
    out << "  login [<username> <password>]\n";
    out << "    log in using the given username and password or if no credentials\n";
    out << "    are provided then looks for credentials in a file called login.txt\n";
    out << "    with the username and password on the first and second lines\n";
    out << "  quit\n";
    out << "    closes application\n";
    return false;
  }
};

template<class T>
void registerCommandT() {
  registerCommand(Command{ T{} });
}

void initCommands() {
  registerCommandT<Quit>();
  registerCommandT<Read>();
  registerCommandT<Login>();
  registerCommandT<Write>();
  registerCommandT<Help>();
}
