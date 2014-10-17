#ifndef IMAP_CONNECTION_H
#define IMAP_CONNECTION_H

#include "fwd.h"
#include "include_pplx.h"

#include <string>
#include <memory>
#include <vector>

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

    struct CommandBuilder;

  public:
    using Line = std::string;
    using Tokens = std::vector<std::string>;

    Connection();
    explicit Connection(std::string host);
    Connection(std::string host, unsigned short port);

    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&&);
    Connection& operator=(Connection&&);

    bool isOpen() const;

    template<class T>
    void setCommandBuilder(T commandBuilder) {
      using Builder = CommandBuilderImpl<T>;
      auto wrapper = std::make_unique<Builder>(std::move(commandBuilder));
      setCommandBuilderInner(std::move(wrapper));
    }

    pplx::task<Endpoint> open(std::string host);
    pplx::task<Endpoint> open(std::string host, unsigned short port);

    pplx::task<Tokens> readLine();
    pplx::task<std::vector<Tokens>> sendRaw(Line line);

    template<class... Params>
    pplx::task<std::vector<Tokens>> send(Params... params) {
      return sendInner({ params... });
    }

  private:

    void setCommandBuilderInner(std::unique_ptr<CommandBuilder> builder);
    pplx::task<std::vector<Tokens>> sendInner(Tokens tokens);
    pplx::task<std::vector<Line>> readUntilPrefixResult(std::string prefix, std::vector<Line> soFar = {});

    struct CommandBuilder {
      virtual ~CommandBuilder() = default;
      virtual Line join(Tokens tokens) = 0;
      virtual Tokens split(Line line) = 0;
      virtual std::string getPrefix(Line line) = 0;
      virtual Tokens removePrefix(Tokens tokens) = 0;
    };

    template<class T>
    class CommandBuilderImpl : public CommandBuilder {
      T m_inner;

    public:
      CommandBuilderImpl(T t) : m_inner{ std::move(t) } {
      }

      Line join(Tokens tokens) override {
        return m_inner.join(std::move(tokens));
      }

      Tokens split(Line line) override {
        return m_inner.split(std::move(line));
      }

      std::string getPrefix(Line line) override {
        return m_inner.getPrefix(std::move(line));
      }

      Tokens removePrefix(Tokens tokens) override {
        return m_inner.removePrefix(std::move(tokens));
      }
    };

  };

}

#endif
