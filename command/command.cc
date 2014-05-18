#include "command.h"

#include <imap/connection.h>

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

struct Help {
  static string name() { return "help"; }
  bool run(std::istream&, std::ostream& out, Connection&, std::vector<std::string>) {
    out << "Supported commands:\n";
    out << "  help - prints this message\n";
    out << "  read - wait from a single line from the server\n";
    out << "  quit - closes application\n";
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
  registerCommandT<Help>();
}
