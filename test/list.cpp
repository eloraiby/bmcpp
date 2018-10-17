#include <list.hpp>
#include <cstdint>
#include <cstddef>
#include <cassert>

using BmCpp::List;
using std::uint32_t;
using std::size_t;

int testPushBack() {
  List<uint32_t> list;

  for (size_t i = 0; i < 10; ++i) {
    list.push_back(uint32_t(i));
  }

  size_t count = 0;
  for (auto &el : list) {
    assert(el == count);
    count++;
  }

  return 0;
}

int testPushFront() {
  List<uint32_t> list;

  const size_t range = 10;
  for (size_t i = 0; i < range; ++i) {
    list.push_front(uint32_t(i));
  }

  size_t count = range - 1;
  for (auto &e : list) {
    assert(e == count);
    count--;
  }

  return 0;
}

int main(void) {
  return testPushBack()
    | testPushFront();
}
