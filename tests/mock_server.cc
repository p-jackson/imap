#include "mock_server.h"

#include <boost/asio.hpp>
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

namespace imap { namespace tests {

  struct MockServer::ServerImpl {
    atomic<bool> m_stop;
    io::io_service m_service;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    io::deadline_timer m_timer;
    thread m_thread;

    ServerImpl(unsigned short& port)
      : m_stop{ false },
        m_acceptor{ findPort(m_service) },
        m_socket{ m_service },
        m_timer{ m_service }
    {
      port = m_acceptor.local_endpoint().port();

      queueAccept();

      m_thread = thread{ [this] {
        while (!m_stop)
          m_service.run_one();
      } };
    }

    ~ServerImpl() {
      m_stop = true;
      m_thread.join();
    }

    void queueAccept() {
      m_acceptor.async_accept(m_socket, boost::bind(&ServerImpl::onAccept, this, io::placeholders::error));
      m_timer.expires_from_now(boost::posix_time::milliseconds{ 50 });
      m_timer.async_wait(bind(&ServerImpl::queueAccept, this));
    }

    void onAccept(const boost::system::error_code&) {
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