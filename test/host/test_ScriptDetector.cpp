#include <ScriptDetector.h>

#include "TestFramework.h"

// ---------------------------------------------------------------
// ScriptDetector::classify — script classification
// ---------------------------------------------------------------

TEST(classify_ascii_as_latin) {
  ASSERT_EQ(ScriptDetector::classify("hello"), ScriptDetector::Script::LATIN);
  return true;
}

TEST(classify_empty_as_other) {
  ASSERT_EQ(ScriptDetector::classify(""), ScriptDetector::Script::OTHER);
  ASSERT_EQ(ScriptDetector::classify(nullptr), ScriptDetector::Script::OTHER);
  return true;
}

TEST(classify_hangul) {
  // "한글" in UTF-8: 0xED 0x95 0x9C 0xEA 0xB8 0x80
  ASSERT_EQ(ScriptDetector::classify("\xED\x95\x9C\xEA\xB8\x80"), ScriptDetector::Script::HANGUL);
  return true;
}

TEST(classify_hangul_mixed_with_ascii) {
  // "Hello 한글" — first non-ASCII is Hangul
  ASSERT_EQ(ScriptDetector::classify("Hello \xED\x95\x9C\xEA\xB8\x80"), ScriptDetector::Script::HANGUL);
  return true;
}

TEST(classify_cjk_kanji) {
  // "漢字" (U+6F22 U+5B57)
  ASSERT_EQ(ScriptDetector::classify("\xE6\xBC\xA2\xE5\xAD\x97"), ScriptDetector::Script::CJK);
  return true;
}

TEST(classify_hiragana) {
  // "あ" U+3042
  ASSERT_EQ(ScriptDetector::classify("\xE3\x81\x82"), ScriptDetector::Script::CJK);
  return true;
}

TEST(classify_katakana) {
  // "ア" U+30A2
  ASSERT_EQ(ScriptDetector::classify("\xE3\x82\xA2"), ScriptDetector::Script::CJK);
  return true;
}

TEST(classify_thai) {
  // "ก" U+0E01
  ASSERT_EQ(ScriptDetector::classify("\xE0\xB8\x81"), ScriptDetector::Script::THAI);
  return true;
}

TEST(classify_arabic) {
  // "ع" U+0639
  ASSERT_EQ(ScriptDetector::classify("\xD8\xB9"), ScriptDetector::Script::ARABIC);
  return true;
}

TEST(classify_cyrillic_as_latin) {
  // "Б" U+0411
  ASSERT_EQ(ScriptDetector::classify("\xD0\x91"), ScriptDetector::Script::LATIN);
  return true;
}

// ---------------------------------------------------------------
// Codepoint-level checks
// ---------------------------------------------------------------

TEST(isCjk_hangul_syllable) {
  // U+AC00 = 가 (first Hangul syllable)
  ASSERT_TRUE(ScriptDetector::isCjkCodepoint(0xAC00));
  return true;
}

TEST(isCjk_cjk_ideograph) {
  // U+4E00 = 一
  ASSERT_TRUE(ScriptDetector::isCjkCodepoint(0x4E00));
  return true;
}

TEST(isCjk_latin_false) {
  ASSERT_FALSE(ScriptDetector::isCjkCodepoint('A'));
  return true;
}

TEST(isHangul_syllable) {
  ASSERT_TRUE(ScriptDetector::isHangul(0xAC00));  // 가
  ASSERT_TRUE(ScriptDetector::isHangul(0xD7AF));  // last Hangul syllable
  return true;
}

TEST(isHangul_jamo) {
  ASSERT_TRUE(ScriptDetector::isHangul(0x1100));  // ᄀ (Hangul Jamo)
  ASSERT_TRUE(ScriptDetector::isHangul(0x3131));  // ㄱ (Hangul Compat Jamo)
  return true;
}

TEST(isHangul_non_hangul) {
  ASSERT_FALSE(ScriptDetector::isHangul(0x4E00));  // CJK ideograph, not Hangul
  ASSERT_FALSE(ScriptDetector::isHangul('A'));
  return true;
}

// ---------------------------------------------------------------
// containsCjk / containsThai / containsArabic
// ---------------------------------------------------------------

TEST(containsCjk_positive) {
  ASSERT_TRUE(ScriptDetector::containsCjk("Hello \xED\x95\x9C\xEA\xB8\x80"));  // "Hello 한글"
  return true;
}

TEST(containsCjk_negative) {
  ASSERT_FALSE(ScriptDetector::containsCjk("Hello World"));
  return true;
}

TEST(containsCjk_null) {
  ASSERT_FALSE(ScriptDetector::containsCjk(nullptr));
  return true;
}

TEST(containsThai_positive) {
  ASSERT_TRUE(ScriptDetector::containsThai("abc \xE0\xB8\x81"));  // "abc ก"
  return true;
}

TEST(containsThai_negative) {
  ASSERT_FALSE(ScriptDetector::containsThai("Hello World"));
  return true;
}

TEST(containsArabic_positive) {
  ASSERT_TRUE(ScriptDetector::containsArabic("abc \xD8\xB9"));  // "abc ع"
  return true;
}

TEST(containsArabic_negative) {
  ASSERT_FALSE(ScriptDetector::containsArabic("Hello World"));
  return true;
}

int main() { return RUN_ALL_TESTS(); }
