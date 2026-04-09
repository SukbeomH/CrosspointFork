#include "TestFramework.h"

// Mock headers first
#include "Arduino.h"
#include "HalStorage.h"

// Now include the real AchievementStore header
#include "AchievementStore.h"

static void resetFs() { MockFs::reset(); }

// ---------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------

TEST(achievement_initial_state) {
  resetFs();
  AchievementStore store;
  ASSERT_EQ(store.getBitmap(), static_cast<uint16_t>(0));
  ASSERT_EQ(store.unlockedCount(), 0);
  ASSERT_FALSE(store.isUnlocked(AchievementId::FirstBookOpened));
  return true;
}

// ---------------------------------------------------------------
// Basic unlock via checkAndUnlock
// ---------------------------------------------------------------

TEST(achievement_unlock_first_book) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.booksOpened = 1;

  int newUnlocks = store.checkAndUnlock(stats);
  ASSERT_GE(newUnlocks, 1);
  ASSERT_TRUE(store.isUnlocked(AchievementId::FirstBookOpened));
  ASSERT_FALSE(store.isUnlocked(AchievementId::FiveBooksOpened));
  return true;
}

TEST(achievement_unlock_multiple_thresholds) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.booksOpened = 10;
  stats.totalSessions = 25;

  int newUnlocks = store.checkAndUnlock(stats);

  // Should unlock: FirstBookOpened, FiveBooksOpened, TenBooksOpened
  ASSERT_TRUE(store.isUnlocked(AchievementId::FirstBookOpened));
  ASSERT_TRUE(store.isUnlocked(AchievementId::FiveBooksOpened));
  ASSERT_TRUE(store.isUnlocked(AchievementId::TenBooksOpened));
  ASSERT_FALSE(store.isUnlocked(AchievementId::TwentyFiveBooksOpened));

  // Should unlock: FirstSession, TenSessions, TwentyFiveSessions
  ASSERT_TRUE(store.isUnlocked(AchievementId::FirstSession));
  ASSERT_TRUE(store.isUnlocked(AchievementId::TenSessions));
  ASSERT_TRUE(store.isUnlocked(AchievementId::TwentyFiveSessions));
  ASSERT_FALSE(store.isUnlocked(AchievementId::FiftySessions));

  ASSERT_EQ(newUnlocks, 6);
  return true;
}

// ---------------------------------------------------------------
// Idempotent — re-checking same stats yields 0 new unlocks
// ---------------------------------------------------------------

TEST(achievement_idempotent) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.booksOpened = 5;
  store.checkAndUnlock(stats);

  int second = store.checkAndUnlock(stats);
  ASSERT_EQ(second, 0);
  return true;
}

// ---------------------------------------------------------------
// Time-based achievements
// ---------------------------------------------------------------

TEST(achievement_time_milestones) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  constexpr uint32_t MS_PER_HOUR = 60UL * 60UL * 1000UL;
  stats.totalReadingMs = 5 * MS_PER_HOUR;

  store.checkAndUnlock(stats);

  ASSERT_TRUE(store.isUnlocked(AchievementId::OneHourTotal));
  ASSERT_TRUE(store.isUnlocked(AchievementId::FiveHoursTotal));
  ASSERT_FALSE(store.isUnlocked(AchievementId::TenHoursTotal));
  return true;
}

// ---------------------------------------------------------------
// Bookmark and streak achievements
// ---------------------------------------------------------------

TEST(achievement_bookmark_and_streak) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.totalBookmarks = 1;
  stats.consecutiveGoalDays = 7;

  store.checkAndUnlock(stats);

  ASSERT_TRUE(store.isUnlocked(AchievementId::FirstBookmark));
  ASSERT_TRUE(store.isUnlocked(AchievementId::ThreeDayStreak));
  ASSERT_TRUE(store.isUnlocked(AchievementId::SevenDayStreak));
  return true;
}

// ---------------------------------------------------------------
// Save / load round-trip
// ---------------------------------------------------------------

TEST(achievement_save_load_roundtrip) {
  resetFs();

  // Unlock some achievements and save
  {
    AchievementStore store;
    AggregateReadingStats stats;
    stats.booksOpened = 5;
    stats.totalSessions = 1;
    store.checkAndUnlock(stats);  // saves automatically on new unlocks
  }

  // Load in fresh store
  {
    AchievementStore store;
    bool loaded = store.loadFromFile();
    ASSERT_TRUE(loaded);
    ASSERT_TRUE(store.isUnlocked(AchievementId::FirstBookOpened));
    ASSERT_TRUE(store.isUnlocked(AchievementId::FiveBooksOpened));
    ASSERT_TRUE(store.isUnlocked(AchievementId::FirstSession));
    ASSERT_FALSE(store.isUnlocked(AchievementId::TenBooksOpened));
  }
  return true;
}

TEST(achievement_load_nonexistent) {
  resetFs();
  AchievementStore store;
  bool loaded = store.loadFromFile();
  ASSERT_FALSE(loaded);
  ASSERT_EQ(store.getBitmap(), static_cast<uint16_t>(0));
  return true;
}

// ---------------------------------------------------------------
// Reset
// ---------------------------------------------------------------

TEST(achievement_reset) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.booksOpened = 10;
  store.checkAndUnlock(stats);
  ASSERT_TRUE(store.unlockedCount() > 0);

  store.reset();
  ASSERT_EQ(store.unlockedCount(), 0);
  ASSERT_EQ(store.getBitmap(), static_cast<uint16_t>(0));
  return true;
}

// ---------------------------------------------------------------
// unlockedCount correctness
// ---------------------------------------------------------------

TEST(achievement_unlocked_count) {
  resetFs();
  AchievementStore store;

  AggregateReadingStats stats;
  stats.booksOpened = 1;
  stats.totalSessions = 1;
  stats.totalBookmarks = 1;
  store.checkAndUnlock(stats);

  // FirstBookOpened + FirstSession + FirstBookmark = 3
  ASSERT_EQ(store.unlockedCount(), 3);
  return true;
}

int main() { return RUN_ALL_TESTS(); }
