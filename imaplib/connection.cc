#include "include/imap/connection.h"

#include <boost/asio.hpp>
#include <thread>

namespace io = boost::asio;
using boost::asio::ip::tcp;
using pplx::task;
using pplx::task_completion_event;
using std::string;
using boost::system::error_code;

template<class T>
using task_with_error = pplx::task<std::pair<error_code, T>>;

template<class T>
using tce_with_error = pplx::task_completion_event<std::pair<error_code, T>>;

namespace imap {

  struct SharedImpl {
    io::io_service m_service;
    io::io_service::work m_work; // this work keeps the shared thread running
    std::thread m_thread;

    SharedImpl() : m_work{ m_service } {
      m_thread = std::thread([this] {
        m_service.run();
      });
    }

    ~SharedImpl() {
      m_service.stop();
      m_thread.join();
    }
  };

  struct Connection::Impl {
    static SharedImpl* sm_sharedImpl;
    static std::atomic<int> sm_sharedCount;

    SharedImpl* m_sharedImpl;
    tcp::socket m_socket;

    static SharedImpl* createSharedImpl() {
      if (sm_sharedCount++ == 0)
        sm_sharedImpl = new SharedImpl{};
      return sm_sharedImpl;
    }

    Impl()
      : m_sharedImpl{ createSharedImpl() },
        m_socket{ service() }
    {
    }

    ~Impl() {
      if (sm_sharedCount-- == 1)
        delete sm_sharedImpl;
    }

    io::io_service& service() const {
      return m_sharedImpl->m_service;
    }

    void syncConnect(string host, string service) {
      auto resolver = tcp::resolver{ this->service() };
      auto query = tcp::resolver::query{ std::move(host), std::move(service) };
      io::connect(m_socket, resolver.resolve(query));
    }

    task<tcp::resolver::iterator> asyncResolve(string host, string service) {
      using Iterator = tcp::resolver::iterator;

      auto tce = tce_with_error<Iterator>{};

      auto resolver = tcp::resolver{ this->service() };
      auto query = tcp::resolver::query{ std::move(host), std::move(service) };

      resolver.async_resolve(query, [tce](const error_code& e, Iterator i) {
        tce.set(std::make_pair(e, i));
      });

      auto thrower = task_with_error<Iterator>{ tce };

      return thrower.then([](task_with_error<Iterator> result) {
        if (result.get().first)
          throw boost::system::system_error{ result.get().first };
        return result.get().second;
      });
    }

    task<void> asyncConnect(string host, string service) {
      using Iterator = tcp::resolver::iterator;

      auto resolveTask = asyncResolve(std::move(host), std::move(service));
      
      return resolveTask.then([this](task<Iterator> t) {
        auto tce = task_completion_event<error_code>{};

        io::async_connect(m_socket, t.get(), [tce](const error_code& e, Iterator) {
          tce.set(e);
        });

        auto thrower = task<error_code>{ tce };

        return thrower.then([](task<error_code> e) {
          if (e.get())
            throw boost::system::system_error(e.get());
        });
      });
    }
  };

  SharedImpl* Connection::Impl::sm_sharedImpl = nullptr;
  std::atomic<int> Connection::Impl::sm_sharedCount = 0;


  Connection::Connection() : m_impl{ std::make_unique<Impl>() } {
  }

  Connection::~Connection() = default;

  Connection::Connection(Connection&& other)
    : m_impl{ std::move(other.m_impl) }
  {
  }

  Connection& Connection::operator=(Connection&& other) {
    m_impl = std::move(other.m_impl);
    return *this;
  }

  Connection::Connection(string host) : Connection() {
    m_impl->syncConnect(std::move(host), "imap");
  }

  Connection::Connection(string host, unsigned short port) : Connection() {
    m_impl->syncConnect(std::move(host), std::to_string(port));
  }

  bool Connection::isOpen() const {
    return m_impl->m_socket.is_open();
  }

  task<void> Connection::open(string host) {
    return m_impl->asyncConnect(std::move(host), "imap");
  }

  task<void> Connection::open(string host, unsigned short port) {
    return m_impl->asyncConnect(std::move(host), std::to_string(port));
  }

}