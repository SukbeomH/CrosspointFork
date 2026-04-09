#include "TxtReaderChapterSelectionActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

int TxtReaderChapterSelectionActivity::getPageItems() const {
  constexpr int lineHeight = 30;

  const int screenHeight = renderer.getScreenHeight();
  const auto orientation = renderer.getOrientation();
  const bool isPortraitInverted = orientation == GfxRenderer::Orientation::PortraitInverted;
  const int hintGutterHeight = isPortraitInverted ? 50 : 0;
  const int startY = 60 + hintGutterHeight;
  const int availableHeight = screenHeight - startY - lineHeight;
  return std::max(1, availableHeight / lineHeight);
}

int TxtReaderChapterSelectionActivity::findChapterIndexForOffset(size_t offset) const {
  if (!txt) return 0;

  const auto& chapters = txt->getChapters();
  if (chapters.empty()) return 0;

  // Find the last chapter whose byteOffset <= current reading offset
  int best = 0;
  for (size_t i = 0; i < chapters.size(); i++) {
    if (chapters[i].byteOffset <= offset) {
      best = static_cast<int>(i);
    } else {
      break;
    }
  }
  return best;
}

void TxtReaderChapterSelectionActivity::onEnter() {
  Activity::onEnter();

  if (!txt) return;

  selectorIndex = findChapterIndexForOffset(currentByteOffset);
  requestUpdate();
}

void TxtReaderChapterSelectionActivity::onExit() { Activity::onExit(); }

void TxtReaderChapterSelectionActivity::loop() {
  const int pageItems = getPageItems();
  const int totalItems = static_cast<int>(txt->getChapters().size());

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    const auto& chapters = txt->getChapters();
    if (!chapters.empty() && selectorIndex >= 0 && selectorIndex < static_cast<int>(chapters.size())) {
      setResult(TxtChapterResult{chapters[selectorIndex].byteOffset});
      finish();
    }
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();
  }

  buttonNavigator.onNextRelease([this, totalItems] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, totalItems);
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, totalItems] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, totalItems);
    requestUpdate();
  });

  buttonNavigator.onNextContinuous([this, totalItems, pageItems] {
    selectorIndex = ButtonNavigator::nextPageIndex(selectorIndex, totalItems, pageItems);
    requestUpdate();
  });

  buttonNavigator.onPreviousContinuous([this, totalItems, pageItems] {
    selectorIndex = ButtonNavigator::previousPageIndex(selectorIndex, totalItems, pageItems);
    requestUpdate();
  });
}

void TxtReaderChapterSelectionActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto orientation = renderer.getOrientation();
  const bool isLandscapeCw = orientation == GfxRenderer::Orientation::LandscapeClockwise;
  const bool isLandscapeCcw = orientation == GfxRenderer::Orientation::LandscapeCounterClockwise;
  const bool isPortraitInverted = orientation == GfxRenderer::Orientation::PortraitInverted;
  const int hintGutterWidth = (isLandscapeCw || isLandscapeCcw) ? 30 : 0;
  const int contentX = isLandscapeCw ? hintGutterWidth : 0;
  const int contentWidth = pageWidth - hintGutterWidth;
  const int hintGutterHeight = isPortraitInverted ? 50 : 0;
  const int contentY = hintGutterHeight;
  const int pageItems = getPageItems();

  // Title
  const int titleX =
      contentX + (contentWidth - renderer.getTextWidth(UI_12_FONT_ID, tr(STR_SELECT_CHAPTER), EpdFontFamily::BOLD)) / 2;
  renderer.drawText(UI_12_FONT_ID, titleX, 15 + contentY, tr(STR_SELECT_CHAPTER), true, EpdFontFamily::BOLD);

  const auto& chapters = txt->getChapters();
  if (chapters.empty()) {
    const int emptyX = contentX + (contentWidth - renderer.getTextWidth(UI_10_FONT_ID, tr(STR_NO_CHAPTERS))) / 2;
    renderer.drawText(UI_10_FONT_ID, emptyX, 120 + contentY, tr(STR_NO_CHAPTERS));
    renderer.displayBuffer();
    return;
  }

  const auto pageStartIndex = selectorIndex / pageItems * pageItems;
  // Highlight selected item
  renderer.fillRect(contentX, 60 + contentY + (selectorIndex % pageItems) * 30 - 2, contentWidth - 1, 30);

  for (int i = pageStartIndex; i < static_cast<int>(chapters.size()) && i < pageStartIndex + pageItems; i++) {
    const auto& chapter = chapters[i];
    const char* title = chapter.title.empty() ? tr(STR_UNNAMED) : chapter.title.c_str();
    const std::string displayTitle = renderer.truncatedText(UI_10_FONT_ID, title, contentWidth - 40);
    renderer.drawText(UI_10_FONT_ID, contentX + 20, 60 + contentY + (i % pageItems) * 30, displayTitle.c_str(),
                      i != selectorIndex);
  }

  // Button hints
  if (renderer.getOrientation() != GfxRenderer::LandscapeClockwise) {
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer();
}
