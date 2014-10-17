#include <imap/connection.h>

#include <imap/default_command_builder.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
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

  struct Endpoint::Impl {
    io::ip::address m_address;

    Impl(io::ip::address a) : m_address{ std::move(a) } {
    }
  };

  Endpoint::Endpoint() = default;
  Endpoint::~Endpoint() = default;

  bool Endpoint::isV4() const {
    return m_impl->m_address.is_v4();
  }

  bool Endpoint::isV6() const {
    return m_impl->m_address.is_v6();
  }

  string Endpoint::toString() const {
    return m_impl->m_address.to_string();
  }

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
    io::ssl::context m_context;
    io::ssl::stream<tcp::socket> m_socket;
    io::streambuf m_readBuffer;
    std::unique_ptr<CommandBuilder> m_builder;

    static SharedImpl* createSharedImpl() {
      if (sm_sharedCount++ == 0)
        sm_sharedImpl = new SharedImpl{};
      return sm_sharedImpl;
    }

    Impl()
      : m_sharedImpl{ createSharedImpl() },
        m_context{ io::ssl::context::sslv23 },
        m_socket{ service(), m_context }
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
      auto ctx = io::ssl::context{ io::ssl::context::sslv23 };
      io::connect(m_socket.lowest_layer(), resolver.resolve(query));
      m_socket.handshake(io::ssl::stream_base::client);
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

    task<void> asyncHandshake() {
      auto tce = task_completion_event<error_code>{};

      m_socket.async_handshake(io::ssl::stream_base::client, [tce](const error_code& e) {
        tce.set(e);
      });

      auto thrower = task<error_code>{ tce };

      return thrower.then([](task<error_code> result) {
        if (result.get())
          throw boost::system::system_error{ result.get() };
      });
    }

    task<Endpoint> asyncConnect(string host, string service) {
      using Iterator = tcp::resolver::iterator;

      auto resolveTask = asyncResolve(std::move(host), std::move(service));
      
      auto connectTask = resolveTask.then([this](task<Iterator> t) {
        auto tce = tce_with_error<Iterator>{};

        io::async_connect(m_socket.lowest_layer(), t.get(), [tce](const error_code& e, Iterator i) {
          tce.set(std::make_pair(e, i));
        });

        auto thrower = task_with_error<Iterator>{ tce };

        return thrower.then([](task_with_error<Iterator> result) {
          if (result.get().first)
            throw boost::system::system_error(result.get().first);

          auto endpoint = Endpoint{};
          auto address = static_cast<tcp::endpoint>(*result.get().second).address();
          endpoint.m_impl = std::make_shared<const Endpoint::Impl>(address);
          return endpoint;
        });
      });

      return connectTask.then([this](task<Endpoint> t) {
        auto endpoint = t.get();
        return asyncHandshake().then([endpoint]() mutable {
          return endpoint;
        });
      });
    }

    task<size_t> asyncWrite(string line) {
      auto buffer = io::buffer(line.data(), line.length());

      auto tce = tce_with_error<size_t>{};

      io::async_write(m_socket, buffer, [tce](const error_code& e, size_t written) {
        tce.set(std::make_pair(e, written));
      });

      auto thrower = task_with_error<size_t>{ tce };

      return thrower.then([](task_with_error<size_t> result) {
        if (result.get().first)
          throw boost::system::system_error{ result.get().first };

        return result.get().second;
      });
    }

    task<string> asyncReadLine() {
      auto tce = tce_with_error<size_t>{};

      io::async_read_until(m_socket, m_readBuffer, "\r\n", [tce](const error_code& e, size_t read) {
        tce.set(std::make_pair(e, read));
      });

      auto thrower = task_with_error<size_t>{ tce };

      return thrower.then([this](task_with_error<size_t> result) {
        if (result.get().first)
          throw boost::system::system_error{ result.get().first };

        std::istream is(&m_readBuffer);
        auto line = string{};
        std::getline(is, line);

        return line;
      });
    }
  };

  SharedImpl* Connection::Impl::sm_sharedImpl = nullptr;
  std::atomic<int> Connection::Impl::sm_sharedCount = 0;


  void throwIfNotOpen(Connection* connection, const char* what) {
    if (connection->isOpen())
      return;

    auto msg = string{ "Can't " } + what + " from an unopened connection";
    throw std::runtime_error{ msg };
  }


  Connection::Connection() : m_impl{ std::make_unique<Impl>() } {
    setCommandBuilder(DefaultCommandBuilder{});
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
    if (m_impl)
      return m_impl->m_socket.lowest_layer().is_open();
    else
      return false;
  }

  void Connection::setCommandBuilderInner(std::unique_ptr<CommandBuilder> builder) {
    m_impl->m_builder = std::move(builder);
  }

  task<Endpoint> Connection::open(string host) {
    return m_impl->asyncConnect(std::move(host), "imap");
  }

  task<Endpoint> Connection::open(string host, unsigned short port) {
    return m_impl->asyncConnect(std::move(host), std::to_string(port));
  }

  task<Connection::Tokens> Connection::readLine() {
    throwIfNotOpen(this, "read");
    return m_impl->asyncReadLine().then([=](Line line) {
      return m_impl->m_builder->split(line);
    });
  }

  task<std::vector<Connection::Line>> Connection::readUntilPrefixResult(string prefix, std::vector<Line> soFar) {
    const auto readLine = m_impl->asyncReadLine();

    return readLine.then([=](Line line) mutable {
      soFar.push_back(line);

      if (m_impl->m_builder->getPrefix(line) != prefix)
        return readUntilPrefixResult(prefix, soFar);

      auto tce = task_completion_event<std::vector<Line>>{};
      tce.set(soFar);
      return task<std::vector<Line>>{ tce };
    });
  }

  task<std::vector<Connection::Tokens>> Connection::sendRaw(Line line) {
    throwIfNotOpen(this, "sendRaw");

    const auto prefix = m_impl->m_builder->getPrefix(line);
    const auto writeLine = m_impl->asyncWrite(std::move(line));

    return writeLine.then([=](size_t) {
      return readUntilPrefixResult(prefix).then([=](std::vector<Line> lines) {
        std::vector<Tokens> result;

        std::transform(begin(lines), end(lines), std::back_inserter(result), [=](Line l) {
          return m_impl->m_builder->split(l);
        });

        return move(result);
      });
    });
  }

  task<std::vector<Connection::Tokens>> Connection::sendInner(Tokens tokens) {
    throwIfNotOpen(this, "send");

    auto line = m_impl->m_builder->join(std::move(tokens));
    return sendRaw(line);
  }

}