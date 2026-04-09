#include "SdFontFamily.h"

#include <Arduino.h>
#include <HardwareSerial.h>

// ============================================================================
// SdFontFamily Implementation
// ============================================================================

SdFontFamily::SdFontFamily(const char* regularPath, const char* boldPath, const char* italicPath,
                           const char* boldItalicPath)
    : regular(nullptr), bold(nullptr), italic(nullptr), boldItalic(nullptr), ownsPointers(true) {
  if (regularPath) {
    regular = new SdFont(regularPath);
  }
  if (boldPath) {
    bold = new SdFont(boldPath);
  }
  if (italicPath) {
    italic = new SdFont(italicPath);
  }
  if (boldItalicPath) {
    boldItalic = new SdFont(boldItalicPath);
  }
}

SdFontFamily::~SdFontFamily() {
  if (ownsPointers) {
    delete regular;
    delete bold;
    delete italic;
    delete boldItalic;
  }
}

SdFontFamily::SdFontFamily(SdFontFamily&& other) noexcept
    : regular(other.regular),
      bold(other.bold),
      italic(other.italic),
      boldItalic(other.boldItalic),
      ownsPointers(other.ownsPointers) {
  other.regular = nullptr;
  other.bold = nullptr;
  other.italic = nullptr;
  other.boldItalic = nullptr;
  other.ownsPointers = false;
}

SdFontFamily& SdFontFamily::operator=(SdFontFamily&& other) noexcept {
  if (this != &other) {
    if (ownsPointers) {
      delete regular;
      delete bold;
      delete italic;
      delete boldItalic;
    }

    regular = other.regular;
    bold = other.bold;
    italic = other.italic;
    boldItalic = other.boldItalic;
    ownsPointers = other.ownsPointers;

    other.regular = nullptr;
    other.bold = nullptr;
    other.italic = nullptr;
    other.boldItalic = nullptr;
    other.ownsPointers = false;
  }
  return *this;
}

bool SdFontFamily::load() {
  bool success = true;

  if (regular && !regular->load()) {
    Serial.printf("[%lu] [SdFontFamily] Failed to load regular font\n", millis());
    success = false;
  }
  if (bold && !bold->load()) {
    Serial.printf("[%lu] [SdFontFamily] Failed to load bold font\n", millis());
    // Bold is optional, don't fail completely
  }
  if (italic && !italic->load()) {
    Serial.printf("[%lu] [SdFontFamily] Failed to load italic font\n", millis());
    // Italic is optional
  }
  if (boldItalic && !boldItalic->load()) {
    Serial.printf("[%lu] [SdFontFamily] Failed to load bold-italic font\n", millis());
    // Bold-italic is optional
  }

  return success;
}

bool SdFontFamily::isLoaded() const { return regular && regular->isLoaded(); }

SdFont* SdFontFamily::getFont(EpdFontStyle style) const {
  if (style == BOLD && bold && bold->isLoaded()) {
    return bold;
  }
  if (style == ITALIC && italic && italic->isLoaded()) {
    return italic;
  }
  if (style == BOLD_ITALIC) {
    if (boldItalic && boldItalic->isLoaded()) {
      return boldItalic;
    }
    if (bold && bold->isLoaded()) {
      return bold;
    }
    if (italic && italic->isLoaded()) {
      return italic;
    }
  }

  return regular;
}

void SdFontFamily::getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style) const {
  SdFont* font = getFont(style);
  if (font) {
    font->getTextDimensions(string, w, h);
  } else {
    *w = 0;
    *h = 0;
  }
}

bool SdFontFamily::hasPrintableChars(const char* string, EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->hasPrintableChars(string) : false;
}

const EpdGlyph* SdFontFamily::getGlyph(uint32_t cp, EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->getGlyph(cp) : nullptr;
}

const uint8_t* SdFontFamily::getGlyphBitmap(uint32_t cp, EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->getGlyphBitmap(cp) : nullptr;
}

uint8_t SdFontFamily::getAdvanceY(EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->getAdvanceY() : 0;
}

int8_t SdFontFamily::getAscender(EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->getAscender() : 0;
}

int8_t SdFontFamily::getDescender(EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->getDescender() : 0;
}

bool SdFontFamily::is2Bit(EpdFontStyle style) const {
  SdFont* font = getFont(style);
  return font ? font->is2Bit() : false;
}

// ============================================================================
// StreamingSdFontFamily Implementation
// ============================================================================

StreamingSdFontFamily::StreamingSdFontFamily(const char* regularPath, const char* boldPath, const char* italicPath,
                                             const char* boldItalicPath)
    : regular(nullptr), bold(nullptr), italic(nullptr), boldItalic(nullptr), ownsPointers(true) {
  if (regularPath) {
    regular = new StreamingEpdFont();
    regularPath_ = regularPath;
  }
  if (boldPath) {
    bold = new StreamingEpdFont();
    boldPath_ = boldPath;
  }
  if (italicPath) {
    italic = new StreamingEpdFont();
    italicPath_ = italicPath;
  }
  if (boldItalicPath) {
    boldItalic = new StreamingEpdFont();
    boldItalicPath_ = boldItalicPath;
  }
}

StreamingSdFontFamily::~StreamingSdFontFamily() {
  if (ownsPointers) {
    delete regular;
    delete bold;
    delete italic;
    delete boldItalic;
  }
}

StreamingSdFontFamily::StreamingSdFontFamily(StreamingSdFontFamily&& other) noexcept
    : regular(other.regular),
      bold(other.bold),
      italic(other.italic),
      boldItalic(other.boldItalic),
      ownsPointers(other.ownsPointers) {
  other.regular = nullptr;
  other.bold = nullptr;
  other.italic = nullptr;
  other.boldItalic = nullptr;
  other.ownsPointers = false;
}

StreamingSdFontFamily& StreamingSdFontFamily::operator=(StreamingSdFontFamily&& other) noexcept {
  if (this != &other) {
    if (ownsPointers) {
      delete regular;
      delete bold;
      delete italic;
      delete boldItalic;
    }

    regular = other.regular;
    bold = other.bold;
    italic = other.italic;
    boldItalic = other.boldItalic;
    ownsPointers = other.ownsPointers;

    other.regular = nullptr;
    other.bold = nullptr;
    other.italic = nullptr;
    other.boldItalic = nullptr;
    other.ownsPointers = false;
  }
  return *this;
}

bool StreamingSdFontFamily::load() {
  bool success = true;

  if (regular && !regularPath_.empty() && !regular->isLoaded()) {
    if (!regular->load(regularPath_.c_str())) {
      Serial.printf("[%lu] [StreamingSdFontFamily] Failed to load regular font\n", millis());
      success = false;
    }
  }
  if (bold && !boldPath_.empty() && !bold->isLoaded()) {
    if (!bold->load(boldPath_.c_str())) {
      Serial.printf("[%lu] [StreamingSdFontFamily] Failed to load bold font\n", millis());
      // Bold is optional, don't fail completely
    }
  }
  if (italic && !italicPath_.empty() && !italic->isLoaded()) {
    if (!italic->load(italicPath_.c_str())) {
      Serial.printf("[%lu] [StreamingSdFontFamily] Failed to load italic font\n", millis());
    }
  }
  if (boldItalic && !boldItalicPath_.empty() && !boldItalic->isLoaded()) {
    if (!boldItalic->load(boldItalicPath_.c_str())) {
      Serial.printf("[%lu] [StreamingSdFontFamily] Failed to load bold-italic font\n", millis());
    }
  }

  return success;
}

bool StreamingSdFontFamily::isLoaded() const { return regular && regular->isLoaded(); }

StreamingEpdFont* StreamingSdFontFamily::getFont(EpdFontStyle style) const {
  if (style == BOLD && bold && bold->isLoaded()) {
    return bold;
  }
  if (style == ITALIC && italic && italic->isLoaded()) {
    return italic;
  }
  if (style == BOLD_ITALIC) {
    if (boldItalic && boldItalic->isLoaded()) {
      return boldItalic;
    }
    if (bold && bold->isLoaded()) {
      return bold;
    }
    if (italic && italic->isLoaded()) {
      return italic;
    }
  }
  return regular;
}

void StreamingSdFontFamily::getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  if (font) {
    font->getTextDimensions(string, w, h);
  } else {
    *w = 0;
    *h = 0;
  }
}

bool StreamingSdFontFamily::hasPrintableChars(const char* string, EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->hasPrintableChars(string) : false;
}

const EpdGlyph* StreamingSdFontFamily::getGlyph(uint32_t cp, EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->getGlyph(cp) : nullptr;
}

const uint8_t* StreamingSdFontFamily::getGlyphBitmap(uint32_t cp, EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  if (!font) return nullptr;

  // StreamingEpdFont::getGlyphBitmap takes a glyph pointer, not codepoint.
  // We need to look up the glyph first, then get its bitmap.
  const EpdGlyph* glyph = font->getGlyph(cp);
  if (!glyph) return nullptr;

  return font->getGlyphBitmap(glyph);
}

uint8_t StreamingSdFontFamily::getAdvanceY(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->getAdvanceY() : 0;
}

int8_t StreamingSdFontFamily::getAscender(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->getAscender() : 0;
}

int8_t StreamingSdFontFamily::getDescender(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->getDescender() : 0;
}

bool StreamingSdFontFamily::is2Bit(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->is2Bit() : false;
}

size_t StreamingSdFontFamily::getMemoryUsage(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  return font ? font->getMemoryUsage() : 0;
}

void StreamingSdFontFamily::logCacheStats(EpdFontStyle style) const {
  StreamingEpdFont* font = getFont(style);
  if (font) font->logCacheStats();
}

// ============================================================================
// UnifiedFontFamily Implementation
// ============================================================================

UnifiedFontFamily::UnifiedFontFamily(const EpdFontFamily* font)
    : type(Type::FLASH), flashFont(font), sdFont(nullptr), streamingSdFont(nullptr) {}

UnifiedFontFamily::UnifiedFontFamily(SdFontFamily* font)
    : type(Type::SD), flashFont(nullptr), sdFont(font), streamingSdFont(nullptr) {}

UnifiedFontFamily::UnifiedFontFamily(StreamingSdFontFamily* font)
    : type(Type::STREAMING_SD), flashFont(nullptr), sdFont(nullptr), streamingSdFont(font) {}

UnifiedFontFamily::~UnifiedFontFamily() {
  // flashFont is not owned (points to global), don't delete
  delete sdFont;
  delete streamingSdFont;
}

UnifiedFontFamily::UnifiedFontFamily(UnifiedFontFamily&& other) noexcept
    : type(other.type), flashFont(other.flashFont), sdFont(other.sdFont), streamingSdFont(other.streamingSdFont) {
  other.flashFont = nullptr;
  other.sdFont = nullptr;
  other.streamingSdFont = nullptr;
}

UnifiedFontFamily& UnifiedFontFamily::operator=(UnifiedFontFamily&& other) noexcept {
  if (this != &other) {
    // flashFont is not owned (points to global), don't delete
    delete sdFont;
    delete streamingSdFont;

    type = other.type;
    flashFont = other.flashFont;
    sdFont = other.sdFont;
    streamingSdFont = other.streamingSdFont;

    other.flashFont = nullptr;
    other.sdFont = nullptr;
    other.streamingSdFont = nullptr;
  }
  return *this;
}

void UnifiedFontFamily::getTextDimensions(const char* string, int* w, int* h, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    flashFont->getTextDimensions(string, w, h, style);
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    streamingSdFont->getTextDimensions(string, w, h, style);
  } else if (sdFont) {
    sdFont->getTextDimensions(string, w, h, style);
  } else {
    *w = 0;
    *h = 0;
  }
}

bool UnifiedFontFamily::hasPrintableChars(const char* string, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->hasPrintableChars(string, style);
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->hasPrintableChars(string, style);
  } else if (sdFont) {
    return sdFont->hasPrintableChars(string, style);
  }
  return false;
}

const EpdGlyph* UnifiedFontFamily::getGlyph(uint32_t cp, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->getGlyph(cp, style);
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->getGlyph(cp, style);
  } else if (sdFont) {
    return sdFont->getGlyph(cp, style);
  }
  return nullptr;
}

const uint8_t* UnifiedFontFamily::getGlyphBitmap(uint32_t cp, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    // For flash fonts, get bitmap from the data structure
    const EpdFontData* data = flashFont->getData(style);
    const EpdGlyph* glyph = flashFont->getGlyph(cp, style);
    if (data && glyph) {
      return &data->bitmap[glyph->dataOffset];
    }
    return nullptr;
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->getGlyphBitmap(cp, style);
  } else if (sdFont) {
    return sdFont->getGlyphBitmap(cp, style);
  }
  return nullptr;
}

uint8_t UnifiedFontFamily::getAdvanceY(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    const EpdFontData* data = flashFont->getData(style);
    return data ? data->advanceY : 0;
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->getAdvanceY(style);
  } else if (sdFont) {
    return sdFont->getAdvanceY(style);
  }
  return 0;
}

int8_t UnifiedFontFamily::getAscender(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    const EpdFontData* data = flashFont->getData(style);
    return data ? data->ascender : 0;
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->getAscender(style);
  } else if (sdFont) {
    return sdFont->getAscender(style);
  }
  return 0;
}

int8_t UnifiedFontFamily::getDescender(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    const EpdFontData* data = flashFont->getData(style);
    return data ? data->descender : 0;
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->getDescender(style);
  } else if (sdFont) {
    return sdFont->getDescender(style);
  }
  return 0;
}

bool UnifiedFontFamily::is2Bit(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    const EpdFontData* data = flashFont->getData(style);
    return data ? data->is2Bit : false;
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->is2Bit(style);
  } else if (sdFont) {
    return sdFont->is2Bit(style);
  }
  return false;
}

const EpdFontData* UnifiedFontFamily::getFlashData(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->getData(style);
  }
  return nullptr;
}

bool UnifiedFontFamily::hasBold() const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->hasBold();
  } else if (type == Type::STREAMING_SD && streamingSdFont) {
    return streamingSdFont->hasBold();
  } else if (sdFont) {
    return sdFont->hasBold();
  }
  return false;
}

const EpdFontData* UnifiedFontFamily::getData(EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->getData(style);
  }
  // SD fonts don't have EpdFontData - return nullptr
  return nullptr;
}

int8_t UnifiedFontFamily::getKerning(uint32_t leftCp, uint32_t rightCp, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->getKerning(leftCp, rightCp, style);
  }
  // SD fonts don't support kerning
  return 0;
}

uint32_t UnifiedFontFamily::applyLigatures(uint32_t cp, const char*& text, EpdFontStyle style) const {
  if (type == Type::FLASH && flashFont) {
    return flashFont->applyLigatures(cp, text, style);
  }
  // SD fonts don't support ligatures - return codepoint as-is
  return cp;
}
