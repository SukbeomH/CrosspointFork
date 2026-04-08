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
  uint8_t lineSpacing = 1;           // NORMAL
  uint8_t paragraphAlignment = 0;    // JUSTIFIED
  uint8_t extraParagraphSpacing = 0;
  uint8_t textAntiAliasing = 1;
  uint8_t hyphenationEnabled = 0;
  uint8_t screenMargin = 5;
  uint8_t orientation = 0;  // PORTRAIT
  uint8_t refreshFrequency = 3;  // REFRESH_15 enum index
  uint8_t characterWrap = 1;
  uint8_t paragraphIndent = 0;
  uint8_t textDarkness = 0;
  uint8_t darkMode = 0;

  bool useCustomSettings = false;

  static constexpr uint8_t FILE_VERSION = 1;
  static constexpr size_t FIELD_COUNT = 12;

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
  }

  // Mark as customized (call when user changes any setting in-reader).
  void markCustomized() { useCustomSettings = true; }
};
