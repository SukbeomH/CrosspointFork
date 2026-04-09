#pragma once
// ---------------------------------------------------------------
// Minimal test framework for CrossPoint host tests.
// No external dependencies (no GoogleTest, no Catch2).
//
// Usage:
//   TEST(name) { ASSERT_TRUE(expr); ASSERT_EQ(a, b); }
//   int main() { return RUN_ALL_TESTS(); }
// ---------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace test_fw {

struct TestCase {
  std::string name;
  std::function<bool()> fn;
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> tests;
  return tests;
}

inline int registerTest(const char* name, std::function<bool()> fn) {
  registry().push_back({name, std::move(fn)});
  return 0;
}

inline int runAll() {
  int passed = 0, failed = 0;
  for (auto& tc : registry()) {
    bool ok = false;
    try {
      ok = tc.fn();
    } catch (const std::exception& e) {
      std::fprintf(stderr, "  EXCEPTION in %s: %s\n", tc.name.c_str(), e.what());
    } catch (...) {
      std::fprintf(stderr, "  EXCEPTION in %s: unknown\n", tc.name.c_str());
    }
    if (ok) {
      std::printf("  PASS  %s\n", tc.name.c_str());
      ++passed;
    } else {
      std::printf("  FAIL  %s\n", tc.name.c_str());
      ++failed;
    }
  }
  std::printf("\n%d passed, %d failed, %d total\n", passed, failed, passed + failed);
  return failed == 0 ? 0 : 1;
}

}  // namespace test_fw

// Macros --------------------------------------------------------

#define TEST(name)                                                                 \
  static bool test_##name##_body();                                                \
  static int test_##name##_reg = test_fw::registerTest(#name, test_##name##_body); \
  static bool test_##name##_body()

#define ASSERT_TRUE(expr)                                                                       \
  do {                                                                                          \
    if (!(expr)) {                                                                              \
      std::fprintf(stderr, "    ASSERT_TRUE failed: %s  (%s:%d)\n", #expr, __FILE__, __LINE__); \
      return false;                                                                             \
    }                                                                                           \
  } while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b)                                                                              \
  do {                                                                                               \
    if (!((a) == (b))) {                                                                             \
      std::fprintf(stderr, "    ASSERT_EQ failed: %s != %s  (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
      return false;                                                                                  \
    }                                                                                                \
  } while (0)

#define ASSERT_NE(a, b)                                                                              \
  do {                                                                                               \
    if ((a) == (b)) {                                                                                \
      std::fprintf(stderr, "    ASSERT_NE failed: %s == %s  (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
      return false;                                                                                  \
    }                                                                                                \
  } while (0)

#define ASSERT_GE(a, b)                                                                             \
  do {                                                                                              \
    if (!((a) >= (b))) {                                                                            \
      std::fprintf(stderr, "    ASSERT_GE failed: %s < %s  (%s:%d)\n", #a, #b, __FILE__, __LINE__); \
      return false;                                                                                 \
    }                                                                                               \
  } while (0)

#define RUN_ALL_TESTS() test_fw::runAll()
