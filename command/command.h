#ifndef IMAP_COMMAND_COMMAND_H
#define IMAP_COMMAND_COMMAND_H

#include <imap/fwd.h>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Command {
public:
  template<class T>
  Command(T) {
    m_runnable = std::make_unique<RunnableImpl<T>>();
  }

  Command(const Command& o) {
    m_runnable = o.m_runnable->copy();
  }

  bool run(std::istream& in, std::ostream& out, imap::Connection& conn, std::vector<std::string> args) {
    return m_runnable->run(in, out, conn, std::move(args));
  }

  bool matches(const std::string& name) const {
    return m_runnable->matches(name);
  }

private:
  struct Runnable {
    virtual ~Runnable() {}
    virtual std::unique_ptr<Runnable> copy() const = 0;
    virtual bool run(std::istream&, std::ostream&, imap::Connection&, std::vector<std::string>) = 0;
    virtual bool matches(const std::string& name) const = 0;
  };

  template<class T>
  class RunnableImpl : public Runnable {
    T m_innerCommand;
  public:
    std::unique_ptr<Runnable> copy() const override {
      return std::make_unique<RunnableImpl<T>>();
    }

    bool run(std::istream& in, std::ostream& out, imap::Connection& conn, std::vector<std::string> args) override {
      return m_innerCommand.run(in, out, conn, move(args));
    }

    bool matches(const std::string& name) const override {
      return name == T::name();
    }
  };

  std::unique_ptr<Runnable> m_runnable;
};

void initCommands();
void registerCommand(Command);

#endif