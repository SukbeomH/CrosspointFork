# Upstream Sync Analysis: 1.2.0 -> upstream/master

> Created: 2026-04-09
> Branch: dev/upstream-sync-next
> Current base: 1.2.0 (tag) -> release/korean (1.3.0-ko.2)
> Target: upstream/master (12 commits ahead of 1.2.0)

## 1. Upstream Commits Since 1.2.0

| Hash | Type | Description | PR | Author | Impact |
|------|------|-------------|-----|--------|--------|
| `9b38851` | **feat** | Initial support for the X3 | #875 | Justin Mitchell | **HIGH** - HAL, GfxRenderer, themes |
| `1c13331` | fix | Support hyphenation for EPUBs using ISO 639-2 language codes | #1461 | Zach Nelson | MEDIUM - Section.cpp header |
| `11984f8` | refactor | Use C++20 `requires` in ActivityResult constructor | #1420 | Zach Nelson | LOW - clean merge |
| `f429f90` | refactor | Use default member initializers for JpegContext and PngContext | #1435 | Zach Nelson | LOW - clean merge |
| `fa3c7d9` | fix | Correct Russian auto-turn translations | #1566 | Andrei Ignatev | LOW - no conflict |
| `cff3e12` | fix | Update Ukrainian translations for footnotes | #1585 | Mirus | LOW - no conflict |
| `6cd19f5` | fix | EPUB images not rendering correctly on X3 | #1572 | Justin Mitchell | MEDIUM - image converters |
| `1398aeb` | **fix** | Use differential rounding for consistent inter-glyph spacing | #1413 | Zach Nelson | **HIGH** - GfxRenderer rewrite |
| `c656673` | refactor | logPrintf and predefined log level strings | #1546 | CSCMe | LOW - Logging only |
| `b898d53` | chore | Drop JPEGDEC patch in favour of upstream fix | #1465 | martin brook | LOW - platformio.ini |
| `d29b8ee` | feat | Adjust Navigation at End of Book | #1425 | nscheung | LOW - reader activities |
| `ed0811c` | fix | Fix failing very first WiFi connection attempt | #1521 | jpirnay | LOW - clean merge |

**Total: 12 commits, 37 files changed, +1130/-387 lines**

## 2. Dry-Run Conflict Analysis

Merge `upstream/master` into `release/korean` produces **3 conflicting files** with 6 conflict hunks:

### 2.1 Conflicting Files

#### `lib/GfxRenderer/GfxRenderer.h` (1 hunk)
- **Cause:** X3 support adds `panelWidth/panelHeight/panelWidthBytes/frameBufferSize` members + changes `bwBufferChunks` from fixed array to `std::vector` + changes `fontMap` type from `std::unique_ptr<UnifiedFontFamily>` to `EpdFontFamily`. Our fork adds `textDarkness`, `darkMode`, `fallbackFontId`, `fontDecompressor` members + uses `UnifiedFontFamily`.
- **Difficulty:** **HIGH** -- the `fontMap` type conflict (`UnifiedFontFamily` vs `EpdFontFamily`) is the same FontCacheManager issue from 1.2.0 merge. Requires architectural decision.
- **Resolution strategy:** Keep our `UnifiedFontFamily` wrapper (it extends `EpdFontFamily` for SD font support). Accept X3 panel dimension members. Accept `std::vector<uint8_t*>` for bwBufferChunks. Keep our darkMode/textDarkness/fallbackFontId additions.

#### `lib/GfxRenderer/GfxRenderer.cpp` (4 hunks)
- **Hunk 1 (line ~177):** X3 adds device-specific AA tuning (`gpio.deviceIsX4() && bmpVal == 2`). Our fork adds `syntheticBold` after BW pixel draw.
  - **Resolution:** Accept both -- they are in different branches of the conditional, combine the X3 condition with our syntheticBold logic.
- **Hunk 2 (line ~290):** Differential rounding rewrites `drawText()`. Our fork adds `letterSpacing` parameter overload + uses `EpdFontStyle` instead of `EpdFontFamily::Style`.
  - **Resolution:** Rebase our letterSpacing onto the new differential rounding code. Keep `EpdFontStyle` (our type alias).
- **Hunk 3 (line ~368):** Differential rounding removes manual `xPosFP` advance. Our fork adds `letterSpacing` accumulation here.
  - **Resolution:** Adapt letterSpacing to work with the new differential rounding accumulator.
- **Hunk 4 (line ~1129):** `getTextWidth()` -- upstream renames variable to `fontIt`, adds `prevAdvanceFP`. Our fork uses `widthFP` fixed-point accumulator.
  - **Resolution:** Adopt upstream's differential rounding approach, integrate our variable naming.

#### `lib/Epub/Epub/Section.cpp` (1 hunk)
- **Cause:** Our fork bumped `SECTION_FILE_VERSION` to 21 (added `paragraphIndent`, `characterWrap` fields to header). Upstream keeps version 19 but changes `HEADER_SIZE` calculation (X3 display dimension support).
- **Difficulty:** MEDIUM -- need to reconcile header layouts.
- **Resolution:** Keep our version 21 (superset of fields). Merge the HEADER_SIZE calculation to include both our extra fields AND upstream's layout changes. Bump to version 22.

### 2.2 Auto-Merged Files (no conflict, verify after merge)

| File | Upstream Change | Risk |
|------|-----------------|------|
| `lib/EpdFont/EpdFont.cpp` | Differential rounding | LOW -- our SdFont/StreamingEpdFont are separate files |
| `lib/EpdFont/EpdFontData.h` | X3 font data | LOW |
| `lib/Epub/Epub/converters/JpegToFramebufferConverter.cpp` | X3 image fix | LOW |
| `lib/Epub/Epub/converters/PngToFramebufferConverter.cpp` | X3 image fix | LOW |
| `lib/Epub/Epub/hyphenation/Hyphenator.cpp` | ISO 639-2 codes | LOW -- benefits Korean (ko = ISO 639-1) |
| `lib/hal/HalDisplay.cpp/h` | X3 panel support | LOW |
| `lib/hal/HalGPIO.cpp/h` | X3 GPIO + device detection | LOW |
| `lib/hal/HalPowerManager.cpp/h` | X3 power | LOW |
| `lib/JpegToBmpConverter/JpegToBmpConverter.cpp` | Refactoring | LOW |
| `lib/Logging/Logging.cpp/h` | logPrintf refactor | LOW |
| `lib/PngToBmpConverter/PngToBmpConverter.cpp` | Refactoring | LOW |
| `platformio.ini` | JPEGDEC patch removal | LOW -- our only change is version string |
| `src/activities/ActivityResult.h` | C++20 `requires` | LOW -- we didn't modify this file |
| `src/activities/home/HomeActivity.cpp` | End-of-book nav | LOW |
| `src/activities/network/WifiSelectionActivity.cpp` | WiFi fix | LOW |
| `src/activities/reader/EpubReaderActivity.cpp` | End-of-book nav | **MEDIUM** -- we modified this file significantly |
| `src/activities/reader/TxtReaderActivity.cpp` | End-of-book nav | **MEDIUM** -- we modified this file |
| `src/activities/reader/XtcReaderActivity.cpp` | End-of-book nav | LOW |
| `src/components/themes/BaseTheme.cpp` | X3 theme | LOW |
| `src/components/themes/lyra/LyraTheme.cpp` | X3 theme | LOW |
| `src/main.cpp` | X3 init + WiFi fix | **MEDIUM** -- heavily modified in our fork |
| `src/util/ScreenshotUtil.cpp` | X3 screenshot | LOW |
| `lib/I18n/translations/russian.yaml` | Translation fix | LOW |
| `lib/I18n/translations/ukrainian.yaml` | Translation fix | LOW |

### 2.3 New Files from Upstream (no conflict)

| File | Description |
|------|-------------|
| `test/differential_rounding/DifferentialRoundingTest.cpp` | New test for glyph spacing |
| `test/run_differential_rounding_test.sh` | Test runner script |

### 2.4 Deleted Files by Upstream

| File | Description | Impact |
|------|-------------|--------|
| `scripts/patch_jpegdec.py` | JPEGDEC patch script (replaced by upstream fix) | LOW -- we don't use this |

## 3. Korean Customization Files to Preserve

These files contain Korean-specific code and MUST be preserved during merge:

### 3.1 Korean Font System (entirely our additions -- no conflict risk)
- `lib/EpdFont/SdFont.cpp/h` -- SD card font loading
- `lib/EpdFont/SdFontFamily.cpp/h` -- SD font family wrapper
- `lib/EpdFont/SdFontFormat.h` -- SD font binary format
- `lib/EpdFont/StreamingEpdFont.cpp/h` -- Memory-efficient font streaming
- `lib/EpdFont/builtinFonts/kopub_14_regular.h` -- KoPub Batang built-in
- `lib/EpdFont/builtinFonts/pretendard_10_regular.h` -- Pretendard built-in
- `lib/EpdFont/builtinFonts/source/` -- Korean TrueType source files
- `lib/EpdFont/scripts/` -- Font build/conversion scripts

### 3.2 Korean i18n
- `lib/I18n/translations/korean.yaml` -- Korean translation file
- `lib/I18n/I18n.cpp/h` -- Modified to include Korean

### 3.3 CJK Text Handling
- `lib/ScriptDetector/ScriptDetector.cpp/h` -- CJK/Hangul script detection
- `lib/Epub/Epub/ParsedText.cpp/h` -- `characterWrap` for CJK line-breaking
- `lib/Epub/Epub/parsers/ChapterHtmlSlimParser.cpp/h` -- `paragraphIndent` support
- `lib/Epub/Epub/Section.cpp/h` -- Extended section header (version 21)

### 3.4 Font Infrastructure (shared, requires careful merge)
- `lib/GfxRenderer/GfxRenderer.cpp/h` -- `UnifiedFontFamily`, letterSpacing, syntheticBold, darkMode
- `lib/GfxRenderer/FontCacheManager.cpp/h` -- Modified for UnifiedFontFamily
- `lib/EpdFont/EpdFontFamily.cpp/h` -- Extended for Korean font support
- `src/fontIds.h` -- Korean font ID definitions
- `src/FontManager.h` -- Font loading management

### 3.5 Fork Features (our additions -- no conflict risk)
- `src/AchievementStore.h` -- Achievement system
- `src/BookSettings.h` -- Per-book settings
- `src/ReadingStats.h` -- Reading statistics
- `src/activities/reader/BookmarkStore.h` -- Bookmarks
- `src/activities/settings/CategorySettingsActivity.cpp/h` -- Settings categories
- `src/activities/settings/FontSelectionActivity.cpp/h` -- Font picker
- `src/util/SleepImageUtils.h` -- Sleep screen utilities

## 4. Risk Assessment

### High Risk (manual resolution required)
1. **GfxRenderer fontMap type conflict** -- `UnifiedFontFamily` vs `EpdFontFamily`. Same issue as 1.2.0 merge. Our `UnifiedFontFamily` wraps `EpdFontFamily` with SD font support; upstream uses `EpdFontFamily` directly. Must keep our wrapper.
2. **Differential rounding + letterSpacing** -- Upstream rewrites the glyph positioning math in `drawText()` and `getTextWidth()`. Our `letterSpacing` parameter must be adapted to work with the new 12.4 fixed-point differential rounding system.

### Medium Risk (straightforward but careful merge)
3. **Section.cpp header layout** -- Both sides changed `HEADER_SIZE`. Need combined layout, bump to version 22. Existing caches will be invalidated (expected and acceptable).
4. **EpubReaderActivity.cpp / TxtReaderActivity.cpp** -- Auto-merged but both sides modified. Need post-merge review to verify our bookmarks/reading-stats hooks coexist with upstream's end-of-book navigation.
5. **main.cpp** -- Heavily modified on both sides. Auto-merged but verify X3 init doesn't conflict with our font loading, dark mode, and achievement initialization.
6. **X3 panel dimensions in GfxRenderer.h** -- New members for dynamic panel sizing. Our code assumes fixed `DISPLAY_WIDTH/HEIGHT`. Need to verify our rendering code handles variable panel sizes.

### Low Risk (auto-merge or no overlap)
7. Translation files (russian.yaml, ukrainian.yaml) -- no conflict with korean.yaml
8. HAL layer changes (X3 GPIO, power, display) -- our fork doesn't modify HAL
9. C++20 `requires` refactoring -- already have `-std=gnu++2a` in build flags
10. JPEGDEC patch removal -- we don't use the patch script
11. Logging refactoring -- no overlap with our code
12. Image converter refactoring -- no overlap
13. New test files -- purely additive

## 5. Recommended Merge Strategy

### Approach: `merge --no-ff` with manual conflict resolution

**Why merge (not rebase):**
- Our fork has 30+ commits of Korean customization since 1.2.0
- Rebasing would require resolving conflicts for each commit individually
- Merge preserves our commit history cleanly
- Previous 1.2.0 merge used the same approach successfully

### Pre-Merge Preparation

1. **FontCacheManager TODO resolution** -- Before merging, resolve the `FontCacheManager` + `UnifiedFontFamily` compatibility (TODO from 1.2.0 merge). This is the #1 blocker.
2. **Review differential rounding PR #1413** -- Understand the new glyph positioning math before trying to integrate letterSpacing.
3. **Create test branch** -- Do the merge on a disposable branch first to iterate on conflict resolution.

### Merge Steps

```bash
# 1. Create merge branch from release/korean
git checkout -b dev/upstream-sync-merge release/korean

# 2. Merge upstream/master
git merge --no-ff upstream/master

# 3. Resolve 3 conflicting files:
#    a. GfxRenderer.h -- keep UnifiedFontFamily + add X3 panel members
#    b. GfxRenderer.cpp -- integrate letterSpacing with differential rounding
#    c. Section.cpp -- combine header layouts, bump version to 22

# 4. Post-merge verification (build + manual review):
#    a. Verify EpubReaderActivity.cpp bookmarks work with end-of-book nav
#    b. Verify TxtReaderActivity.cpp reading stats work
#    c. Verify main.cpp initialization order (X3 + our features)
#    d. Verify Korean font rendering (syntheticBold, letterSpacing)
#    e. Run gen_i18n.py to verify korean.yaml still compiles

# 5. Build test (CI -- local build blocked by corporate SSL proxy)
```

### Conflict Resolution Cheatsheet

| File | Keep Ours | Keep Theirs | Manual Merge |
|------|-----------|-------------|--------------|
| `GfxRenderer.h` | UnifiedFontFamily, darkMode, textDarkness, fallbackFontId | X3 panel members, vector bwBufferChunks | Combine both |
| `GfxRenderer.cpp` hunk 1 | syntheticBold | X3 AA tuning | Combine conditions |
| `GfxRenderer.cpp` hunk 2 | letterSpacing overload, EpdFontStyle | Differential rounding math | Adapt letterSpacing to new math |
| `GfxRenderer.cpp` hunk 3 | letterSpacing accumulation | (removed manual advance) | Adapt to new accumulator |
| `GfxRenderer.cpp` hunk 4 | (our naming) | Differential rounding width calc | Adopt upstream, integrate |
| `Section.cpp` | Version 21, extra header fields | New header layout | Combine, bump to v22 |

### Post-Merge Version

Update to `1.4.0-ko.1` (upstream sync = minor version bump).

## 6. Estimated Effort

| Task | Time | Notes |
|------|------|-------|
| FontCacheManager prep | 2-4h | Prerequisite TODO from 1.2.0 |
| Conflict resolution (3 files) | 1-2h | GfxRenderer is the main work |
| Post-merge review (auto-merged files) | 1h | Verify reader activities, main.cpp |
| Build verification (CI) | 0.5h | Push and check CI results |
| **Total** | **4.5-7.5h** | One focused session |

## 7. Known Patterns from Previous Merge (1.2.0)

From memory `project_upstream_sync.md`:

| Pattern | 1.2.0 | Next Merge |
|---------|-------|------------|
| HAL wrapping (FsFile -> HalFile) | Major issue | Not expected (HAL stable since 1.2.0) |
| Activity restructuring | ActivityWithSubactivity removed | Minor (ActivityResult `requires` keyword) |
| FontCacheManager type mismatch | Disabled (TODO) | **Still unresolved** -- same conflict pattern |
| i18n key changes | gen_i18n.py reorder | No changes this time |

## 8. Upstream SD Font PRs to Watch

These upstream PRs were mentioned as potential conflicts with our SdFont/StreamingEpdFont:

| PR | Title | Status | Impact |
|----|-------|--------|--------|
| #1327 | SD card font loading | Unknown | **HIGH** -- direct overlap with our SdFont implementation |
| #1392 | Font improvements | Unknown | **HIGH** -- may conflict with StreamingEpdFont |

**Action:** Before merging, check if these PRs were merged into upstream/master or are still pending. If merged, our SdFont/StreamingEpdFont may need significant rework or could be replaced by upstream's implementation.

```bash
# Check PR status
gh pr view 1327 --repo crosspoint-reader/crosspoint-reader --json state
gh pr view 1392 --repo crosspoint-reader/crosspoint-reader --json state
```
