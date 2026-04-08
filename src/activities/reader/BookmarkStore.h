#pragma once

#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

// Stores per-book bookmarks in the book's cache directory on SD card.
class BookmarkStore {
 public:
  struct Bookmark {
    uint16_t spineIndex = 0;
    uint16_t pageNumber = 0;
  };

  void load(const std::string& cachePath) {
    bookmarks.clear();
    dirty = false;
    storagePath = cachePath.empty() ? "" : (cachePath + "/bookmarks.bin");
    if (storagePath.empty()) return;

    FsFile file;
    if (!Storage.openFileForRead("BKM", storagePath, file)) return;

    uint8_t version = 0;
    if (file.read(reinterpret_cast<uint8_t*>(&version), sizeof(version)) != sizeof(version) ||
        version != FILE_VERSION) {
      file.close();
      return;
    }

    uint16_t count = 0;
    if (file.read(reinterpret_cast<uint8_t*>(&count), sizeof(count)) != sizeof(count) || count > MAX_BOOKMARKS) {
      file.close();
      return;
    }

    bookmarks.reserve(count);
    for (uint16_t i = 0; i < count; i++) {
      Bookmark bm;
      if (file.read(reinterpret_cast<uint8_t*>(&bm.spineIndex), sizeof(bm.spineIndex)) != sizeof(bm.spineIndex) ||
          file.read(reinterpret_cast<uint8_t*>(&bm.pageNumber), sizeof(bm.pageNumber)) != sizeof(bm.pageNumber)) {
        bookmarks.clear();
        file.close();
        return;
      }
      bookmarks.push_back(bm);
    }
    file.close();
    LOG_DBG("BKM", "Loaded %d bookmarks from %s", bookmarks.size(), storagePath.c_str());
  }

  void save() {
    if (!dirty || storagePath.empty()) return;

    FsFile file;
    if (!Storage.openFileForWrite("BKM", storagePath, file)) {
      LOG_ERR("BKM", "Failed to save bookmarks");
      return;
    }

    const uint16_t count = static_cast<uint16_t>(bookmarks.size());
    file.write(reinterpret_cast<const uint8_t*>(&FILE_VERSION), sizeof(FILE_VERSION));
    file.write(reinterpret_cast<const uint8_t*>(&count), sizeof(count));

    for (const auto& bm : bookmarks) {
      file.write(reinterpret_cast<const uint8_t*>(&bm.spineIndex), sizeof(bm.spineIndex));
      file.write(reinterpret_cast<const uint8_t*>(&bm.pageNumber), sizeof(bm.pageNumber));
    }
    file.close();
    dirty = false;
    LOG_DBG("BKM", "Saved %d bookmarks", count);
  }

  // Returns true if added, false if removed
  bool toggle(uint16_t spineIndex, uint16_t pageNumber) {
    auto it = find(spineIndex, pageNumber);
    if (it != bookmarks.end()) {
      bookmarks.erase(it);
      dirty = true;
      return false;
    }
    if (bookmarks.size() >= MAX_BOOKMARKS) {
      LOG_ERR("BKM", "Max bookmarks reached (%d)", MAX_BOOKMARKS);
      return false;
    }
    bookmarks.push_back({spineIndex, pageNumber});
    dirty = true;
    return true;
  }

  [[nodiscard]] bool has(uint16_t spineIndex, uint16_t pageNumber) const {
    return std::any_of(bookmarks.begin(), bookmarks.end(), [spineIndex, pageNumber](const Bookmark& bm) {
      return bm.spineIndex == spineIndex && bm.pageNumber == pageNumber;
    });
  }

  [[nodiscard]] const std::vector<Bookmark>& getAll() const { return bookmarks; }
  [[nodiscard]] bool isEmpty() const { return bookmarks.empty(); }

 private:
  static constexpr uint8_t FILE_VERSION = 1;
  static constexpr uint16_t MAX_BOOKMARKS = 100;

  std::vector<Bookmark> bookmarks;
  std::string storagePath;
  bool dirty = false;

  std::vector<Bookmark>::iterator find(uint16_t spineIndex, uint16_t pageNumber) {
    return std::find_if(bookmarks.begin(), bookmarks.end(), [spineIndex, pageNumber](const Bookmark& bm) {
      return bm.spineIndex == spineIndex && bm.pageNumber == pageNumber;
    });
  }
};
