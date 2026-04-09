#pragma once
// ---------------------------------------------------------------
// HalStorage / HalFile mock for host-based unit tests.
// Uses an in-memory std::map<std::string, std::vector<uint8_t>>
// to simulate SD card file I/O.
// ---------------------------------------------------------------

#include <common/FsApiConstants.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Arduino.h"  // for Print, String

// Forward declaration
class HalFile;

// ---------------------------------------------------------------
// In-memory filesystem backing store (global, reset between tests)
// ---------------------------------------------------------------
namespace MockFs {
std::map<std::string, std::vector<uint8_t>>& files();
void reset();
}  // namespace MockFs

// ---------------------------------------------------------------
// HalFile — in-memory file handle
// ---------------------------------------------------------------
class HalFile : public Print {
 public:
  HalFile();
  ~HalFile();
  HalFile(HalFile&&);
  HalFile& operator=(HalFile&&);
  HalFile(const HalFile&) = delete;
  HalFile& operator=(const HalFile&) = delete;

  void flush() override {}
  size_t getName(char* /*name*/, size_t /*len*/) { return 0; }
  size_t size();
  size_t fileSize() { return size(); }
  bool seek(size_t pos);
  bool seekCur(int64_t offset);
  bool seekSet(size_t offset) { return seek(offset); }
  int available() const;
  size_t position() const;
  int read(void* buf, size_t count);
  int read();
  size_t write(const void* buf, size_t count);
  size_t write(uint8_t b) override;
  bool rename(const char* /*newPath*/) { return false; }
  bool isDirectory() const { return false; }
  void rewindDirectory() {}
  bool close();
  HalFile openNextFile();
  bool isOpen() const;
  operator bool() const { return isOpen(); }

  // Internal: bind to a path and mode
  void bind(const std::string& path, bool writable);

 private:
  std::string path_;
  bool writable_ = false;
  bool open_ = false;
  size_t pos_ = 0;
  // Write buffer (flushed to MockFs on close)
  std::vector<uint8_t> writeBuf_;
};

// ---------------------------------------------------------------
// HalStorage singleton
// ---------------------------------------------------------------
class HalStorage {
 public:
  HalStorage() = default;
  bool begin() { return true; }
  bool ready() const { return true; }

  bool openFileForRead(const char* moduleName, const char* path, HalFile& file);
  bool openFileForRead(const char* moduleName, const std::string& path, HalFile& file);
  bool openFileForRead(const char* moduleName, const String& path, HalFile& file);
  bool openFileForWrite(const char* moduleName, const char* path, HalFile& file);
  bool openFileForWrite(const char* moduleName, const std::string& path, HalFile& file);
  bool openFileForWrite(const char* moduleName, const String& path, HalFile& file);

  HalFile open(const char* path, oflag_t oflag = O_RDONLY);
  bool mkdir(const char* path, bool pFlag = true);
  bool exists(const char* path);
  bool remove(const char* path);
  bool rename(const char* oldPath, const char* newPath);
  bool rmdir(const char* /*path*/) { return true; }
  bool ensureDirectoryExists(const char* /*path*/) { return true; }
  bool removeDir(const char* /*path*/) { return true; }
  std::vector<String> listFiles(const char* /*path*/ = "/", int /*maxFiles*/ = 200) { return {}; }
  String readFile(const char* /*path*/) { return String(""); }
  bool readFileToStream(const char* /*path*/, Print& /*out*/, size_t /*chunk*/ = 256) { return false; }
  size_t readFileToBuffer(const char* /*path*/, char* /*buf*/, size_t /*sz*/, size_t /*max*/ = 0) { return 0; }
  bool writeFile(const char* /*path*/, const String& /*content*/) { return true; }

  static HalStorage& getInstance() { return instance; }

 private:
  static HalStorage instance;
};

#define Storage HalStorage::getInstance()

// Map FsFile -> HalFile so production headers compile
#ifndef HAL_STORAGE_IMPL
using FsFile = HalFile;
#endif

// Suppress upstream SdMan references
#ifdef SdMan
#undef SdMan
#endif
