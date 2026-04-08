# Phase 1 사전 조사 결과

> 2026-04-08 조사. 각 Task 시작 시 세부 구현 구체화 참고용.

## Task 1: Dark Mode (vCodex)

### 적용 커밋 순서
1. `e682951` — 핵심 dark mode 구현
2. `386ae0f` — 리프레시 개선 + deep sleep wake 시 부트 스킵
3. `bf34c6d` — AA 페이지 리프레시 정제

### 수정 파일 (충돌 위험도)
| 파일 | 변경 | 충돌 위험 |
|------|------|-----------|
| `lib/GfxRenderer/GfxRenderer.h/.cpp` | darkMode 필드, drawPixel 반전, clearScreen | 낮음 |
| `lib/hal/HalDisplay.h/.cpp` | invert 파라미터 추가 | 낮음 |
| `lib/Epub/Epub/Page.h/.cpp` | renderImages() | 낮음 |
| `lib/Epub/Epub/converters/DirectPixelWriter.h` | darkMode BW 로직 | 낮음 |
| `src/CrossPointSettings.h` | `uint8_t darkMode = 0` | 낮음 |
| `src/SettingsList.h` | 토글 추가 | 낮음 |
| `src/activities/reader/EpubReaderActivity.cpp` | AA bypass, 이미지 렌더링 | **중** (characterWrap 근처) |
| `src/activities/reader/ReaderUtils.h` | dark mode aware refresh | 낮음 |
| `src/activities/boot_sleep/` | 부트/슬립 시 임시 비활성화 | 낮음 |
| `src/main.cpp` | init + loop sync | 낮음 |
| `korean.yaml` | `STR_DARK_MODE` 추가 | 없음 |

### 권장 방식
cherry-pick이 아닌 **파일별 수동 포팅**. 렌더링 코어 → 설정 → 리더 순서.

---

## Task 2: Bookmarks (vCodex)

### 새 파일 (4개)
- `src/activities/reader/BookmarkStore.h` — 바이너리 저장소
- `src/activities/reader/BookmarksActivity.h/.cpp` — 인리더 북마크 UI
- `src/activities/apps/BookmarksAppActivity.h/.cpp` — 독립 앱

### 의존 파일 (2개, 함께 포팅)
- `src/util/BookIdentity.h/.cpp` — 도서별 데이터 경로
- `src/util/ButtonNavigator.h/.cpp` — 페이지네이션 헬퍼 (이미 존재 확인 필요)

### 기존 파일 수정
- `EpubReaderActivity.h/.cpp` — BookmarkStore 멤버, long-press 토글, 메뉴 연동
- `EpubReaderMenuActivity` — BOOKMARKS 메뉴 옵션
- `korean.yaml` — `STR_BOOKMARK_ADDED`, `STR_BOOKMARK_REMOVED` 등

### 저장 포맷
`bookmarks.bin`: `[version:u8][count:u16][{spineIndex:u16, pageNumber:u16, snippetLen:u8, snippet:char[80]}...]`
최대 1000개, 도서별 저장.

### 충돌 위험
**중~높** — `EpubReaderActivity`가 ko 포크에서 많이 수정됨 (characterWrap, paragraphIndent, customFont). 수동 머지 필요.

---

## Task 3: Text Darkness (vCodex/crosspet)

### 개요
2-bit AA 그레이 픽셀 임계값을 조정하여 BW 모드에서 더 진하게 렌더링.

### 수정 예상
- `src/CrossPointSettings.h` — `uint8_t textDarkness = 0` 추가
- `lib/GfxRenderer/GfxRenderer.cpp` — `renderCharImpl()` 내 BW 판정 임계값 변경
- `src/SettingsList.h` — 토글 추가

### 한국어 포크 주의
ko 포크의 `syntheticBold` (screenX+1 픽셀 복제)와 동시 활성화 시 시각적 무게 과도할 수 있음 → 가드 필요.

### 충돌 위험
낮음 — 렌더링 로직 내 독립적 변경.
