#ifndef IMAP_TESTS_MOCK_SERVER_H
#define IMAP_TESTS_MOCK_SERVER_H

#include <memory>

namespace imap { namespace tests {

  class MockServer {
    struct ServerImpl;
    std::unique_ptr<ServerImpl> m_impl;
    unsigned short m_port;

  public:
    MockServer();
    ~MockServer();

    unsigned short port() const;
  };

} }

#endif