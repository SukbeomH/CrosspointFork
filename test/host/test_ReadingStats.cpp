#include "TestFramework.h"

// Mock headers first
#include "Arduino.h"
#include "HalStorage.h"

// Now include the real ReadingStats header
#include "ReadingStats.h"

static void resetFs() { MockFs::reset(); }

// ---------------------------------------------------------------
// ReadingStats — defaults
// ---------------------------------------------------------------

TEST(readingstats_defaults) {
  ReadingStats rs;
  ASSERT_EQ(rs.totalReadingMs, static_cast<uint32_t>(0));
  ASSERT_EQ(rs.sessionCount, static_cast<uint16_t>(0));
  ASSERT_EQ(rs.lastSessionMs, static_cast<uint32_t>(0));
  return true;
}

// ---------------------------------------------------------------
// Save / load round-trip
// ---------------------------------------------------------------

TEST(readingstats_save_load_roundtrip) {
  resetFs();

  // Save
  {
    ReadingStats rs;
    rs.totalReadingMs = 1800000;  // 30 minutes
    rs.sessionCount = 5;
    rs.lastSessionMs = 600000;  // 10 minutes
    rs.saveToFile("/tmp/book1");
  }

  // Load
  {
    ReadingStats rs;
    bool loaded = rs.loadFromFile("/tmp/book1");
    ASSERT_TRUE(loaded);
    ASSERT_EQ(rs.totalReadingMs, static_cast<uint32_t>(1800000));
    ASSERT_EQ(rs.sessionCount, static_cast<uint16_t>(5));
    ASSERT_EQ(rs.lastSessionMs, static_cast<uint32_t>(600000));
  }
  return true;
}

TEST(readingstats_load_nonexistent) {
  resetFs();
  ReadingStats rs;
  bool loaded = rs.loadFromFile("/tmp/no_such");
  ASSERT_FALSE(loaded);
  return true;
}

// ---------------------------------------------------------------
// Large values (uint32 max-ish)
// ---------------------------------------------------------------

TEST(readingstats_large_values) {
  resetFs();

  {
    ReadingStats rs;
    rs.totalReadingMs = 86400000;  // 24 hours
    rs.sessionCount = 1000;
    rs.lastSessionMs = 7200000;  // 2 hours
    rs.saveToFile("/tmp/heavy_reader");
  }

  {
    ReadingStats rs;
    bool loaded = rs.loadFromFile("/tmp/heavy_reader");
    ASSERT_TRUE(loaded);
    ASSERT_EQ(rs.totalReadingMs, static_cast<uint32_t>(86400000));
    ASSERT_EQ(rs.sessionCount, static_cast<uint16_t>(1000));
    ASSERT_EQ(rs.lastSessionMs, static_cast<uint32_t>(7200000));
  }
  return true;
}

// ---------------------------------------------------------------
// Corrupt file
// ---------------------------------------------------------------

TEST(readingstats_corrupt_version) {
  resetFs();
  std::vector<uint8_t> bad(13, 0);
  bad[0] = 99;  // wrong version
  MockFs::files()["/tmp/corrupt/statistics.bin"] = bad;

  ReadingStats rs;
  bool loaded = rs.loadFromFile("/tmp/corrupt");
  ASSERT_FALSE(loaded);
  return true;
}

TEST(readingstats_truncated_file) {
  resetFs();
  // Too short — only 5 bytes
  std::vector<uint8_t> short_data = {1, 0, 0, 0, 0};
  MockFs::files()["/tmp/short/statistics.bin"] = short_data;

  ReadingStats rs;
  bool loaded = rs.loadFromFile("/tmp/short");
  ASSERT_FALSE(loaded);
  return true;
}

// ---------------------------------------------------------------
// ReadingSessionTracker — session tracking
// ---------------------------------------------------------------

TEST(session_tracker_basic) {
  setMillis(1000);
  ReadingSessionTracker tracker;
  tracker.start();
  ASSERT_TRUE(tracker.isActive());

  setMillis(5000);
  tracker.tick();
  ASSERT_GE(tracker.getElapsedMs(), static_cast<uint32_t>(4000));

  uint32_t total = tracker.stop();
  ASSERT_FALSE(tracker.isActive());
  ASSERT_GE(total, static_cast<uint32_t>(4000));
  return true;
}

TEST(session_tracker_stop_without_start) {
  ReadingSessionTracker tracker;
  uint32_t total = tracker.stop();
  ASSERT_EQ(total, static_cast<uint32_t>(0));
  return true;
}

TEST(session_tracker_caps_large_delta) {
  // If delta > 10 seconds, it's treated as 0 (sleep/wake gap handling)
  setMillis(1000);
  ReadingSessionTracker tracker;
  tracker.start();

  setMillis(2000);
  tracker.tick();  // +1000ms

  setMillis(20000);  // 18 second gap (> 10s cap)
  tracker.tick();    // delta capped to 0

  setMillis(21000);
  tracker.tick();  // +1000ms

  uint32_t total = tracker.stop();
  // Should be ~2000ms (1000 + 0 + 1000), not 20000ms
  ASSERT_GE(total, static_cast<uint32_t>(1900));
  ASSERT_TRUE(total < 5000);
  return true;
}

int main() { return RUN_ALL_TESTS(); }
