#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "CrossPointSettings.h"
#include "I18n.h"
#include "components/themes/BaseTheme.h"

enum class ShortcutId {
  BrowseFiles = 0,
  RecentBooks,
  FileTransfer,
  Settings,
};

struct ShortcutDefinition {
  ShortcutId id;
  StrId nameId;
  UIIcon icon;
  uint8_t CrossPointSettings::* locationPtr;
  uint8_t CrossPointSettings::* orderPtr;
  uint8_t CrossPointSettings::* visiblePtr;
};

inline const std::array<ShortcutDefinition, 4>& getShortcutDefinitions() {
  static const std::array<ShortcutDefinition, 4> definitions = {
      ShortcutDefinition{ShortcutId::BrowseFiles, StrId::STR_BROWSE_FILES, UIIcon::Folder,
                         &CrossPointSettings::browseFilesShortcut, &CrossPointSettings::browseFilesShortcutOrder,
                         &CrossPointSettings::browseFilesShortcutVisible},
      ShortcutDefinition{ShortcutId::RecentBooks, StrId::STR_MENU_RECENT_BOOKS, UIIcon::Recent,
                         &CrossPointSettings::recentBooksShortcut, &CrossPointSettings::recentBooksShortcutOrder,
                         &CrossPointSettings::recentBooksShortcutVisible},
      ShortcutDefinition{ShortcutId::FileTransfer, StrId::STR_FILE_TRANSFER, UIIcon::Transfer,
                         &CrossPointSettings::fileTransferShortcut, &CrossPointSettings::fileTransferShortcutOrder,
                         &CrossPointSettings::fileTransferShortcutVisible},
      ShortcutDefinition{ShortcutId::Settings, StrId::STR_SETTINGS_TITLE, UIIcon::Settings,
                         &CrossPointSettings::settingsShortcut, &CrossPointSettings::settingsShortcutOrder,
                         &CrossPointSettings::settingsShortcutVisible},
  };
  return definitions;
}

inline const ShortcutDefinition* findShortcutDefinition(const ShortcutId id) {
  const auto& definitions = getShortcutDefinitions();
  auto it =
      std::find_if(definitions.begin(), definitions.end(), [id](const ShortcutDefinition& d) { return d.id == id; });
  return it != definitions.end() ? &(*it) : nullptr;
}

inline uint8_t getShortcutOrder(const ShortcutDefinition& definition, const CrossPointSettings& settings = SETTINGS) {
  return settings.*(definition.orderPtr);
}

inline bool getShortcutVisibility(const ShortcutDefinition& definition, const CrossPointSettings& settings = SETTINGS) {
  return settings.*(definition.visiblePtr) != 0;
}

inline uint8_t& getShortcutOrderRef(CrossPointSettings& settings, const ShortcutDefinition& definition) {
  return settings.*(definition.orderPtr);
}

inline uint8_t& getShortcutVisibilityRef(CrossPointSettings& settings, const ShortcutDefinition& definition) {
  return settings.*(definition.visiblePtr);
}

// Normalize order values so they are contiguous 0..N-1
inline void normalizeShortcutOrderSettings(CrossPointSettings& settings) {
  struct OrderSlot {
    int stableIndex;
    uint8_t* value;
  };

  std::vector<OrderSlot> slots;
  slots.reserve(getShortcutDefinitions().size() + 1);
  slots.push_back(OrderSlot{0, &settings.appsHubShortcutOrder});

  int stableIndex = 1;
  const auto& definitions = getShortcutDefinitions();
  std::transform(definitions.begin(), definitions.end(), std::back_inserter(slots),
                 [&](const ShortcutDefinition& definition) {
                   return OrderSlot{stableIndex++, &(settings.*(definition.orderPtr))};
                 });

  std::stable_sort(slots.begin(), slots.end(), [](const OrderSlot& lhs, const OrderSlot& rhs) {
    if (*lhs.value != *rhs.value) {
      return *lhs.value < *rhs.value;
    }
    return lhs.stableIndex < rhs.stableIndex;
  });

  for (size_t index = 0; index < slots.size(); ++index) {
    *slots[index].value = static_cast<uint8_t>(index);
  }
}

// Get shortcuts configured for a specific location, sorted by order
inline std::vector<const ShortcutDefinition*> getConfiguredShortcuts(
    const CrossPointSettings::SHORTCUT_LOCATION location) {
  std::vector<const ShortcutDefinition*> shortcuts;
  for (const auto& definition : getShortcutDefinitions()) {
    if (static_cast<CrossPointSettings::SHORTCUT_LOCATION>(SETTINGS.*(definition.locationPtr)) == location &&
        getShortcutVisibility(definition)) {
      shortcuts.push_back(&definition);
    }
  }
  std::stable_sort(shortcuts.begin(), shortcuts.end(),
                   [](const ShortcutDefinition* lhs, const ShortcutDefinition* rhs) {
                     return getShortcutOrder(*lhs) < getShortcutOrder(*rhs);
                   });
  return shortcuts;
}

// Home shortcut entry -- includes the Apps hub as a virtual entry
struct HomeShortcutEntry {
  const ShortcutDefinition* definition = nullptr;
  bool isAppsHub = false;
  bool isOpds = false;
};

inline std::vector<HomeShortcutEntry> getHomeShortcutEntries(const bool hasOpdsUrl) {
  std::vector<HomeShortcutEntry> entries;
  entries.push_back(HomeShortcutEntry{nullptr, true, false});

  for (const auto& definition : getShortcutDefinitions()) {
    const auto location = static_cast<CrossPointSettings::SHORTCUT_LOCATION>(SETTINGS.*(definition.locationPtr));
    if (location == CrossPointSettings::SHORTCUT_HOME && getShortcutVisibility(definition)) {
      entries.push_back(HomeShortcutEntry{&definition});
    }
  }

  std::stable_sort(entries.begin(), entries.end(), [](const HomeShortcutEntry& lhs, const HomeShortcutEntry& rhs) {
    const uint8_t lhsOrder = lhs.isAppsHub ? SETTINGS.appsHubShortcutOrder : getShortcutOrder(*lhs.definition);
    const uint8_t rhsOrder = rhs.isAppsHub ? SETTINGS.appsHubShortcutOrder : getShortcutOrder(*rhs.definition);
    return lhsOrder < rhsOrder;
  });

  if (hasOpdsUrl) {
    entries.push_back(HomeShortcutEntry{nullptr, false, true});
  }

  return entries;
}

inline std::string getHomeShortcutTitle(const HomeShortcutEntry& entry) {
  if (entry.isAppsHub) {
    return I18N.get(StrId::STR_APPS);
  }
  if (entry.isOpds) {
    return I18N.get(StrId::STR_OPDS_BROWSER);
  }
  if (!entry.definition) {
    return "";
  }
  return I18N.get(entry.definition->nameId);
}

inline UIIcon getHomeShortcutIcon(const HomeShortcutEntry& entry) {
  if (entry.isAppsHub) {
    return UIIcon::Book;
  }
  if (entry.isOpds) {
    return UIIcon::Library;
  }
  return entry.definition ? entry.definition->icon : UIIcon::Folder;
}
