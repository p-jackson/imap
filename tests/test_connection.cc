#include "mock_server.h"

#include "include/imap/connection.h"

#include <catch.hpp>

using namespace imap;

TEST_CASE("Connections can be default constructed", "[connection]") {
  auto c = Connection{};

  SECTION("in which case they're 'closed'") {
    REQUIRE_FALSE(c.isOpen());
  }
}

TEST_CASE("A Connection can be opened to an imap service", "[connection]") {
  auto server = tests::MockServer{};

  SECTION("synchronously using a constructor") {
    auto c = Connection{ "localhost", server.port() };
    REQUIRE(c.isOpen());
  }

  SECTION("asynchronously using open") {
    auto c = Connection{};
    auto openTask = c.open("localhost", server.port());

    auto continuationCalled = false;
    openTask.then([&](Endpoint) {
      REQUIRE(c.isOpen());
      continuationCalled = true;
    }).wait();

    REQUIRE(c.isOpen());
    REQUIRE(continuationCalled);
  }

  SECTION("connecting asynchronously allows you to see the ip address of the service") {
    auto c = Connection{};
    auto openTask = c.open("localhost", server.port());

    openTask.then([](Endpoint e) {
      REQUIRE(e.toString() == "127.0.0.1");
      REQUIRE(e.isV4());
      REQUIRE(!e.isV6());
    }).wait();
  }

  SECTION("two connections can be opened at the same time") {
    auto c = Connection{ "localhost", server.port() };
    auto d = Connection{ "localhost", server.port() };
    REQUIRE(c.isOpen());
    REQUIRE(d.isOpen());
  }
}

TEST_CASE("Connection can't be opened to an invalid imap service", "[connection][slow]") {
  auto server = tests::MockServer{};

  SECTION("fails synchronously in constructor by throwing an exception") {
    // This test assumes there's no imap server running on the local machine
    // (MockServer doesn't use the default imap port)
    REQUIRE_THROWS(auto c = Connection{ "localhost" });

    REQUIRE_THROWS(auto d = Connection{ "wrong_hostname" });
  }

  SECTION("fails asynchronously by throwing an exception in the task") {
    auto c = Connection{};
    auto openTask = c.open("wrong_localhost");

    auto continuationCalled = false;
    openTask.then([&](pplx::task<Endpoint> t) {
      REQUIRE(!c.isOpen());
      REQUIRE_THROWS(t.get());
      continuationCalled = true;
    }).wait();

    REQUIRE(!c.isOpen());
    REQUIRE(continuationCalled);
  }
}