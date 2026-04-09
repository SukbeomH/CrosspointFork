#pragma once

#include <HalStorage.h>
#include <Logging.h>

#include <cstdint>

#include "ReadingStats.h"

// Lightweight achievement system for CrossPoint Korean.
// Tracks 15 reading milestones based on per-book ReadingStats.
// Persisted as a 2-byte bitmap to SD card (/.crosspoint/achievements.bin).
//
// Design: header-only, no dynamic allocation, ~200 bytes resident RAM.
// UI Activity is NOT included here -- this is data/logic only.

enum class AchievementId : uint8_t {
  FirstBookOpened = 0,
  FiveBooksOpened,
  TenBooksOpened,
  TwentyFiveBooksOpened,
  FirstSession,
  TenSessions,
  TwentyFiveSessions,
  FiftySessions,
  OneHourTotal,
  FiveHoursTotal,
  TenHoursTotal,
  TwentyFourHoursTotal,
  FirstBookmark,
  ThreeDayStreak,
  SevenDayStreak,
  _COUNT  // Must be <= 16 (fits in uint16_t bitmap)
};

static_assert(static_cast<int>(AchievementId::_COUNT) <= 16, "Achievement count exceeds 16-bit bitmap capacity");

struct Achievement {
  const char* title;
  const char* description;
  bool unlocked;
};

// Aggregate stats collected across all books for achievement evaluation.
// Populated by the caller before calling checkAndUnlock().
struct AggregateReadingStats {
  uint16_t booksOpened = 0;
  uint16_t totalSessions = 0;
  uint32_t totalReadingMs = 0;
  uint16_t totalBookmarks = 0;
  uint8_t consecutiveGoalDays = 0;
};

class AchievementStore {
 public:
  static constexpr const char* STORAGE_PATH = "/.crosspoint/achievements.bin";
  static constexpr uint8_t FILE_VERSION = 1;
  // File layout: [1 byte version] [2 bytes bitmap LE]
  static constexpr size_t FILE_SIZE = 3;

  // Load bitmap from SD card. Returns false if file missing or corrupt.
  bool loadFromFile() {
    FsFile f;
    if (!Storage.openFileForRead("ACH", STORAGE_PATH, f)) {
      LOG_DBG("ACH", "No achievements file, starting fresh");
      bitmap_ = 0;
      return false;
    }

    uint8_t data[FILE_SIZE];
    size_t bytesRead = f.read(data, sizeof(data));
    f.close();

    if (bytesRead < FILE_SIZE || data[0] != FILE_VERSION) {
      LOG_DBG("ACH", "Invalid achievements file (version=%d, bytes=%d)", data[0], bytesRead);
      bitmap_ = 0;
      return false;
    }

    bitmap_ = static_cast<uint16_t>(data[1]) | (static_cast<uint16_t>(data[2]) << 8);
    LOG_DBG("ACH", "Loaded achievements bitmap: 0x%04X", bitmap_);
    return true;
  }

  // Save bitmap to SD card.
  void saveToFile() const {
    Storage.mkdir("/.crosspoint");

    FsFile f;
    if (!Storage.openFileForWrite("ACH", STORAGE_PATH, f)) {
      LOG_ERR("ACH", "Failed to save achievements");
      return;
    }

    uint8_t data[FILE_SIZE];
    data[0] = FILE_VERSION;
    data[1] = bitmap_ & 0xFF;
    data[2] = (bitmap_ >> 8) & 0xFF;
    f.write(data, sizeof(data));
    f.close();
    LOG_DBG("ACH", "Saved achievements bitmap: 0x%04X", bitmap_);
  }

  // Check all achievements against aggregate stats.
  // Newly unlocked achievements are flagged; returns count of new unlocks.
  int checkAndUnlock(const AggregateReadingStats& stats) {
    int newUnlocks = 0;

    newUnlocks += tryUnlock(AchievementId::FirstBookOpened, stats.booksOpened >= 1);
    newUnlocks += tryUnlock(AchievementId::FiveBooksOpened, stats.booksOpened >= 5);
    newUnlocks += tryUnlock(AchievementId::TenBooksOpened, stats.booksOpened >= 10);
    newUnlocks += tryUnlock(AchievementId::TwentyFiveBooksOpened, stats.booksOpened >= 25);

    newUnlocks += tryUnlock(AchievementId::FirstSession, stats.totalSessions >= 1);
    newUnlocks += tryUnlock(AchievementId::TenSessions, stats.totalSessions >= 10);
    newUnlocks += tryUnlock(AchievementId::TwentyFiveSessions, stats.totalSessions >= 25);
    newUnlocks += tryUnlock(AchievementId::FiftySessions, stats.totalSessions >= 50);

    constexpr uint32_t MS_PER_HOUR = 60UL * 60UL * 1000UL;
    newUnlocks += tryUnlock(AchievementId::OneHourTotal, stats.totalReadingMs >= 1 * MS_PER_HOUR);
    newUnlocks += tryUnlock(AchievementId::FiveHoursTotal, stats.totalReadingMs >= 5 * MS_PER_HOUR);
    newUnlocks += tryUnlock(AchievementId::TenHoursTotal, stats.totalReadingMs >= 10 * MS_PER_HOUR);
    newUnlocks += tryUnlock(AchievementId::TwentyFourHoursTotal, stats.totalReadingMs >= 24 * MS_PER_HOUR);

    newUnlocks += tryUnlock(AchievementId::FirstBookmark, stats.totalBookmarks >= 1);

    newUnlocks += tryUnlock(AchievementId::ThreeDayStreak, stats.consecutiveGoalDays >= 3);
    newUnlocks += tryUnlock(AchievementId::SevenDayStreak, stats.consecutiveGoalDays >= 7);

    if (newUnlocks > 0) {
      saveToFile();
      LOG_DBG("ACH", "Unlocked %d new achievements (bitmap=0x%04X)", newUnlocks, bitmap_);
    }

    return newUnlocks;
  }

  // Query whether a specific achievement is unlocked.
  bool isUnlocked(AchievementId id) const { return (bitmap_ & (1U << static_cast<uint8_t>(id))) != 0; }

  // Get the count of unlocked achievements.
  int unlockedCount() const {
    int count = 0;
    for (int i = 0; i < static_cast<int>(AchievementId::_COUNT); ++i) {
      if (bitmap_ & (1U << i)) ++count;
    }
    return count;
  }

  // Get the raw bitmap (for debugging / display).
  uint16_t getBitmap() const { return bitmap_; }

  // Reset all achievements.
  void reset() {
    bitmap_ = 0;
    saveToFile();
  }

 private:
  uint16_t bitmap_ = 0;

  // Try to unlock an achievement. Returns 1 if newly unlocked, 0 otherwise.
  int tryUnlock(AchievementId id, bool condition) {
    if (!condition) return 0;
    uint16_t mask = 1U << static_cast<uint8_t>(id);
    if (bitmap_ & mask) return 0;  // Already unlocked
    bitmap_ |= mask;
    return 1;
  }
};
