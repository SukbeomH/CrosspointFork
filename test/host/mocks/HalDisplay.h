#pragma once
// ---------------------------------------------------------------
// HalDisplay stub for host-based unit tests.
// Provides type definitions only — no actual display operations.
// ---------------------------------------------------------------

#include <cstdint>

class HalDisplay {
 public:
  enum RefreshMode { FULL_REFRESH, HALF_REFRESH, FAST_REFRESH };

  static constexpr uint16_t DISPLAY_WIDTH = 480;
  static constexpr uint16_t DISPLAY_HEIGHT = 800;
  static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
  static constexpr uint32_t BUFFER_SIZE = DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

  void begin() {}
  void clearScreen(uint8_t /*color*/ = 0xFF) {}
  void displayBuffer(RefreshMode /*mode*/ = FAST_REFRESH, bool /*off*/ = false) {}
  void refreshDisplay(RefreshMode /*mode*/ = FAST_REFRESH, bool /*off*/ = false) {}
  void deepSleep() {}
  uint8_t* getFrameBuffer() { return nullptr; }
};
