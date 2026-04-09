#include <algorithm>
#include <cstring>

#include "HalStorage.h"

// ---------------------------------------------------------------
// In-memory filesystem
// ---------------------------------------------------------------
namespace MockFs {
static std::map<std::string, std::vector<uint8_t>> fsData;

std::map<std::string, std::vector<uint8_t>>& files() { return fsData; }

void reset() { fsData.clear(); }
}  // namespace MockFs

// ---------------------------------------------------------------
// HalStorage singleton
// ---------------------------------------------------------------
HalStorage HalStorage::instance;

bool HalStorage::openFileForRead(const char* /*moduleName*/, const char* path, HalFile& file) {
  auto it = MockFs::files().find(path);
  if (it == MockFs::files().end()) return false;
  file.bind(path, false);
  return true;
}

bool HalStorage::openFileForRead(const char* moduleName, const std::string& path, HalFile& file) {
  return openFileForRead(moduleName, path.c_str(), file);
}

bool HalStorage::openFileForRead(const char* moduleName, const String& path, HalFile& file) {
  return openFileForRead(moduleName, path.c_str(), file);
}

bool HalStorage::openFileForWrite(const char* /*moduleName*/, const char* path, HalFile& file) {
  // Create empty entry if not present
  if (MockFs::files().find(path) == MockFs::files().end()) {
    MockFs::files()[path] = {};
  }
  file.bind(path, true);
  return true;
}

bool HalStorage::openFileForWrite(const char* moduleName, const std::string& path, HalFile& file) {
  return openFileForWrite(moduleName, path.c_str(), file);
}

bool HalStorage::openFileForWrite(const char* moduleName, const String& path, HalFile& file) {
  return openFileForWrite(moduleName, path.c_str(), file);
}

HalFile HalStorage::open(const char* path, oflag_t oflag) {
  HalFile f;
  if (oflag & O_WRONLY || oflag & O_RDWR) {
    openFileForWrite("", path, f);
  } else {
    openFileForRead("", path, f);
  }
  return f;
}

bool HalStorage::mkdir(const char* /*path*/, bool /*pFlag*/) { return true; }
bool HalStorage::exists(const char* path) { return MockFs::files().count(path) > 0; }

bool HalStorage::remove(const char* path) {
  auto it = MockFs::files().find(path);
  if (it != MockFs::files().end()) {
    MockFs::files().erase(it);
    return true;
  }
  return false;
}

bool HalStorage::rename(const char* oldPath, const char* newPath) {
  auto it = MockFs::files().find(oldPath);
  if (it == MockFs::files().end()) return false;
  MockFs::files()[newPath] = std::move(it->second);
  MockFs::files().erase(it);
  return true;
}

// ---------------------------------------------------------------
// HalFile implementation
// ---------------------------------------------------------------
HalFile::HalFile() = default;
HalFile::~HalFile() { close(); }
HalFile::HalFile(HalFile&& o)
    : path_(std::move(o.path_)),
      writable_(o.writable_),
      open_(o.open_),
      pos_(o.pos_),
      writeBuf_(std::move(o.writeBuf_)) {
  o.open_ = false;
}
HalFile& HalFile::operator=(HalFile&& o) {
  if (this != &o) {
    close();
    path_ = std::move(o.path_);
    writable_ = o.writable_;
    open_ = o.open_;
    pos_ = o.pos_;
    writeBuf_ = std::move(o.writeBuf_);
    o.open_ = false;
  }
  return *this;
}

void HalFile::bind(const std::string& path, bool writable) {
  path_ = path;
  writable_ = writable;
  open_ = true;
  pos_ = 0;
  writeBuf_.clear();
}

bool HalFile::isOpen() const { return open_; }

size_t HalFile::size() {
  if (!open_) return 0;
  if (writable_) return writeBuf_.size();
  auto it = MockFs::files().find(path_);
  return it != MockFs::files().end() ? it->second.size() : 0;
}

bool HalFile::seek(size_t pos) {
  pos_ = pos;
  return true;
}

bool HalFile::seekCur(int64_t offset) {
  int64_t newPos = static_cast<int64_t>(pos_) + offset;
  int64_t maxPos = static_cast<int64_t>(size());
  if (newPos < 0) {
    newPos = 0;
  } else if (newPos > maxPos) {
    newPos = maxPos;
  }
  pos_ = static_cast<size_t>(newPos);
  return true;
}

int HalFile::available() const {
  if (!open_) return 0;
  auto it = MockFs::files().find(path_);
  if (it == MockFs::files().end()) return 0;
  size_t sz = it->second.size();
  return pos_ < sz ? static_cast<int>(sz - pos_) : 0;
}

size_t HalFile::position() const { return pos_; }

int HalFile::read(void* buf, size_t count) {
  if (!open_) return -1;
  auto it = MockFs::files().find(path_);
  if (it == MockFs::files().end()) return -1;
  const auto& data = it->second;
  if (pos_ >= data.size()) return 0;
  size_t avail = data.size() - pos_;
  size_t toRead = std::min(count, avail);
  std::memcpy(buf, data.data() + pos_, toRead);
  pos_ += toRead;
  return static_cast<int>(toRead);
}

int HalFile::read() {
  uint8_t b;
  if (read(&b, 1) == 1) return b;
  return -1;
}

size_t HalFile::write(const void* buf, size_t count) {
  if (!open_ || !writable_) return 0;
  const uint8_t* p = static_cast<const uint8_t*>(buf);
  writeBuf_.insert(writeBuf_.end(), p, p + count);
  return count;
}

size_t HalFile::write(uint8_t b) { return write(&b, 1); }

bool HalFile::close() {
  if (!open_) return false;
  if (writable_) {
    MockFs::files()[path_] = std::move(writeBuf_);
  }
  open_ = false;
  writeBuf_.clear();
  pos_ = 0;
  return true;
}

HalFile HalFile::openNextFile() { return HalFile(); }
