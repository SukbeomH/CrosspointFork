#pragma once

#include <string>

#include "EpdFontFamily.h"
#include "SdFont.h"
#include "StreamingEpdFont.h"

/**
 * SD Card font family - similar interface to EpdFontFamily but uses SdFont.
 * Supports regular, bold, italic, and bold-italic variants.
 */
class SdFontFamily {
 private:
  SdFont* regular;
  SdFont* bold;
  SdFont* italic;
  SdFont* boldItalic;
  bool ownsPointers;

  SdFont* getFont(EpdFontStyle style) const;

 public:
  // Constructor with raw pointers (does not take ownership)
  explicit SdFontFamily(SdFont* regular, SdFont* bold = nullptr, SdFont* italic = nullptr, SdFont* boldItalic = nullptr)
      : regular(regular), bold(bold), italic(italic), boldItalic(boldItalic), ownsPointers(false) {}

  // Constructor with file paths (creates and owns SdFont objects)
  explicit SdFontFamily(const char* regularPath, const char* boldPath = nullptr, const char* italicPath = nullptr,
                        const char* boldItalicPath = nullptr);

  ~SdFontFamily();

  // Disable copy
  SdFontFamily(const SdFontFamily&) = delete;
  SdFontFamily& operator=(const SdFontFamily&) = delete;

  // Enable move
  SdFontFamily(SdFontFamily&& other) noexcept;
  SdFontFamily& operator=(SdFontFamily&& other) noexcept;

  // Load all fonts in the family
  bool load();
  bool isLoaded() const;

  // EpdFontFamily-compatible interface
  void getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style = REGULAR) const;
  bool hasPrintableChars(const char* string, EpdFontStyle style = REGULAR) const;

  // Get glyph (metadata only, no bitmap)
  const EpdGlyph* getGlyph(uint32_t cp, EpdFontStyle style = REGULAR) const;

  // Get glyph bitmap data (loaded on demand from SD)
  const uint8_t* getGlyphBitmap(uint32_t cp, EpdFontStyle style = REGULAR) const;

  // Font metadata
  uint8_t getAdvanceY(EpdFontStyle style = REGULAR) const;
  int8_t getAscender(EpdFontStyle style = REGULAR) const;
  int8_t getDescender(EpdFontStyle style = REGULAR) const;
  bool is2Bit(EpdFontStyle style = REGULAR) const;

  // Check if bold variant is available
  bool hasBold() const { return bold != nullptr; }
};

/**
 * Streaming SD font family - uses StreamingEpdFont for lower RAM usage.
 * Drop-in alternative to SdFontFamily with identical interface.
 * RAM savings: ~45KB per font (streams bitmap data from SD on demand).
 */
class StreamingSdFontFamily {
 private:
  StreamingEpdFont* regular;
  StreamingEpdFont* bold;
  StreamingEpdFont* italic;
  StreamingEpdFont* boldItalic;
  bool ownsPointers;

  // Store paths for deferred loading
  std::string regularPath_;
  std::string boldPath_;
  std::string italicPath_;
  std::string boldItalicPath_;

  StreamingEpdFont* getFont(EpdFontStyle style) const;

 public:
  // Constructor with file paths (creates StreamingEpdFont objects, call load() to open)
  explicit StreamingSdFontFamily(const char* regularPath, const char* boldPath = nullptr,
                                 const char* italicPath = nullptr, const char* boldItalicPath = nullptr);

  ~StreamingSdFontFamily();

  // Disable copy
  StreamingSdFontFamily(const StreamingSdFontFamily&) = delete;
  StreamingSdFontFamily& operator=(const StreamingSdFontFamily&) = delete;

  // Enable move
  StreamingSdFontFamily(StreamingSdFontFamily&& other) noexcept;
  StreamingSdFontFamily& operator=(StreamingSdFontFamily&& other) noexcept;

  // Load all fonts in the family
  bool load();
  bool isLoaded() const;

  // SdFontFamily-compatible interface
  void getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style = REGULAR) const;
  bool hasPrintableChars(const char* string, EpdFontStyle style = REGULAR) const;
  const EpdGlyph* getGlyph(uint32_t cp, EpdFontStyle style = REGULAR) const;
  const uint8_t* getGlyphBitmap(uint32_t cp, EpdFontStyle style = REGULAR) const;

  // Font metadata
  uint8_t getAdvanceY(EpdFontStyle style = REGULAR) const;
  int8_t getAscender(EpdFontStyle style = REGULAR) const;
  int8_t getDescender(EpdFontStyle style = REGULAR) const;
  bool is2Bit(EpdFontStyle style = REGULAR) const;
  bool hasBold() const { return bold != nullptr; }

  // Memory diagnostics
  size_t getMemoryUsage(EpdFontStyle style = REGULAR) const;
  void logCacheStats(EpdFontStyle style = REGULAR) const;
};

/**
 * Unified font family that can hold either EpdFontFamily (flash), SdFontFamily (SD card),
 * or StreamingSdFontFamily (streaming SD with LRU cache).
 * This allows GfxRenderer to work with all types transparently.
 */
class UnifiedFontFamily {
 public:
  enum class Type { FLASH, SD, STREAMING_SD };

 private:
  Type type;
  const EpdFontFamily* flashFont;          // Non-owning pointer for flash fonts (they're global)
  SdFontFamily* sdFont;                    // Owned pointer for SD fonts
  StreamingSdFontFamily* streamingSdFont;  // Owned pointer for streaming SD fonts

 public:
  // Construct from flash font (EpdFontFamily) - stores pointer, does not copy
  explicit UnifiedFontFamily(const EpdFontFamily* font);

  // Construct from SD font family (takes ownership)
  explicit UnifiedFontFamily(SdFontFamily* font);

  // Construct from streaming SD font family (takes ownership)
  explicit UnifiedFontFamily(StreamingSdFontFamily* font);

  ~UnifiedFontFamily();

  // Disable copy
  UnifiedFontFamily(const UnifiedFontFamily&) = delete;
  UnifiedFontFamily& operator=(const UnifiedFontFamily&) = delete;

  // Enable move
  UnifiedFontFamily(UnifiedFontFamily&& other) noexcept;
  UnifiedFontFamily& operator=(UnifiedFontFamily&& other) noexcept;

  Type getType() const { return type; }
  bool isSdFont() const { return type == Type::SD || type == Type::STREAMING_SD; }

  // Unified interface
  void getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style = REGULAR) const;
  bool hasPrintableChars(const char* string, EpdFontStyle style = REGULAR) const;
  const EpdGlyph* getGlyph(uint32_t cp, EpdFontStyle style = REGULAR) const;

  // For SD fonts: get bitmap data (for flash fonts, use getData()->bitmap[offset])
  const uint8_t* getGlyphBitmap(uint32_t cp, EpdFontStyle style = REGULAR) const;

  // Metadata (common interface)
  uint8_t getAdvanceY(EpdFontStyle style = REGULAR) const;
  int8_t getAscender(EpdFontStyle style = REGULAR) const;
  int8_t getDescender(EpdFontStyle style = REGULAR) const;
  bool is2Bit(EpdFontStyle style = REGULAR) const;

  // Flash font specific (returns nullptr for SD fonts)
  const EpdFontData* getFlashData(EpdFontStyle style = REGULAR) const;

  // Upstream-compatible interface for kerning, ligatures, and font data access
  const EpdFontData* getData(EpdFontStyle style = REGULAR) const;
  int8_t getKerning(uint32_t leftCp, uint32_t rightCp, EpdFontStyle style = REGULAR) const;
  uint32_t applyLigatures(uint32_t cp, const char*& text, EpdFontStyle style = REGULAR) const;

  // Check if bold variant is available (for synthetic bold decision)
  bool hasBold() const;
};
