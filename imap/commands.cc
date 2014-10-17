#include <imap/commands.h>

#include <imap/connection.h>

using pplx::task;
using namespace std;
using Tokens = imap::Connection::Tokens;

namespace imap {

  void throwUnexpectedResult() {
      throw std::runtime_error("unexpected result");
  }

  enum class Result { InvalidResult, OK, NO, BAD };

  Result parseResult(const string& result, bool throwOnBad = true) {
    if (result == "OK")
      return Result::OK;
    if (result == "NO")
      return Result::NO;
    if (result == "BAD") {
      if (throwOnBad)
        throw std::runtime_error("command unknown or arguments invalid");
      return Result::BAD;
    }

    return Result::InvalidResult;
  }

  task<bool> login(Connection& connection, string username, string password) {
    auto task = connection.send("LOGIN", username, password);

    return task.then([](vector<Tokens> tokens) {
      auto result = parseResult(tokens.back().at(1));

      switch (result) {
      default:
        throwUnexpectedResult();
      case Result::OK:
        return true;
      case Result::NO:
        return false;
      }
    });
  }

  task<void> logout(Connection& connection) {
    return connection.send("LOGOUT").then([&connection](vector<Tokens> tokens) {
      auto result = parseResult(tokens.back().at(1));

      if (result != Result::OK)
        throwUnexpectedResult();
    });
  }

}
