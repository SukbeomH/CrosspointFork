#pragma once
#include <Txt.h>

#include <memory>

#include "../Activity.h"
#include "util/ButtonNavigator.h"

class TxtReaderChapterSelectionActivity final : public Activity {
  std::shared_ptr<Txt> txt;
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
  size_t currentByteOffset = 0;  // Current reading position for initial highlight

  int getPageItems() const;
  int findChapterIndexForOffset(size_t offset) const;

 public:
  explicit TxtReaderChapterSelectionActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                             const std::shared_ptr<Txt>& txt, size_t currentByteOffset)
      : Activity("TxtReaderChapterSelection", renderer, mappedInput),
        txt(txt),
        currentByteOffset(currentByteOffset) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool isReaderActivity() const override { return true; }
};
