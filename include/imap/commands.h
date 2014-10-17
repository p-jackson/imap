#ifndef IMAP_COMMANDS_H
#define IMAP_COMMANDS_H

#include "fwd.h"
#include "include_pplx.h"

namespace imap {

  // Returns false if username or password rejected
  pplx::task<bool> login(Connection& connection, std::string username, std::string password);

  pplx::task<void> logout(Connection& connection);

}

#endif
