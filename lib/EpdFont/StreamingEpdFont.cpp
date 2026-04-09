#include "StreamingEpdFont.h"

#include <Arduino.h>
#include <HalStorage.h>
#include <HardwareSerial.h>
#include <Utf8.h>

#include <algorithm>
#include <cstring>
#include <new>

// Maximum reasonable values for validation (same as SdFont)
static constexpr uint32_t MAX_INTERVAL_COUNT = 10000;
static constexpr uint32_t MAX_GLYPH_COUNT = 150000;
static constexpr size_t MIN_FREE_HEAP_AFTER_LOAD = 16384;

StreamingEpdFont::StreamingEpdFont() {
  memset(&_header, 0, sizeof(_header));
  for (int i = 0; i < CACHE_SIZE; i++) {
    _hashTable[i] = HASH_EMPTY;
  }
}

StreamingEpdFont::~StreamingEpdFont() { unload(); }

bool StreamingEpdFont::ensureFileOpen() const {
  if (_fontFile && _fontFile.isOpen()) {
    return true;
  }
  // Reopen if closed (e.g., after SD card remount)
  // We don't store path, so we can't reopen. The file should stay open.
  return false;
}

bool StreamingEpdFont::load(const char* path) {
  unload();

  // Check available heap
  size_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < MIN_FREE_HEAP_AFTER_LOAD) {
    Serial.printf("[%lu] [StreamingEpdFont] Insufficient heap: %u bytes\n", millis(), freeHeap);
    return false;
  }

  // Open font file
  FsFile file;
  if (!Storage.openFileForRead("StreamFont", path, file)) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to open: %s\n", millis(), path);
    return false;
  }

  // Read and validate header (32 bytes)
  if (file.read(&_header, sizeof(EpdFontHeader)) != sizeof(EpdFontHeader)) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to read header: %s\n", millis(), path);
    file.close();
    return false;
  }

  if (_header.magic != EPDFONT_MAGIC) {
    Serial.printf("[%lu] [StreamingEpdFont] Invalid magic: 0x%08X\n", millis(), _header.magic);
    file.close();
    return false;
  }

  if (_header.version != EPDFONT_VERSION) {
    Serial.printf("[%lu] [StreamingEpdFont] Bad version: %u\n", millis(), _header.version);
    file.close();
    return false;
  }

  if (_header.intervalCount > MAX_INTERVAL_COUNT) {
    Serial.printf("[%lu] [StreamingEpdFont] Too many intervals: %u\n", millis(), _header.intervalCount);
    file.close();
    return false;
  }

  if (_header.glyphCount > MAX_GLYPH_COUNT) {
    Serial.printf("[%lu] [StreamingEpdFont] Too many glyphs: %u\n", millis(), _header.glyphCount);
    file.close();
    return false;
  }

  _glyphCount = _header.glyphCount;

  // Calculate required memory: intervals + glyphs (NOT bitmap -- that's streamed)
  _intervalsSize = _header.intervalCount * sizeof(EpdFontInterval);
  _glyphsSize = _glyphCount * sizeof(EpdGlyph);
  size_t requiredMemory = _intervalsSize + _glyphsSize;

  freeHeap = ESP.getFreeHeap();
  if (requiredMemory > freeHeap - MIN_FREE_HEAP_AFTER_LOAD) {
    Serial.printf("[%lu] [StreamingEpdFont] Not enough memory: need %u, have %u\n", millis(), requiredMemory, freeHeap);
    file.close();
    return false;
  }

  // Allocate intervals
  _intervals = new (std::nothrow) EpdFontInterval[_header.intervalCount];
  if (!_intervals) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to allocate intervals\n", millis());
    file.close();
    return false;
  }

  // Allocate glyphs (full table in RAM for O(1) access)
  _glyphs = new (std::nothrow) EpdGlyph[_glyphCount];
  if (!_glyphs) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to allocate glyphs\n", millis());
    delete[] _intervals;
    _intervals = nullptr;
    file.close();
    return false;
  }

  // Seek to intervals section
  if (!file.seekSet(_header.intervalsOffset)) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to seek to intervals\n", millis());
    delete[] _intervals;
    delete[] _glyphs;
    _intervals = nullptr;
    _glyphs = nullptr;
    file.close();
    return false;
  }

  // Read intervals (12 bytes each, matches EpdFontInterval)
  if (file.read(_intervals, _intervalsSize) != static_cast<int>(_intervalsSize)) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to read intervals\n", millis());
    delete[] _intervals;
    delete[] _glyphs;
    _intervals = nullptr;
    _glyphs = nullptr;
    file.close();
    return false;
  }

  // Seek to glyphs section and read them
  if (!file.seekSet(_header.glyphsOffset)) {
    Serial.printf("[%lu] [StreamingEpdFont] Failed to seek to glyphs\n", millis());
    delete[] _intervals;
    delete[] _glyphs;
    _intervals = nullptr;
    _glyphs = nullptr;
    file.close();
    return false;
  }

  // Read glyphs one by one, converting from file format (16 bytes) to runtime format
  for (uint32_t i = 0; i < _glyphCount; i++) {
    EpdFontGlyph fileGlyph;
    if (file.read(&fileGlyph, sizeof(EpdFontGlyph)) != sizeof(EpdFontGlyph)) {
      Serial.printf("[%lu] [StreamingEpdFont] Failed to read glyph %u\n", millis(), i);
      delete[] _intervals;
      delete[] _glyphs;
      _intervals = nullptr;
      _glyphs = nullptr;
      file.close();
      return false;
    }

    // Convert from file format to runtime format
    _glyphs[i].width = fileGlyph.width;
    _glyphs[i].height = fileGlyph.height;
    _glyphs[i].advanceX = fp4::fromPixel(fileGlyph.advanceX);  // pixel -> 12.4 fixed-point
    _glyphs[i].left = fileGlyph.left;
    _glyphs[i].top = fileGlyph.top;
    _glyphs[i].dataLength = static_cast<uint16_t>(fileGlyph.dataLength);
    _glyphs[i].dataOffset = fileGlyph.dataOffset;
  }

  // Transfer the file handle for streaming (keep open)
  _fontFile = std::move(file);

  _isLoaded = true;

  Serial.printf("[%lu] [StreamingEpdFont] Loaded: advanceY=%u, glyphs=%u, intervals=%u, RAM=%uKB (no bitmap)\n",
                millis(), _header.advanceY, _glyphCount, _header.intervalCount,
                (sizeof(StreamingEpdFont) + _intervalsSize + _glyphsSize) / 1024);

  return true;
}

void StreamingEpdFont::unload() {
  if (_fontFile) {
    _fontFile.close();
  }

  delete[] _glyphs;
  delete[] _intervals;

  // Free all cached bitmaps
  for (int i = 0; i < CACHE_SIZE; i++) {
    delete[] _cache[i].bitmap;
    _cache[i].bitmap = nullptr;
    _cache[i].glyphIndex = INVALID_GLYPH_INDEX;
    _cache[i].bitmapSize = 0;
    _cache[i].lastUsed = 0;
    _hashTable[i] = HASH_EMPTY;
  }

  // Clear glyph lookup cache
  for (int i = 0; i < GLYPH_CACHE_SIZE; i++) {
    _glyphCache[i].codepoint = INVALID_GLYPH_INDEX;
    _glyphCache[i].glyph = nullptr;
  }

  _glyphs = nullptr;
  _intervals = nullptr;
  _glyphCount = 0;
  _glyphsSize = 0;
  _intervalsSize = 0;
  _isLoaded = false;
  _accessCounter = 0;
  _totalCacheAllocation = 0;
  _tombstoneCount = 0;
  _cacheHits = 0;
  _cacheMisses = 0;

  memset(&_header, 0, sizeof(_header));
}

const EpdGlyph* StreamingEpdFont::lookupGlyph(uint32_t cp) const {
  // Check glyph cache first (O(1) for hot glyphs)
  const int cacheIdx = cp % GLYPH_CACHE_SIZE;
  if (_glyphCache[cacheIdx].codepoint == cp) {
    return _glyphCache[cacheIdx].glyph;
  }

  // Binary search in intervals (O(log n))
  const int count = static_cast<int>(_header.intervalCount);
  if (count == 0) return nullptr;

  int left = 0;
  int right = count - 1;

  while (left <= right) {
    const int mid = left + (right - left) / 2;
    const EpdFontInterval* interval = &_intervals[mid];

    if (cp < interval->first) {
      right = mid - 1;
    } else if (cp > interval->last) {
      left = mid + 1;
    } else {
      // Found: cp is within this interval
      const uint32_t glyphIdx = interval->offset + (cp - interval->first);
      if (glyphIdx >= _glyphCount) {
        return nullptr;  // Corrupted font data
      }
      const EpdGlyph* glyph = &_glyphs[glyphIdx];
      // Store in cache
      _glyphCache[cacheIdx].codepoint = cp;
      _glyphCache[cacheIdx].glyph = glyph;
      return glyph;
    }
  }

  return nullptr;
}

const EpdGlyph* StreamingEpdFont::getGlyph(uint32_t cp) {
  if (!_isLoaded) return nullptr;
  return lookupGlyph(cp);
}

int StreamingEpdFont::findInBitmapCache(uint32_t glyphIndex) {
  int hash = hashIndex(glyphIndex);
  for (int i = 0; i < CACHE_SIZE; i++) {
    int idx = (hash + i) % CACHE_SIZE;
    int16_t cacheIdx = _hashTable[idx];
    if (cacheIdx == HASH_EMPTY) {
      return -1;
    }
    if (cacheIdx == HASH_TOMBSTONE) {
      continue;
    }
    if (_cache[cacheIdx].glyphIndex == glyphIndex) {
      return cacheIdx;
    }
  }
  return -1;
}

int StreamingEpdFont::getLruSlot() {
  int lruIndex = 0;
  uint32_t minUsed = _cache[0].lastUsed;

  for (int i = 1; i < CACHE_SIZE; i++) {
    // Prefer unused slots
    if (_cache[i].glyphIndex == INVALID_GLYPH_INDEX) {
      return i;
    }
    if (_cache[i].lastUsed < minUsed) {
      minUsed = _cache[i].lastUsed;
      lruIndex = i;
    }
  }
  return lruIndex;
}

bool StreamingEpdFont::loadGlyphBitmap(uint32_t glyphIndex, CachedBitmap& entry) {
  if (!_fontFile || glyphIndex >= _glyphCount) {
    return false;
  }

  const EpdGlyph& glyph = _glyphs[glyphIndex];
  const uint16_t dataLen = glyph.dataLength;

  if (dataLen == 0 || dataLen > MAX_GLYPH_BITMAP_SIZE) {
    return false;
  }

  // Reallocate bitmap buffer if needed
  if (entry.bitmapSize < dataLen) {
    const uint16_t oldSize = entry.bitmapSize;
    delete[] entry.bitmap;
    entry.bitmap = new (std::nothrow) uint8_t[dataLen];
    if (!entry.bitmap) {
      entry.bitmapSize = 0;
      _totalCacheAllocation -= oldSize;
      return false;
    }
    _totalCacheAllocation = _totalCacheAllocation - oldSize + dataLen;
    entry.bitmapSize = dataLen;
  }

  // Calculate file position: bitmapOffset + glyph's dataOffset
  // Retry seek+read on transient SD card failures
  uint32_t filePos = _header.bitmapOffset + glyph.dataOffset;
  for (int attempt = 0; attempt < 3; attempt++) {
    if (attempt > 0) delay(50);
    if (!_fontFile.seekSet(filePos)) continue;
    if (_fontFile.read(entry.bitmap, dataLen) == dataLen) return true;
  }

  return false;
}

const uint8_t* StreamingEpdFont::getGlyphBitmap(const EpdGlyph* glyph) {
  if (!_isLoaded || !glyph) return nullptr;

  // Validate glyph pointer belongs to this font instance
  if (glyph < _glyphs || glyph >= _glyphs + _glyphCount) {
    return nullptr;
  }

  uint32_t glyphIndex = glyph - _glyphs;

  // Check bitmap cache
  int cacheIndex = findInBitmapCache(glyphIndex);
  if (cacheIndex >= 0) {
    _cache[cacheIndex].lastUsed = ++_accessCounter;
    _cacheHits++;
    return _cache[cacheIndex].bitmap;
  }

  _cacheMisses++;

  // Cache miss - need to load from SD
  int slot = getLruSlot();

  // If replacing an existing entry, mark tombstone in hash table
  if (_cache[slot].glyphIndex != INVALID_GLYPH_INDEX) {
    int oldHash = hashIndex(_cache[slot].glyphIndex);
    for (int i = 0; i < CACHE_SIZE; i++) {
      int idx = (oldHash + i) % CACHE_SIZE;
      if (_hashTable[idx] == slot) {
        _hashTable[idx] = HASH_TOMBSTONE;
        _tombstoneCount++;
        break;
      }
    }
    _totalCacheAllocation -= _cache[slot].bitmapSize;

    if (_tombstoneCount >= TOMBSTONE_REHASH_THRESHOLD) {
      rehashTable();
    }
  }

  // Load glyph bitmap from SD
  if (!loadGlyphBitmap(glyphIndex, _cache[slot])) {
    return nullptr;
  }

  _cache[slot].glyphIndex = glyphIndex;
  _cache[slot].lastUsed = ++_accessCounter;

  // Add to hash table
  int hash = hashIndex(glyphIndex);
  bool inserted = false;
  for (int i = 0; i < CACHE_SIZE; i++) {
    int idx = (hash + i) % CACHE_SIZE;
    if (_hashTable[idx] == HASH_EMPTY || _hashTable[idx] == HASH_TOMBSTONE) {
      _hashTable[idx] = slot;
      inserted = true;
      break;
    }
  }

  // Force rehash if hash table is full
  if (!inserted) {
    rehashTable();
    hash = hashIndex(glyphIndex);
    for (int i = 0; i < CACHE_SIZE; i++) {
      int idx = (hash + i) % CACHE_SIZE;
      if (_hashTable[idx] == HASH_EMPTY) {
        _hashTable[idx] = slot;
        break;
      }
    }
  }

  return _cache[slot].bitmap;
}

void StreamingEpdFont::rehashTable() {
  for (int i = 0; i < CACHE_SIZE; i++) {
    _hashTable[i] = HASH_EMPTY;
  }
  _tombstoneCount = 0;

  for (int slot = 0; slot < CACHE_SIZE; slot++) {
    if (_cache[slot].glyphIndex != INVALID_GLYPH_INDEX) {
      int hash = hashIndex(_cache[slot].glyphIndex);
      for (int i = 0; i < CACHE_SIZE; i++) {
        int idx = (hash + i) % CACHE_SIZE;
        if (_hashTable[idx] == HASH_EMPTY) {
          _hashTable[idx] = slot;
          break;
        }
      }
    }
  }
}

void StreamingEpdFont::getTextDimensions(const char* string, int* w, int* h) const {
  *w = 0;
  *h = 0;

  if (!_isLoaded || !string || *string == '\0') {
    return;
  }

  int minX = 0, minY = 0, maxX = 0, maxY = 0;
  int32_t cursorXFP = 0;  // Fixed-point accumulator (12.4)
  const int cursorY = 0;

  uint32_t cp;
  while ((cp = utf8NextCodepoint(reinterpret_cast<const uint8_t**>(&string)))) {
    const EpdGlyph* glyph = lookupGlyph(cp);
    if (!glyph) {
      glyph = lookupGlyph('?');
    }
    if (!glyph) {
      continue;
    }

    int cursorXPixels = fp4::toPixel(cursorXFP);

    if (utf8IsCombiningMark(cp)) {
      // Center combining marks over previous base character
      // (simplified: just extend bounding box without advancing)
      minX = std::min(minX, cursorXPixels + glyph->left);
      maxX = std::max(maxX, cursorXPixels + glyph->left + glyph->width);
    } else {
      minX = std::min(minX, cursorXPixels + glyph->left);
      maxX = std::max(maxX, cursorXPixels + glyph->left + glyph->width);
      cursorXFP += glyph->advanceX;
    }

    minY = std::min(minY, cursorY + glyph->top - glyph->height);
    maxY = std::max(maxY, cursorY + glyph->top);
  }

  *w = maxX - minX;
  *h = maxY - minY;
}

bool StreamingEpdFont::hasPrintableChars(const char* string) const {
  int w = 0, h = 0;
  getTextDimensions(string, &w, &h);
  return w > 0 || h > 0;
}

size_t StreamingEpdFont::getMemoryUsage() const {
  size_t usage = sizeof(StreamingEpdFont);
  usage += _glyphsSize;
  usage += _intervalsSize;
  usage += _totalCacheAllocation;
  return usage;
}

void StreamingEpdFont::logCacheStats() const {
  if (!_isLoaded) return;

  uint32_t total = _cacheHits + _cacheMisses;
  float hitRate = total > 0 ? (100.0f * _cacheHits / total) : 0.0f;

  Serial.printf("[%lu] [StreamingEpdFont] Cache: hits=%u misses=%u rate=%.1f%% alloc=%uB\n", millis(), _cacheHits,
                _cacheMisses, hitRate, _totalCacheAllocation);
}
