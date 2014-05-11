#include "command.h"

using namespace std;

struct Quit {
  static string name() { return "quit"; }
  bool run(std::istream&, std::ostream&, std::vector<std::string>) {
    return true;
  }
};

struct Help {
  static string name() { return "help"; }
  bool run(std::istream&, std::ostream& out, std::vector<std::string>) {
    out << "Supported commands:\n";
    out << "  help - prints this message\n";
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
  registerCommandT<Help>();
}
