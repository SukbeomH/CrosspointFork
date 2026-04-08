#pragma once

#include <HalStorage.h>
#include <Logging.h>

#include <cstdint>
#include <string>

// Lightweight per-book reading statistics.
// Tracks total reading time and session count.
// Persisted to .crosspoint/epub_<hash>/statistics.bin
struct ReadingStats {
  uint32_t totalReadingMs = 0;    // Total reading time in milliseconds
  uint16_t sessionCount = 0;      // Number of reading sessions (3+ minutes)
  uint32_t lastSessionMs = 0;     // Duration of last session
  uint32_t lastReadTimestamp = 0;  // Unix timestamp of last read (0 if unknown)

  static constexpr uint8_t FILE_VERSION = 1;
  static constexpr uint32_t MIN_SESSION_MS = 3 * 60 * 1000;  // 3 minutes minimum

  bool loadFromFile(const std::string& cachePath) {
    std::string path = cachePath + "/statistics.bin";
    FsFile f;
    if (!Storage.openFileForRead("RST", path, f)) return false;

    uint8_t data[13];  // 1 version + 4 totalMs + 2 sessions + 4 lastSessionMs + 2 reserved
    size_t bytesRead = f.read(data, sizeof(data));
    f.close();

    if (bytesRead < 11 || data[0] != FILE_VERSION) return false;

    size_t i = 1;
    totalReadingMs = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);
    i += 4;
    sessionCount = data[i] | (data[i + 1] << 8);
    i += 2;
    lastSessionMs = data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);

    LOG_DBG("RST", "Loaded stats: %lu ms, %d sessions", totalReadingMs, sessionCount);
    return true;
  }

  void saveToFile(const std::string& cachePath) const {
    std::string path = cachePath + "/statistics.bin";
    FsFile f;
    if (!Storage.openFileForWrite("RST", path, f)) {
      LOG_ERR("RST", "Failed to save reading stats");
      return;
    }

    uint8_t data[11];
    size_t i = 0;
    data[i++] = FILE_VERSION;
    data[i++] = totalReadingMs & 0xFF;
    data[i++] = (totalReadingMs >> 8) & 0xFF;
    data[i++] = (totalReadingMs >> 16) & 0xFF;
    data[i++] = (totalReadingMs >> 24) & 0xFF;
    data[i++] = sessionCount & 0xFF;
    data[i++] = (sessionCount >> 8) & 0xFF;
    data[i++] = lastSessionMs & 0xFF;
    data[i++] = (lastSessionMs >> 8) & 0xFF;
    data[i++] = (lastSessionMs >> 16) & 0xFF;
    data[i++] = (lastSessionMs >> 24) & 0xFF;

    f.write(data, sizeof(data));
    f.close();
    LOG_DBG("RST", "Saved stats: %lu ms, %d sessions", totalReadingMs, sessionCount);
  }
};

// Tracks a single reading session in-progress.
// Call start() when entering reader, stop() when exiting.
// Accumulates time via tick() called from the main loop.
class ReadingSessionTracker {
 public:
  void start() {
    active = true;
    sessionStartMs = millis();
    lastTickMs = sessionStartMs;
    accumulatedMs = 0;
  }

  void tick() {
    if (!active) return;
    unsigned long now = millis();
    unsigned long delta = now - lastTickMs;
    // Cap delta to 10 seconds to handle sleep/wake gaps
    if (delta > 10000) delta = 0;
    accumulatedMs += delta;
    lastTickMs = now;
  }

  uint32_t stop() {
    if (!active) return 0;
    active = false;
    tick();  // Final accumulation
    return accumulatedMs;
  }

  bool isActive() const { return active; }
  uint32_t getElapsedMs() const { return accumulatedMs; }

 private:
  bool active = false;
  unsigned long sessionStartMs = 0;
  unsigned long lastTickMs = 0;
  uint32_t accumulatedMs = 0;
};
