#ifndef IMAP_CONNECTION_H
#define IMAP_CONNECTION_H

#include "include_pplx.h"

#include <string>
#include <memory>

namespace imap {

  class Endpoint {
    struct Impl;
    std::shared_ptr<const Impl> m_impl;

    friend class Connection;

  public:
    Endpoint();
    ~Endpoint();

    bool isV4() const;
    bool isV6() const;
    std::string toString() const;
  };

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

    pplx::task<Endpoint> open(std::string host);
    pplx::task<Endpoint> open(std::string host, unsigned short port);
  };

}

#endif
