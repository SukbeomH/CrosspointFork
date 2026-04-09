#include "TestFramework.h"

// Mock HalStorage is resolved via include-path priority
#include "HalStorage.h"

// Real production headers — CrossPointSettings.h compiles with our HalStorage mock
#include "BookSettings.h"

// CrossPointSettings singleton definition (needed by linker)
CrossPointSettings CrossPointSettings::instance;

static void resetFs() { MockFs::reset(); }

// ---------------------------------------------------------------
// Default values
// ---------------------------------------------------------------

TEST(booksettings_defaults) {
  BookSettings bs;
  ASSERT_EQ(bs.lineSpacing, static_cast<uint8_t>(1));
  ASSERT_EQ(bs.paragraphAlignment, static_cast<uint8_t>(0));
  ASSERT_EQ(bs.screenMargin, static_cast<uint8_t>(5));
  ASSERT_EQ(bs.orientation, static_cast<uint8_t>(0));
  ASSERT_FALSE(bs.useCustomSettings);
  return true;
}

// ---------------------------------------------------------------
// Serialize / deserialize round-trip
// ---------------------------------------------------------------

TEST(booksettings_save_load_roundtrip) {
  resetFs();

  // Save with custom values
  {
    BookSettings bs;
    bs.useCustomSettings = true;
    bs.lineSpacing = 2;
    bs.paragraphAlignment = 3;
    bs.screenMargin = 10;
    bs.darkMode = 1;
    bs.statusBarBattery = 0;
    bs.saveToFile("/tmp/book1");
  }

  // Load into fresh struct
  {
    BookSettings bs;
    bool loaded = bs.loadFromFile("/tmp/book1");
    ASSERT_TRUE(loaded);
    ASSERT_EQ(bs.lineSpacing, static_cast<uint8_t>(2));
    ASSERT_EQ(bs.paragraphAlignment, static_cast<uint8_t>(3));
    ASSERT_EQ(bs.screenMargin, static_cast<uint8_t>(10));
    ASSERT_EQ(bs.darkMode, static_cast<uint8_t>(1));
    ASSERT_EQ(bs.statusBarBattery, static_cast<uint8_t>(0));
    ASSERT_TRUE(bs.useCustomSettings);
  }
  return true;
}

// ---------------------------------------------------------------
// Load from non-existent file returns false
// ---------------------------------------------------------------

TEST(booksettings_load_nonexistent) {
  resetFs();
  BookSettings bs;
  bool loaded = bs.loadFromFile("/tmp/no_such_book");
  ASSERT_FALSE(loaded);
  return true;
}

// ---------------------------------------------------------------
// Save without useCustomSettings is no-op
// ---------------------------------------------------------------

TEST(booksettings_save_noop_without_custom) {
  resetFs();
  BookSettings bs;
  bs.saveToFile("/tmp/book1");
  ASSERT_TRUE(MockFs::files().find("/tmp/book1/settings.bin") == MockFs::files().end());
  return true;
}

// ---------------------------------------------------------------
// markCustomized
// ---------------------------------------------------------------

TEST(booksettings_mark_customized) {
  BookSettings bs;
  ASSERT_FALSE(bs.useCustomSettings);
  bs.markCustomized();
  ASSERT_TRUE(bs.useCustomSettings);
  return true;
}

// ---------------------------------------------------------------
// loadFromGlobal copies SETTINGS fields
// ---------------------------------------------------------------

TEST(booksettings_load_from_global) {
  SETTINGS.lineSpacing = 2;
  SETTINGS.darkMode = 1;
  SETTINGS.screenMargin = 12;

  BookSettings bs;
  bs.loadFromGlobal();

  ASSERT_EQ(bs.lineSpacing, static_cast<uint8_t>(2));
  ASSERT_EQ(bs.darkMode, static_cast<uint8_t>(1));
  ASSERT_EQ(bs.screenMargin, static_cast<uint8_t>(12));
  ASSERT_FALSE(bs.useCustomSettings);

  // Restore defaults
  SETTINGS.lineSpacing = 1;
  SETTINGS.darkMode = 0;
  SETTINGS.screenMargin = 5;
  return true;
}

// ---------------------------------------------------------------
// applyToGlobal pushes fields into SETTINGS
// ---------------------------------------------------------------

TEST(booksettings_apply_to_global) {
  BookSettings bs;
  bs.lineSpacing = 0;
  bs.darkMode = 1;
  bs.screenMargin = 15;
  bs.applyToGlobal();

  ASSERT_EQ(SETTINGS.lineSpacing, static_cast<uint8_t>(0));
  ASSERT_EQ(SETTINGS.darkMode, static_cast<uint8_t>(1));
  ASSERT_EQ(SETTINGS.screenMargin, static_cast<uint8_t>(15));

  // Restore
  SETTINGS.lineSpacing = 1;
  SETTINGS.darkMode = 0;
  SETTINGS.screenMargin = 5;
  return true;
}

// ---------------------------------------------------------------
// Corrupt file (wrong version) returns false
// ---------------------------------------------------------------

TEST(booksettings_corrupt_version) {
  resetFs();
  // Write a file with wrong version byte
  std::vector<uint8_t> bad(20, 0);
  bad[0] = 99;  // wrong version
  MockFs::files()["/tmp/corrupt/settings.bin"] = bad;

  BookSettings bs;
  bool loaded = bs.loadFromFile("/tmp/corrupt");
  ASSERT_FALSE(loaded);
  return true;
}

int main() { return RUN_ALL_TESTS(); }
