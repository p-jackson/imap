#include "mock_server.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <future>
#include <thread>

namespace io = boost::asio;
using boost::asio::ip::tcp;
using namespace std;

tcp::acceptor findPort(io::io_service& service) {
  for (unsigned short port = 49152; port <= 65535; ++port) {
    try {
      return tcp::acceptor{ service, { tcp::v4(), port } };
    }
    catch (boost::system::system_error&) {
    }
  }

  throw runtime_error("No free tcp ports for MockServer");
}

class Session {
  io::ssl::stream<tcp::socket> m_socket;

public:
  Session(io::io_service& service, io::ssl::context& context)
    : m_socket{ service, context }
  {
  }

  auto socket() -> decltype(m_socket.lowest_layer()) {
    return m_socket.lowest_layer();
  }

  void start() {
    m_socket.async_handshake(io::ssl::stream_base::server, [this](const boost::system::error_code& e) {
      if (e) {
        std::cerr << boost::system::system_error{ e }.what() << "\n";
        delete this;
      }

      delete this;
    });
  }
};

namespace imap { namespace tests {

  io::ssl::context configuredContext() {
    auto context = io::ssl::context{ io::ssl::context::sslv23 };

    try {
      context.use_certificate_file("server.pem", io::ssl::context::pem);
      context.use_private_key_file("privkey.pem", io::ssl::context::pem);
    }
    catch (std::exception& e) {
      std::cerr << e.what() << "\n";
    }

    return context;
  }

  struct MockServer::ServerImpl {
    io::io_service m_service;
    tcp::acceptor m_acceptor;
    io::ssl::context m_context;
    thread m_thread;

    ServerImpl(unsigned short& port)
      : m_acceptor{ findPort(m_service) },
        m_context{ configuredContext() }
    {
      port = m_acceptor.local_endpoint().port();

      queueAccept();

      m_thread = thread{ [this] {
        m_service.run();
        auto work = io::io_service::work{ m_service };
      } };
    }

    ~ServerImpl() {
      m_service.stop();
      m_thread.join();
    }

    void queueAccept() {
      auto session = new Session{ m_service, m_context };
      m_acceptor.async_accept(session->socket(), [this, session](const boost::system::error_code& e) {
        if (e)
          delete session;
        else
          session->start();

        queueAccept();
      });
    }

  };

  MockServer::MockServer() {
    m_impl = make_unique<ServerImpl>(m_port);
  }

  MockServer::~MockServer() = default;

  unsigned short MockServer::port() const {
    return m_port;
  }

} }