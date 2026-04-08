# Phase 2 사전 조사 결과

> 2026-04-08 조사. 출처: obijuankenobiii/inx

## Task 4: Per-Book Settings (도서별 설정)

### inx 구현
- **핵심 파일:** `src/state/BookSetting.h` (~220줄)
- **구조:** `BookSettings` struct — 폰트, 여백, 방향, 상태 바 등 14바이트
- **저장:** `/.metadata/epub/<hash>/settings.bin` (바이너리)
- **로직:** `loadFromFile()` / `saveToFile()` / `loadFromGlobalSettings()` (fallback)
- **플래그:** `useCustomSettings` — 도서별 오버라이드 여부 추적

### 우리 코드와의 갭
- `BookSettings` 구조 없음, 글로벌 `CrossPointSettings` 싱글톤만 사용
- epub 캐시 경로 (`Epub::getCachePath()`)는 이미 존재

### 구현 계획
1. `BookSettings` struct 생성 (CrossPointSettings subset)
2. `EpubReaderActivity`에 `BookSettings` 멤버 추가
3. onEnter: 도서별 설정 로드 (없으면 글로벌 fallback)
4. onExit/설정변경: 도서별 설정 저장
5. 저장 경로: `.crosspoint/epub_<hash>/settings.bin`

### 복잡도: 중

---

## Task 5: Reading Stats + Heatmap (vCodex)

### 별도 조사 필요 (Phase 2 시작 시)
- vCodex의 Stats/Heatmap/Sync Day 코드 분석
- NTP 동기화, 세션 타이머, daily goal

---

## Task 6: Status Bar Customization (상태 바 커스터마이징)

### inx 구현
- **핵심 파일:** `StatusBar.h/.cpp`, `SettingsDrawer.cpp`
- **12개 옵션 enum:** NONE, PAGE_NUMBERS, PERCENTAGE, CHAPTER_TITLE, BATTERY_ICON, BATTERY_PERCENTAGE, BATTERY_ICON_WITH_PERCENT, PROGRESS_BAR, PROGRESS_BAR_WITH_PERCENT, PAGE_BARS, BOOK_TITLE, AUTHOR_NAME
- **3섹션:** `statusBarLeft`, `statusBarMiddle`, `statusBarRight` (각 1바이트)
- **렌더링:** `StatusBar::renderSection()` — switch on item type

### 우리 코드 현황
- 6개 토글/enum: `statusBarChapterPageCount`, `statusBarBookProgressPercentage`, `statusBarProgressBar`, `statusBarProgressBarThickness`, `statusBarTitle`, `statusBarBattery`
- `StatusBarSettingsActivity.cpp`에서 관리
- `GUI.drawStatusBar()`에서 렌더링

### 구현 옵션
- **Option A (inx 모델):** 6개 토글 → 3섹션 + enum으로 교체. 단순하지만 thickness 컨트롤 손실.
- **Option B (하이브리드):** 기존 컨트롤 유지 + 섹션 배치 추가. 복잡하지만 기존 기능 보존.
- **권장: Option A** — 직관적 UX, inx 패턴 검증됨

### 중요: 도서별 설정과 연동
inx는 상태 바 설정도 `BookSettings`에 포함 (도서별 상태 바).
Task 4와 함께 구현하면 시너지.

### 복잡도: 중~높
