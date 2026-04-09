#pragma once

#include <HalStorage.h>

#include <cstdint>

#include "EpdFontData.h"
#include "SdFontFormat.h"

/**
 * Streaming font loader for .epdfont files.
 *
 * Unlike SdFont which loads glyph metadata on-demand and caches bitmaps
 * with STL containers (~40-70KB), StreamingEpdFont preloads the interval
 * and glyph tables into RAM but streams bitmap data from SD card on demand
 * with a compact LRU cache (~25KB total).
 *
 * Memory comparison for typical Korean font:
 *   - SdFont:            ~40-70KB (intervals + on-demand glyphs + shared bitmap cache)
 *   - StreamingEpdFont:  ~25KB   (intervals + glyphs + fixed LRU cache)
 *
 * Trade-off: Slightly slower glyph access on cache miss (SD card read)
 *            but significantly lower and more predictable RAM usage.
 *
 * Uses the same .epdfont binary format as SdFont -- drop-in alternative.
 */
class StreamingEpdFont {
 public:
  StreamingEpdFont();
  ~StreamingEpdFont();

  StreamingEpdFont(const StreamingEpdFont&) = delete;
  StreamingEpdFont& operator=(const StreamingEpdFont&) = delete;

  /**
   * Load font from .epdfont file (streaming mode).
   * Loads header, intervals, and glyph table into RAM.
   * Keeps file handle open for bitmap streaming.
   *
   * @param path Path to .epdfont file on SD card
   * @return true on success
   */
  bool load(const char* path);

  /**
   * Unload font and free all resources.
   */
  void unload();

  /**
   * Check if font is loaded and ready.
   */
  bool isLoaded() const { return _isLoaded; }

  /**
   * Get glyph data for a unicode codepoint.
   * Performs binary search on interval table. O(log n).
   *
   * @param cp Unicode codepoint
   * @return Pointer to glyph data, or nullptr if not found
   */
  const EpdGlyph* getGlyph(uint32_t cp);

  /**
   * Get glyph bitmap data for a glyph.
   * Uses LRU cache - may trigger SD card read on cache miss.
   *
   * @param glyph Pointer to glyph obtained from this font instance
   * @return Pointer to bitmap data, or nullptr on error
   */
  const uint8_t* getGlyphBitmap(const EpdGlyph* glyph);

  /**
   * Calculate text dimensions without rendering.
   */
  void getTextDimensions(const char* string, int* w, int* h) const;

  /**
   * Check if string contains any printable characters.
   */
  bool hasPrintableChars(const char* string) const;

  // Font metrics accessors
  uint8_t getAdvanceY() const { return _header.advanceY; }
  int8_t getAscender() const { return _header.ascender; }
  int8_t getDescender() const { return _header.descender; }
  bool is2Bit() const { return _header.is2Bit != 0; }

  /**
   * Get total RAM usage of this font instance.
   */
  size_t getMemoryUsage() const;

  /**
   * Log cache statistics for debugging.
   */
  void logCacheStats() const;

  /**
   * Get the configured cache size.
   */
  static constexpr int getCacheSize() { return CACHE_SIZE; }

 private:
  // Cache configuration: 15 entries for Korean text (frequently reused syllable blocks)
  static constexpr int CACHE_SIZE = 15;
  static constexpr uint32_t INVALID_GLYPH_INDEX = 0xFFFFFFFF;

  // Maximum allowed glyph bitmap size (defense against corrupted font files)
  static constexpr uint16_t MAX_GLYPH_BITMAP_SIZE = 4096;

  // Hash table markers
  static constexpr int16_t HASH_EMPTY = -1;
  static constexpr int16_t HASH_TOMBSTONE = -2;

  // Rehash when tombstones exceed 25% of table size
  static constexpr int TOMBSTONE_REHASH_THRESHOLD = CACHE_SIZE / 4;

  // Font file metadata (in RAM)
  EpdFontHeader _header;
  EpdGlyph* _glyphs = nullptr;          // Full glyph table (advanceX already in fp4)
  EpdFontInterval* _intervals = nullptr; // Unicode interval table
  uint32_t _glyphCount = 0;

  // File handle (kept open for streaming)
  mutable FsFile _fontFile;
  bool _isLoaded = false;

  // Memory tracking
  size_t _glyphsSize = 0;
  size_t _intervalsSize = 0;

  // LRU bitmap cache (fixed-size array, no STL containers)
  struct CachedBitmap {
    uint32_t glyphIndex = INVALID_GLYPH_INDEX;
    uint8_t* bitmap = nullptr;
    uint16_t bitmapSize = 0;
    uint32_t lastUsed = 0;
  };
  CachedBitmap _cache[CACHE_SIZE];
  int16_t _hashTable[CACHE_SIZE];
  uint32_t _accessCounter = 0;
  size_t _totalCacheAllocation = 0;
  int _tombstoneCount = 0;

  // Cache statistics
  mutable uint32_t _cacheHits = 0;
  mutable uint32_t _cacheMisses = 0;

  // Glyph lookup cache (codepoint -> glyph pointer, for O(1) repeated lookups)
  static constexpr int GLYPH_CACHE_SIZE = 64;
  struct GlyphCacheEntry {
    uint32_t codepoint = INVALID_GLYPH_INDEX;
    const EpdGlyph* glyph = nullptr;
  };
  mutable GlyphCacheEntry _glyphCache[GLYPH_CACHE_SIZE];

  // Helper methods
  static int hashIndex(uint32_t index) { return index % CACHE_SIZE; }
  int findInBitmapCache(uint32_t glyphIndex);
  int getLruSlot();
  bool loadGlyphBitmap(uint32_t glyphIndex, CachedBitmap& entry);
  const EpdGlyph* lookupGlyph(uint32_t cp) const;
  void rehashTable();

  // Ensure font file is open
  bool ensureFileOpen() const;
};
