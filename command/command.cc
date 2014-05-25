#include "command.h"

#include <imap/connection.h>
#include <imap/command_builder.h>

#include <fstream>

using namespace std;
using imap::Connection;

struct Quit {
  static string name() { return "quit"; }
  bool run(std::istream&, std::ostream&, Connection&, std::vector<std::string>) {
    return true;
  }
};

struct Read {
  static string name() { return "read"; }
  bool run(std::istream&, std::ostream& out, Connection& conn, std::vector<std::string>) {
    out << conn.readLine().get() << "\n";
    return false;
  }
};

struct Write {
  static string name() { return "write"; }
  bool run(std::istream& in, std::ostream& out, Connection& conn, std::vector<std::string>) {
    string line;
    getline(in, line);
    auto task = conn.send(line);
    out << task.get() << "\n";
    return false;
  }
};

struct Login {
  static string name() { return "login"; }
  bool run(std::istream&, std::ostream& out, Connection& conn, std::vector<std::string> args) {
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

    auto line = conn.getCommandBuilder().makeLine("LOGIN " + username + " " + password);
    auto task = conn.sendRaw(line);
    auto response = task.get();
    out << response << "\n";

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
  bool run(std::istream&, std::ostream& out, Connection&, std::vector<std::string>) {
    out << "Supported commands:\n";
    out << "  help\n";
    out << "    prints this message\n";
    out << "  read\n";
    out << "    wait from a single line from the server\n";
    out << "  write\n";
    out << "    prompts for a line which is sent to the server\n";
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
