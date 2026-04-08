#pragma once

#include <HalStorage.h>
#include <Logging.h>

#include <cstdint>
#include <string>

#include "CrossPointSettings.h"

// Per-book reader settings, persisted to SD card.
// Falls back to global CrossPointSettings when no per-book file exists.
struct BookSettings {
  // Reader settings (subset of CrossPointSettings)
  uint8_t lineSpacing = 1;         // NORMAL
  uint8_t paragraphAlignment = 0;  // JUSTIFIED
  uint8_t extraParagraphSpacing = 0;
  uint8_t textAntiAliasing = 1;
  uint8_t hyphenationEnabled = 0;
  uint8_t screenMargin = 5;
  uint8_t orientation = 0;       // PORTRAIT
  uint8_t refreshFrequency = 3;  // REFRESH_15 enum index
  uint8_t characterWrap = 1;
  uint8_t paragraphIndent = 0;
  uint8_t textDarkness = 0;
  uint8_t darkMode = 0;

  // Status bar (per-book override of global status bar settings)
  uint8_t statusBarChapterPageCount = 1;
  uint8_t statusBarBookProgressPercentage = 1;
  uint8_t statusBarProgressBar = 0;
  uint8_t statusBarProgressBarThickness = 1;
  uint8_t statusBarTitle = 1;
  uint8_t statusBarBattery = 1;

  bool useCustomSettings = false;

  static constexpr uint8_t FILE_VERSION = 2;
  static constexpr size_t FIELD_COUNT = 18;  // 12 reader + 6 status bar

  // Load from per-book file. Returns true if loaded successfully.
  bool loadFromFile(const std::string& bookCachePath) {
    std::string path = bookCachePath + "/settings.bin";
    FsFile f;
    if (!Storage.openFileForRead("BST", path, f)) {
      return false;
    }

    uint8_t data[FIELD_COUNT + 1];  // +1 for version
    size_t bytesRead = f.read(data, sizeof(data));
    f.close();

    if (bytesRead < FIELD_COUNT + 1 || data[0] != FILE_VERSION) {
      return false;
    }

    size_t i = 1;
    lineSpacing = data[i++];
    paragraphAlignment = data[i++];
    extraParagraphSpacing = data[i++];
    textAntiAliasing = data[i++];
    hyphenationEnabled = data[i++];
    screenMargin = data[i++];
    orientation = data[i++];
    refreshFrequency = data[i++];
    characterWrap = data[i++];
    paragraphIndent = data[i++];
    textDarkness = data[i++];
    darkMode = data[i++];
    statusBarChapterPageCount = data[i++];
    statusBarBookProgressPercentage = data[i++];
    statusBarProgressBar = data[i++];
    statusBarProgressBarThickness = data[i++];
    statusBarTitle = data[i++];
    statusBarBattery = data[i++];

    useCustomSettings = true;
    LOG_DBG("BST", "Loaded per-book settings from %s", path.c_str());
    return true;
  }

  // Save to per-book file.
  void saveToFile(const std::string& bookCachePath) const {
    if (!useCustomSettings) return;

    std::string path = bookCachePath + "/settings.bin";
    FsFile f;
    if (!Storage.openFileForWrite("BST", path, f)) {
      LOG_ERR("BST", "Failed to save per-book settings");
      return;
    }

    uint8_t data[FIELD_COUNT + 1];
    size_t i = 0;
    data[i++] = FILE_VERSION;
    data[i++] = lineSpacing;
    data[i++] = paragraphAlignment;
    data[i++] = extraParagraphSpacing;
    data[i++] = textAntiAliasing;
    data[i++] = hyphenationEnabled;
    data[i++] = screenMargin;
    data[i++] = orientation;
    data[i++] = refreshFrequency;
    data[i++] = characterWrap;
    data[i++] = paragraphIndent;
    data[i++] = textDarkness;
    data[i++] = darkMode;
    data[i++] = statusBarChapterPageCount;
    data[i++] = statusBarBookProgressPercentage;
    data[i++] = statusBarProgressBar;
    data[i++] = statusBarProgressBarThickness;
    data[i++] = statusBarTitle;
    data[i++] = statusBarBattery;

    f.write(data, sizeof(data));
    f.close();
    LOG_DBG("BST", "Saved per-book settings");
  }

  // Initialize from global settings (fallback when no per-book file).
  void loadFromGlobal() {
    lineSpacing = SETTINGS.lineSpacing;
    paragraphAlignment = SETTINGS.paragraphAlignment;
    extraParagraphSpacing = SETTINGS.extraParagraphSpacing;
    textAntiAliasing = SETTINGS.textAntiAliasing;
    hyphenationEnabled = SETTINGS.hyphenationEnabled;
    screenMargin = SETTINGS.screenMargin;
    orientation = SETTINGS.orientation;
    refreshFrequency = SETTINGS.refreshFrequency;
    characterWrap = SETTINGS.characterWrap;
    paragraphIndent = SETTINGS.paragraphIndent;
    textDarkness = SETTINGS.textDarkness;
    darkMode = SETTINGS.darkMode;
    statusBarChapterPageCount = SETTINGS.statusBarChapterPageCount;
    statusBarBookProgressPercentage = SETTINGS.statusBarBookProgressPercentage;
    statusBarProgressBar = SETTINGS.statusBarProgressBar;
    statusBarProgressBarThickness = SETTINGS.statusBarProgressBarThickness;
    statusBarTitle = SETTINGS.statusBarTitle;
    statusBarBattery = SETTINGS.statusBarBattery;
    useCustomSettings = false;
  }

  // Apply these settings to the global SETTINGS singleton (for rendering).
  // Call this when entering a book, restore original on exit.
  void applyToGlobal() const {
    SETTINGS.lineSpacing = lineSpacing;
    SETTINGS.paragraphAlignment = paragraphAlignment;
    SETTINGS.extraParagraphSpacing = extraParagraphSpacing;
    SETTINGS.textAntiAliasing = textAntiAliasing;
    SETTINGS.hyphenationEnabled = hyphenationEnabled;
    SETTINGS.screenMargin = screenMargin;
    SETTINGS.orientation = orientation;
    SETTINGS.refreshFrequency = refreshFrequency;
    SETTINGS.characterWrap = characterWrap;
    SETTINGS.paragraphIndent = paragraphIndent;
    SETTINGS.textDarkness = textDarkness;
    SETTINGS.darkMode = darkMode;
    SETTINGS.statusBarChapterPageCount = statusBarChapterPageCount;
    SETTINGS.statusBarBookProgressPercentage = statusBarBookProgressPercentage;
    SETTINGS.statusBarProgressBar = statusBarProgressBar;
    SETTINGS.statusBarProgressBarThickness = statusBarProgressBarThickness;
    SETTINGS.statusBarTitle = statusBarTitle;
    SETTINGS.statusBarBattery = statusBarBattery;
  }

  // Mark as customized (call when user changes any setting in-reader).
  void markCustomized() { useCustomSettings = true; }
};
