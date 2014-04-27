#ifndef IMAP_CONNECTION_H
#define IMAP_CONNECTION_H

#include "include_pplx.h"

#include <string>
#include <memory>

namespace imap {

  class Connection {
    struct Impl;
    std::unique_ptr<Impl> m_impl;

  public:
    Connection();
    Connection(std::string host);
    Connection(std::string host, unsigned short port);

    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&);
    Connection& operator=(Connection&&);

    bool isOpen() const;

    pplx::task<void> open(std::string host);
    pplx::task<void> open(std::string host, unsigned short port);
  };

}

#endif
