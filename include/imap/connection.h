#ifndef IMAP_CONNECTION_H
#define IMAP_CONNECTION_H

#include "fwd.h"
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
    explicit Connection(std::string host);
    Connection(std::string host, unsigned short port);

    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&);
    Connection& operator=(Connection&&);

    bool isOpen() const;
    CommandBuilder& getCommandBuilder() const;

    pplx::task<Endpoint> open(std::string host);
    pplx::task<Endpoint> open(std::string host, unsigned short port);

    pplx::task<std::string> readLine();
    pplx::task<std::string> send(std::string line);
    pplx::task<std::string> sendRaw(std::string line);
  };

}

#endif
