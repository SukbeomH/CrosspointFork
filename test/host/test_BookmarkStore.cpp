#include "TestFramework.h"

// Mock headers are resolved first (via include path order in CMake)
#include "HalStorage.h"

// Now include the real BookmarkStore header
#include "activities/reader/BookmarkStore.h"

// Helper: reset MockFs between tests
static void resetFs() { MockFs::reset(); }

// ---------------------------------------------------------------
// Basic toggle / has / isEmpty
// ---------------------------------------------------------------

TEST(bookmark_toggle_adds) {
  resetFs();
  BookmarkStore store;
  store.load("/tmp/test_book");

  bool added = store.toggle(0, 1);
  ASSERT_TRUE(added);
  ASSERT_TRUE(store.has(0, 1));
  ASSERT_FALSE(store.isEmpty());
  return true;
}

TEST(bookmark_toggle_removes) {
  resetFs();
  BookmarkStore store;
  store.load("/tmp/test_book");

  store.toggle(0, 1);                 // add
  bool removed = store.toggle(0, 1);  // remove
  ASSERT_FALSE(removed);
  ASSERT_FALSE(store.has(0, 1));
  ASSERT_TRUE(store.isEmpty());
  return true;
}

TEST(bookmark_multiple) {
  resetFs();
  BookmarkStore store;
  store.load("/tmp/test_book");

  store.toggle(0, 1);
  store.toggle(1, 5);
  store.toggle(2, 10);

  ASSERT_TRUE(store.has(0, 1));
  ASSERT_TRUE(store.has(1, 5));
  ASSERT_TRUE(store.has(2, 10));
  ASSERT_FALSE(store.has(3, 15));

  ASSERT_EQ(store.getAll().size(), static_cast<size_t>(3));
  return true;
}

// ---------------------------------------------------------------
// Save and load round-trip
// ---------------------------------------------------------------

TEST(bookmark_save_load_roundtrip) {
  resetFs();

  // Save
  {
    BookmarkStore store;
    store.load("/tmp/test_book");
    store.toggle(0, 1);
    store.toggle(3, 42);
    store.toggle(7, 100);
    store.save();
  }

  // Load in a new store
  {
    BookmarkStore store;
    store.load("/tmp/test_book");
    ASSERT_TRUE(store.has(0, 1));
    ASSERT_TRUE(store.has(3, 42));
    ASSERT_TRUE(store.has(7, 100));
    ASSERT_EQ(store.getAll().size(), static_cast<size_t>(3));
  }
  return true;
}

TEST(bookmark_load_nonexistent_returns_empty) {
  resetFs();
  BookmarkStore store;
  store.load("/tmp/no_such_book");
  ASSERT_TRUE(store.isEmpty());
  return true;
}

// ---------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------

TEST(bookmark_empty_cache_path) {
  resetFs();
  BookmarkStore store;
  store.load("");
  ASSERT_TRUE(store.isEmpty());
  // toggle still works in-memory
  store.toggle(0, 1);
  ASSERT_TRUE(store.has(0, 1));
  // save is no-op (no path)
  store.save();
  return true;
}

TEST(bookmark_save_without_changes_noop) {
  resetFs();
  BookmarkStore store;
  store.load("/tmp/test_book");
  // No toggle → dirty=false → save should be no-op
  store.save();
  // Verify no file was written
  ASSERT_TRUE(MockFs::files().find("/tmp/test_book/bookmarks.bin") == MockFs::files().end());
  return true;
}

int main() { return RUN_ALL_TESTS(); }
