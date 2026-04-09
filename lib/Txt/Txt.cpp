#include "Txt.h"

#include <FsHelpers.h>
#include <JpegToBmpConverter.h>
#include <Logging.h>
#include <Serialization.h>

Txt::Txt(std::string path, std::string cacheBasePath)
    : filepath(std::move(path)), cacheBasePath(std::move(cacheBasePath)) {
  // Generate cache path from file path hash
  const size_t hash = std::hash<std::string>{}(filepath);
  cachePath = this->cacheBasePath + "/txt_" + std::to_string(hash);
}

bool Txt::load() {
  if (loaded) {
    return true;
  }

  if (!Storage.exists(filepath.c_str())) {
    LOG_ERR("TXT", "File does not exist: %s", filepath.c_str());
    return false;
  }

  FsFile file;
  if (!Storage.openFileForRead("TXT", filepath, file)) {
    LOG_ERR("TXT", "Failed to open file: %s", filepath.c_str());
    return false;
  }

  fileSize = file.size();
  file.close();

  loaded = true;
  LOG_DBG("TXT", "Loaded TXT file: %s (%zu bytes)", filepath.c_str(), fileSize);
  return true;
}

std::string Txt::getTitle() const {
  // Extract filename without path and extension
  size_t lastSlash = filepath.find_last_of('/');
  std::string filename = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;

  // Remove .txt extension
  if (FsHelpers::hasTxtExtension(filename)) {
    filename = filename.substr(0, filename.length() - 4);
  }

  return filename;
}

void Txt::setupCacheDir() const {
  if (!Storage.exists(cacheBasePath.c_str())) {
    Storage.mkdir(cacheBasePath.c_str());
  }
  if (!Storage.exists(cachePath.c_str())) {
    Storage.mkdir(cachePath.c_str());
  }
}

std::string Txt::findCoverImage() const {
  // Get the folder containing the txt file
  size_t lastSlash = filepath.find_last_of('/');
  std::string folder = (lastSlash != std::string::npos) ? filepath.substr(0, lastSlash) : "";
  if (folder.empty()) {
    folder = "/";
  }

  // Get the base filename without extension (e.g., "mybook" from "/books/mybook.txt")
  std::string baseName = getTitle();

  // Image extensions to try
  const char* extensions[] = {".bmp", ".jpg", ".jpeg", ".png", ".BMP", ".JPG", ".JPEG", ".PNG"};

  // First priority: look for image with same name as txt file (e.g., mybook.jpg)
  for (const auto& ext : extensions) {
    std::string coverPath = folder + "/" + baseName + ext;
    if (Storage.exists(coverPath.c_str())) {
      LOG_DBG("TXT", "Found matching cover image: %s", coverPath.c_str());
      return coverPath;
    }
  }

  // Fallback: look for cover image files
  const char* coverNames[] = {"cover", "Cover", "COVER"};
  for (const auto& name : coverNames) {
    for (const auto& ext : extensions) {
      std::string coverPath = folder + "/" + std::string(name) + ext;
      if (Storage.exists(coverPath.c_str())) {
        LOG_DBG("TXT", "Found fallback cover image: %s", coverPath.c_str());
        return coverPath;
      }
    }
  }

  return "";
}

std::string Txt::getCoverBmpPath() const { return cachePath + "/cover.bmp"; }

bool Txt::generateCoverBmp() const {
  // Already generated, return true
  if (Storage.exists(getCoverBmpPath().c_str())) {
    return true;
  }

  std::string coverImagePath = findCoverImage();
  if (coverImagePath.empty()) {
    LOG_DBG("TXT", "No cover image found for TXT file");
    return false;
  }

  // Setup cache directory
  setupCacheDir();

  if (FsHelpers::hasBmpExtension(coverImagePath)) {
    // Copy BMP file to cache
    LOG_DBG("TXT", "Copying BMP cover image to cache");
    FsFile src, dst;
    if (!Storage.openFileForRead("TXT", coverImagePath, src)) {
      return false;
    }
    if (!Storage.openFileForWrite("TXT", getCoverBmpPath(), dst)) {
      src.close();
      return false;
    }
    uint8_t buffer[1024];
    while (src.available()) {
      size_t bytesRead = src.read(buffer, sizeof(buffer));
      dst.write(buffer, bytesRead);
    }
    src.close();
    dst.close();
    LOG_DBG("TXT", "Copied BMP cover to cache");
    return true;
  } else if (FsHelpers::hasJpgExtension(coverImagePath)) {
    // Convert JPG/JPEG to BMP (same approach as Epub)
    LOG_DBG("TXT", "Generating BMP from JPG cover image");
    FsFile coverJpg, coverBmp;
    if (!Storage.openFileForRead("TXT", coverImagePath, coverJpg)) {
      return false;
    }
    if (!Storage.openFileForWrite("TXT", getCoverBmpPath(), coverBmp)) {
      coverJpg.close();
      return false;
    }
    const bool success = JpegToBmpConverter::jpegFileToBmpStream(coverJpg, coverBmp);
    coverJpg.close();
    coverBmp.close();

    if (!success) {
      LOG_ERR("TXT", "Failed to generate BMP from JPG cover image");
      Storage.remove(getCoverBmpPath().c_str());
    } else {
      LOG_DBG("TXT", "Generated BMP from JPG cover image");
    }
    return success;
  }

  // PNG files are not supported (would need a PNG decoder)
  LOG_ERR("TXT", "Cover image format not supported (only BMP/JPG/JPEG)");
  return false;
}

bool Txt::readContent(uint8_t* buffer, size_t offset, size_t length) const {
  if (!loaded) {
    return false;
  }

  FsFile file;
  if (!Storage.openFileForRead("TXT", filepath, file)) {
    return false;
  }

  if (!file.seek(offset)) {
    file.close();
    return false;
  }

  size_t bytesRead = file.read(buffer, length);
  file.close();

  return bytesRead > 0;
}

// ---------------------------------------------------------------------------
// Chapter Detection — streaming scan for multilingual chapter patterns
// ---------------------------------------------------------------------------

namespace {

constexpr size_t LINE_BUF_SIZE = 128;
constexpr int MAX_CHAPTER_TITLE_LEN = 60;  // Lines longer than this are not chapter headings

// Chapter cache format
constexpr uint32_t CHAPTER_CACHE_MAGIC = 0x54584348;  // "TXCH"
constexpr uint8_t CHAPTER_CACHE_VERSION = 1;

/// Check if a byte sequence contains digits (ASCII 0-9) or fullwidth digits.
/// Returns true if at least one digit is found between positions start and end.
bool hasDigitInRange(const char* s, int start, int end) {
  for (int i = start; i < end; i++) {
    uint8_t b = static_cast<uint8_t>(s[i]);
    // ASCII digits
    if (b >= '0' && b <= '9') return true;
    // Fullwidth digits: U+FF10-U+FF19 → UTF-8: 0xEF 0xBC 0x90-0x99
    if (b == 0xEF && i + 2 < end) {
      if (static_cast<uint8_t>(s[i + 1]) == 0xBC) {
        uint8_t b3 = static_cast<uint8_t>(s[i + 2]);
        if (b3 >= 0x90 && b3 <= 0x99) return true;
      }
    }
  }
  return false;
}

/// Match Korean chapter pattern: "제" + digits + suffix(장/화/편/회/부)
/// "제" = U+C81C → UTF-8: 0xEC 0xA0 0x9C
/// "장" = U+C7A5 → 0xEC 0x9E 0xA5
/// "화" = U+D654 → 0xED 0x99 0x94
/// "편" = U+D3B8 → 0xED 0x8E 0xB8
/// "회" = U+D68C → 0xED 0x9A 0x8C
/// "부" = U+BD80 → 0xEB 0xB6 0x80
bool matchKoreanChapter(const char* s, int len) {
  if (len < 9) return false;  // At minimum: 제(3) + digit(1) + 장(3) = 7, but be safe
  for (int i = 0; i <= len - 7; i++) {
    uint8_t b0 = static_cast<uint8_t>(s[i]);
    uint8_t b1 = static_cast<uint8_t>(s[i + 1]);
    uint8_t b2 = static_cast<uint8_t>(s[i + 2]);
    // Check "제" (0xEC 0xA0 0x9C)
    if (b0 == 0xEC && b1 == 0xA0 && b2 == 0x9C) {
      // Search for suffix within the next ~20 bytes
      int searchEnd = (i + 24 < len) ? i + 24 : len;
      for (int j = i + 3; j <= searchEnd - 3; j++) {
        uint8_t s0 = static_cast<uint8_t>(s[j]);
        uint8_t s1 = static_cast<uint8_t>(s[j + 1]);
        uint8_t s2 = static_cast<uint8_t>(s[j + 2]);
        bool isSuffix = (s0 == 0xEC && s1 == 0x9E && s2 == 0xA5) ||  // 장
                        (s0 == 0xED && s1 == 0x99 && s2 == 0x94) ||  // 화
                        (s0 == 0xED && s1 == 0x8E && s2 == 0xB8) ||  // 편
                        (s0 == 0xED && s1 == 0x9A && s2 == 0x8C) ||  // 회
                        (s0 == 0xEB && s1 == 0xB6 && s2 == 0x80);    // 부
        if (isSuffix && hasDigitInRange(s, i + 3, j)) {
          return true;
        }
      }
    }
  }
  return false;
}

/// Match Chinese chapter pattern: "第" + digits + suffix(章/节/回)
/// "第" = U+7B2C → UTF-8: 0xE7 0xAC 0xAC
/// "章" = U+7AE0 → 0xE7 0xAB 0xA0
/// "节" = U+8282 → 0xE8 0x8A 0x82
/// "回" = U+56DE → 0xE5 0x9B 0x9E
bool matchChineseChapter(const char* s, int len) {
  if (len < 9) return false;
  for (int i = 0; i <= len - 6; i++) {
    uint8_t b0 = static_cast<uint8_t>(s[i]);
    uint8_t b1 = static_cast<uint8_t>(s[i + 1]);
    uint8_t b2 = static_cast<uint8_t>(s[i + 2]);
    // Check "第" (0xE7 0xAC 0xAC)
    if (b0 == 0xE7 && b1 == 0xAC && b2 == 0xAC) {
      int searchEnd = (i + 24 < len) ? i + 24 : len;
      for (int j = i + 3; j <= searchEnd - 3; j++) {
        uint8_t s0 = static_cast<uint8_t>(s[j]);
        uint8_t s1 = static_cast<uint8_t>(s[j + 1]);
        uint8_t s2 = static_cast<uint8_t>(s[j + 2]);
        bool isSuffix = (s0 == 0xE7 && s1 == 0xAB && s2 == 0xA0) ||  // 章
                        (s0 == 0xE8 && s1 == 0x8A && s2 == 0x82) ||  // 节
                        (s0 == 0xE5 && s1 == 0x9B && s2 == 0x9E);    // 回
        if (isSuffix && hasDigitInRange(s, i + 3, j)) {
          return true;
        }
      }
    }
  }
  return false;
}

/// Match English chapter pattern: "Chapter " or "CHAPTER " followed by a digit.
/// Case-insensitive prefix match.
bool matchEnglishChapter(const char* s, int len) {
  if (len < 9) return false;  // "Chapter 1" = 9 chars minimum

  // Scan for "Chapter" or "CHAPTER" at start of meaningful content
  // Allow up to a few leading spaces/tabs
  int start = 0;
  while (start < len && (s[start] == ' ' || s[start] == '\t')) {
    start++;
    if (start > 4) return false;  // Too much leading whitespace
  }

  int remaining = len - start;
  if (remaining < 9) return false;

  const char* p = s + start;

  // Case-insensitive "chapter" check
  bool match = true;
  const char* target = "chapter";
  for (int i = 0; i < 7; i++) {
    char c = p[i];
    if (c >= 'A' && c <= 'Z') c += 32;  // tolower
    if (c != target[i]) {
      match = false;
      break;
    }
  }

  if (!match) return false;

  // Must be followed by space(s) then a digit
  int pos = 7;
  if (p[pos] != ' ' && p[pos] != '\t') return false;
  while (pos < remaining && (p[pos] == ' ' || p[pos] == '\t')) pos++;

  if (pos < remaining) {
    uint8_t b = static_cast<uint8_t>(p[pos]);
    if (b >= '0' && b <= '9') return true;
    // Roman numerals (I, V, X, L, C, D, M)
    if (b == 'I' || b == 'V' || b == 'X' || b == 'L' || b == 'C' || b == 'D' || b == 'M') return true;
    if (b == 'i' || b == 'v' || b == 'x' || b == 'l' || b == 'c' || b == 'd' || b == 'm') return true;
  }

  return false;
}

/// Check if a line matches any chapter pattern.
bool isChapterLine(const char* line, int len) {
  if (len <= 0 || len > MAX_CHAPTER_TITLE_LEN) return false;
  return matchKoreanChapter(line, len) || matchChineseChapter(line, len) || matchEnglishChapter(line, len);
}

/// Extract a displayable title from a line buffer, truncated to maxChars UTF-8 characters.
std::string extractTitle(const char* line, int len, int maxChars = 30) {
  int charCount = 0;
  int bytePos = 0;
  while (bytePos < len && charCount < maxChars) {
    uint8_t b = static_cast<uint8_t>(line[bytePos]);
    if (b < 0x80) {
      bytePos += 1;
    } else if (b < 0xE0) {
      bytePos += 2;
    } else if (b < 0xF0) {
      bytePos += 3;
    } else {
      bytePos += 4;
    }
    charCount++;
  }
  if (bytePos > len) bytePos = len;
  return std::string(line, bytePos);
}

}  // namespace

void Txt::detectChapters() {
  if (chaptersDetected) return;
  chaptersDetected = true;

  if (!loaded) return;

  // Try loading from cache first
  if (loadChapterCache()) {
    LOG_DBG("TXT", "Loaded %zu chapters from cache", chapters.size());
    return;
  }

  LOG_DBG("TXT", "Scanning for chapters in %s (%zu bytes)", filepath.c_str(), fileSize);

  FsFile file;
  if (!Storage.openFileForRead("TXT", filepath, file)) {
    LOG_ERR("TXT", "Failed to open file for chapter scan");
    return;
  }

  char lineBuf[LINE_BUF_SIZE];
  int bufLen = 0;
  size_t currentOffset = 0;
  size_t lineStartOffset = 0;
  bool skipBom = true;

  while (file.available()) {
    char c = file.read();
    currentOffset++;

    if (c == '\n' || c == '\r' || bufLen >= static_cast<int>(LINE_BUF_SIZE) - 1) {
      if (bufLen > 0) {
        // Skip UTF-8 BOM at start of file
        int start = 0;
        if (skipBom && bufLen >= 3) {
          if (static_cast<uint8_t>(lineBuf[0]) == 0xEF && static_cast<uint8_t>(lineBuf[1]) == 0xBB &&
              static_cast<uint8_t>(lineBuf[2]) == 0xBF) {
            start = 3;
            skipBom = false;
          }
        }

        int effectiveLen = bufLen - start;
        if (effectiveLen > 0 && isChapterLine(lineBuf + start, effectiveLen)) {
          TxtChapterInfo ch;
          ch.title = extractTitle(lineBuf + start, effectiveLen);
          ch.byteOffset = lineStartOffset;
          chapters.push_back(std::move(ch));
        }
      }

      bufLen = 0;
      lineStartOffset = currentOffset;
      continue;
    }

    lineBuf[bufLen++] = c;

    // Yield every 64KB to avoid watchdog issues on ESP32
    if (currentOffset % (64 * 1024) == 0) {
      delay(1);
    }
  }

  // Process last line if no trailing newline
  if (bufLen > 0 && isChapterLine(lineBuf, bufLen)) {
    TxtChapterInfo ch;
    ch.title = extractTitle(lineBuf, bufLen);
    ch.byteOffset = lineStartOffset;
    chapters.push_back(std::move(ch));
  }

  file.close();

  LOG_DBG("TXT", "Detected %zu chapters", chapters.size());

  // Cache results for next time
  if (!chapters.empty()) {
    saveChapterCache();
  }
}

void Txt::saveChapterCache() const {
  setupCacheDir();
  std::string path = cachePath + "/chapters.bin";
  FsFile f;
  if (!Storage.openFileForWrite("TXT", path, f)) {
    LOG_ERR("TXT", "Failed to save chapter cache");
    return;
  }

  serialization::writePod(f, CHAPTER_CACHE_MAGIC);
  serialization::writePod(f, CHAPTER_CACHE_VERSION);
  serialization::writePod(f, static_cast<uint32_t>(fileSize));
  serialization::writePod(f, static_cast<uint32_t>(chapters.size()));

  for (const auto& ch : chapters) {
    serialization::writePod(f, static_cast<uint32_t>(ch.byteOffset));
    serialization::writeString(f, ch.title);
  }

  f.close();
  LOG_DBG("TXT", "Saved chapter cache: %zu chapters", chapters.size());
}

bool Txt::loadChapterCache() {
  std::string path = cachePath + "/chapters.bin";
  FsFile f;
  if (!Storage.openFileForRead("TXT", path, f)) {
    return false;
  }

  uint32_t magic;
  serialization::readPod(f, magic);
  if (magic != CHAPTER_CACHE_MAGIC) {
    f.close();
    return false;
  }

  uint8_t version;
  serialization::readPod(f, version);
  if (version != CHAPTER_CACHE_VERSION) {
    f.close();
    return false;
  }

  uint32_t cachedFileSize;
  serialization::readPod(f, cachedFileSize);
  if (cachedFileSize != static_cast<uint32_t>(fileSize)) {
    f.close();
    return false;
  }

  uint32_t count;
  serialization::readPod(f, count);
  if (count > 10000) {  // Sanity limit
    f.close();
    return false;
  }

  chapters.clear();
  chapters.reserve(count);

  for (uint32_t i = 0; i < count; i++) {
    TxtChapterInfo ch;
    uint32_t offset;
    serialization::readPod(f, offset);
    ch.byteOffset = offset;
    if (!serialization::readString(f, ch.title, 256)) {
      chapters.clear();
      f.close();
      return false;
    }
    chapters.push_back(std::move(ch));
  }

  f.close();
  return true;
}
