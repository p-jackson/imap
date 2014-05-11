#include <imap/imap.h>

#include <iostream>

int main() {
  const auto title = "narrow";
  std::cout << imap::greeting(title) << std::endl;
  return 0;
}
