#include "AppsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "components/UITheme.h"
#include "fontIds.h"
#include "util/ShortcutRegistry.h"

void AppsActivity::onEnter() {
  Activity::onEnter();
  appShortcuts = getConfiguredShortcuts(CrossPointSettings::SHORTCUT_APPS);
  selectedIndex = 0;
  requestUpdate();
}

void AppsActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    openSelectedApp();
    return;
  }

  buttonNavigator.onNext([this] {
    if (appShortcuts.empty()) {
      return;
    }
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, static_cast<int>(appShortcuts.size()));
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    if (appShortcuts.empty()) {
      return;
    }
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, static_cast<int>(appShortcuts.size()));
    requestUpdate();
  });
}

void AppsActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_APPS));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;

  if (appShortcuts.empty()) {
    renderer.drawCenteredText(UI_FONT_ID, contentTop + 24, tr(STR_NO_ENTRIES));
  } else {
    GUI.drawList(
        renderer, Rect{0, contentTop, pageWidth, contentHeight}, static_cast<int>(appShortcuts.size()), selectedIndex,
        [this](const int index) { return std::string(I18N.get(appShortcuts[index]->nameId)); }, nullptr,
        [this](const int index) { return appShortcuts[index]->icon; });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

void AppsActivity::openSelectedApp() {
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(appShortcuts.size())) {
    return;
  }

  switch (appShortcuts[selectedIndex]->id) {
    case ShortcutId::BrowseFiles:
      activityManager.goToFileBrowser();
      return;
    case ShortcutId::RecentBooks:
      activityManager.goToRecentBooks();
      return;
    case ShortcutId::FileTransfer:
      activityManager.goToFileTransfer();
      return;
    case ShortcutId::Settings:
      activityManager.goToSettings();
      return;
  }
}
