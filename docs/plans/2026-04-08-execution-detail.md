# Execution Detail — Community Fork Integration

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 4 Phase, 13 Task를 효율적으로 실행하기 위한 세부 계획.
병렬 에이전트, 워크트리 격리, 의존 관계를 고려.

**Hardware Constraints:** `.hxsk/docs/hardware-constraints.md` 필수 참조

---

## 의존 관계 & 병렬성 분석

### Phase 1 Task 간 의존

```
Task 1 (Dark Mode)     ─── GfxRenderer.h/.cpp, Settings 수정
Task 2 (Bookmarks)     ─── EpubReaderActivity, 새 파일 4개
Task 3 (Text Darkness) ─── GfxRenderer.cpp (renderCharImpl)

충돌 분석:
  Task 1 ↔ Task 3: GfxRenderer 동시 수정 → 순차 필요
  Task 1 ↔ Task 2: EpubReaderActivity 일부 겹침 → 순차 권장
  Task 2 ↔ Task 3: 독립 → 병렬 가능

최적 실행 순서:
  Wave A: Task 2 (Bookmarks) — 독립, 새 파일 위주
  Wave A: Task 3 (Text Darkness) — 독립, 소규모 변경
  Wave B: Task 1 (Dark Mode) — A 완료 후, GfxRenderer 최종 통합
```

### Phase 2 Task 간 의존

```
Task 4 (도서별 설정)     ─── Settings, ReaderActivity
Task 5 (Reading Stats)  ─── 새 파일, NTP 동기화
Task 6 (상태 바 커스텀)  ─── BaseTheme, UITheme

충돌 분석:
  Task 4 ↔ Task 5: Settings 동시 수정 → 순차 권장
  Task 5 ↔ Task 6: 독립 → 병렬 가능
  Task 4 ↔ Task 6: 독립 → 병렬 가능

최적 실행 순서:
  Wave A: Task 4 (도서별 설정) + Task 6 (상태 바) — 병렬
  Wave B: Task 5 (Reading Stats) — Settings 충돌 해소 후
```

### Phase 3 Task 간 의존

```
Task 7 (테스트 인프라) ─── test/ 디렉토리, CMake (독립)
Task 8 (ScriptDetector) ─── lib/ 새 라이브러리 + ParsedText 수정
Task 9 (StreamingFont)  ─── lib/EpdFont 리팩토링

충돌 분석: 모두 독립 → 전체 병렬 가능

최적 실행 순서:
  Wave A: Task 7 + Task 8 + Task 9 — 전체 병렬 (워크트리 3개)
```

---

## Phase 1 세부 실행 계획

### 준비

```bash
# vCodex 리모트 추가 (아직 없다면)
git remote add vcodex https://github.com/franssjz/cpr-vcodex.git
git fetch vcodex

# Phase 1 베이스 브랜치 생성
git checkout -b dev/phase1-quick-wins release/korean
```

### Wave A (병렬 — 워크트리 2개)

#### Worktree 1: Task 2 — Bookmarks

```bash
git worktree add ../crosspoint-wt-bookmarks dev/phase1-quick-wins
```

**에이전트 지시:**
1. vCodex에서 bookmark 관련 파일 추출 (`git show vcodex/master:path`)
2. 새 파일 생성:
   - `src/activities/reader/BookmarkStore.h`
   - `src/activities/reader/BookmarksActivity.h/.cpp`
   - `src/activities/apps/BookmarksAppActivity.h/.cpp` (선택)
   - `src/util/BookIdentity.h/.cpp` (이미 있으면 skip)
3. 기존 파일 수정:
   - `EpubReaderActivity.h/.cpp` — BookmarkStore 멤버, long-press 토글
   - `EpubReaderMenuActivity` — BOOKMARKS 메뉴 옵션
4. i18n: `korean.yaml`, `english.yaml`에 키 추가
5. `clang-format-fix` 실행
6. 커밋: `feat: add EPUB bookmark support`

**충돌 주의:** EpubReaderActivity는 ko 포크에서 수정됨. vCodex 코드를 그대로 복사하지 말고, 우리 코드에 로직을 삽입.

**Verify:** CI 빌드 통과

---

#### Worktree 2: Task 3 — Text Darkness

```bash
git worktree add ../crosspoint-wt-textdarkness dev/phase1-quick-wins
```

**에이전트 지시:**
1. vCodex/crosspet에서 text darkness 관련 코드 분석
2. 수정:
   - `src/CrossPointSettings.h` — `uint8_t textDarkness = 0` 추가
   - `src/SettingsList.h` — Display 카테고리에 토글 추가
   - `lib/GfxRenderer/GfxRenderer.cpp` — `renderCharImpl()` 내 BW 판정 임계값 조건 추가
3. syntheticBold 가드: `textDarkness && syntheticBold` 동시 활성 시 처리
4. i18n: `STR_TEXT_DARKNESS` 키 추가
5. `clang-format-fix` 실행
6. 커밋: `feat: add text darkness option`

**Verify:** CI 빌드 통과

---

### Wave A 완료 후 통합

```bash
# 워크트리에서 작업 완료 확인
cd ../crosspoint-wt-bookmarks && git log --oneline -3
cd ../crosspoint-wt-textdarkness && git log --oneline -3

# 메인 브랜치에 순차 머지
cd /path/to/CrosspointFork
git checkout dev/phase1-quick-wins
git merge crosspoint-wt-bookmarks --no-edit
git merge crosspoint-wt-textdarkness --no-edit

# 워크트리 정리
git worktree remove ../crosspoint-wt-bookmarks
git worktree remove ../crosspoint-wt-textdarkness
```

### Wave B (순차 — 메인 브랜치)

#### Task 1 — Dark Mode

Wave A의 GfxRenderer 변경이 완료된 상태에서 진행.

**에이전트 지시:**
1. vCodex 커밋 3개 분석 (`e682951`, `386ae0f`, `bf34c6d`)
2. 렌더링 코어 수정 (GfxRenderer):
   - `darkMode` 필드, `setDarkMode()`/`isDarkMode()`
   - `drawPixel` 반전 로직
   - `clearScreen`, `drawBitmap`, `displayBuffer` 반전 지원
3. HAL 수정 (HalDisplay): `invert` 파라미터
4. 설정 추가: `CrossPointSettings.h`, `SettingsList.h`
5. 리더 통합: EpubReaderActivity (AA bypass, 이미지), ReaderUtils
6. 부트/슬립: 임시 비활성화 로직
7. main.cpp: 초기화 + 루프 동기화
8. i18n: `STR_DARK_MODE` 키 추가
9. `clang-format-fix` 실행
10. 커밋: `feat: add dark mode toggle`

**충돌 주의:** Wave A에서 수정된 GfxRenderer, Settings와 충돌 가능 → 수동 해결

**Verify:** CI 빌드 통과

### Phase 1 마무리

```bash
# Phase 1 통합 검증
./bin/clang-format-fix
git push origin dev/phase1-quick-wins

# CI 통과 확인 후
# PR: dev/phase1-quick-wins → release/korean
gh pr create --base release/korean --head dev/phase1-quick-wins \
  --title "feat: Phase 1 — Dark Mode, Bookmarks, Text Darkness"

# CI 통과 + 코드 리뷰 후 머지
# 버전 범프: 1.2.0-ko.2
```

---

## Phase 2 세부 실행 계획

### 준비

```bash
git checkout -b dev/phase2-core-features release/korean
```

### Wave A (병렬 — 워크트리 2개)

#### Worktree 1: Task 4 — 도서별 설정

**에이전트 지시:**
1. inx의 `setting.bin` 구현 분석
2. `BookSettings` 클래스 생성 (CrossPointSettings의 subset)
3. ReaderActivity에서 onEnter 시 도서별 설정 로드, onExit 시 저장
4. 글로벌 설정 fallback 로직
5. 캐시 디렉토리 구조: `.crosspoint/epub_<hash>/setting.bin`

#### Worktree 2: Task 6 — 상태 바 커스터마이징

**에이전트 지시:**
1. inx의 상태 바 구현 분석
2. `StatusBarConfig` (좌/중/우 3섹션)
3. BaseTheme 렌더링 수정
4. Settings UI 추가

### Wave B (순차)

#### Task 5 — Reading Stats + Heatmap

**에이전트 지시:**
1. vCodex Stats/Heatmap/Sync Day 분석
2. NTP 날짜 동기화 구현
3. 세션 추적 (3분 임계, deep sleep 협력)
4. `statistics.bin` per book
5. Stats/Heatmap Activity 생성
6. Daily Goal 설정

### Phase 2 마무리

```bash
# 워크트리 통합 → CI → PR → 머지 → 버전 1.2.0-ko.3
```

---

## Phase 3 세부 실행 계획

### 전체 병렬 (워크트리 3개)

```bash
git worktree add ../crosspoint-wt-tests dev/phase3-architecture
git worktree add ../crosspoint-wt-scriptdetector dev/phase3-architecture
git worktree add ../crosspoint-wt-streamingfont dev/phase3-architecture
```

#### Worktree 1: Task 7 — 테스트 인프라

**에이전트 지시:**
1. Papyrix `test/` 디렉토리 분석 및 포팅
2. `test/CMakeLists.txt` 생성
3. `test/mocks/` (Arduino, SdFat, FreeRTOS)
4. 기존 코드에 대한 기본 테스트 5개 작성

#### Worktree 2: Task 8 — ScriptDetector

**에이전트 지시:**
1. Papyrix `lib/ScriptDetector/` 포팅
2. 기존 inline CJK 체크 대체
3. 테스트 작성

#### Worktree 3: Task 9 — StreamingEpdFont

**에이전트 지시:**
1. Papyrix `StreamingEpdFont` 분석 및 포팅
2. 기존 SdFont/SdFontFamily와 통합
3. LRU 캐시 구현
4. RAM 사용량 비교 검증

### Phase 3 마무리

```bash
# 3개 워크트리 순차 머지 → CI → PR → 머지
# upstream 동기화 확인 → 버전 1.3.0-ko.1
```

---

## Phase 4 세부 (선택)

독립 태스크. 필요 시 워크트리에서 개별 실행.

| Task | 병렬 가능 | 워크트리 | 비고 |
|------|-----------|----------|------|
| 10 Achievements | 독립 (Stats 의존) | 1개 | Phase 2 완료 후 |
| 11 Home/Apps | 독립 | 1개 | 언제든 |
| 12 Sleep Tools | 독립 | 1개 | 언제든 |
| 13 EPUB 최적화 | 독립 | 1개 | 브라우저 JS만, 언제든 |

---

## 실행 타임라인 (세션 기준)

```
Session N: Phase 1
├── Wave A: Agent-Bookmarks ∥ Agent-TextDarkness (워크트리 병렬)
├── Merge Wave A
├── Wave B: Agent-DarkMode (메인 순차)
├── CI 검증 + 코드 리뷰
└── 버전 범프 1.2.0-ko.2, 태그, 푸시

Session N+1: Phase 2
├── Wave A: Agent-PerBookSettings ∥ Agent-StatusBar (워크트리 병렬)
├── Merge Wave A
├── Wave B: Agent-ReadingStats (메인 순차)
├── CI 검증 + 코드 리뷰
└── 버전 범프 1.2.0-ko.3, 태그, 푸시

Session N+2: Phase 3
├── Wave A: Agent-Tests ∥ Agent-ScriptDetector ∥ Agent-StreamingFont (3병렬)
├── Merge All
├── upstream 동기화 확인
├── CI 검증 + 코드 리뷰
└── 버전 범프 1.3.0-ko.1, 태그, 푸시

Session N+3~: Phase 4 (선택, 개별)
```

---

## 에이전트 파견 템플릿

각 Task 실행 시 사용:

```
Agent(
  description: "Task N: [feature name]",
  isolation: "worktree",
  prompt: """
  CrossPoint Korean Fork에 [feature] 기능을 포팅합니다.
  
  ## Context
  - Working dir: [worktree path]
  - Branch: dev/phase[N]-[name]
  - 출처 레포: [repo] — [관련 코드/커밋]
  - 사전 조사: .hxsk/docs/phase[N]-research.md
  - 하드웨어 제약: .hxsk/docs/hardware-constraints.md
  
  ## Task
  [구체적 지시사항]
  
  ## Rules
  - 새 기능 상주 RAM < 10KB
  - SD-first 캐싱 (RAM에 데이터 보유 금지)
  - clang-format-fix 실행 후 커밋
  - korean.yaml + english.yaml에 i18n 키 추가
  - .hxsk/ 변경사항 포함하여 커밋
  
  ## Verify
  - CI 빌드 통과 기준 (clang-format, cppcheck, build)
  - [기능별 검증 항목]
  """
)
```

---

## 참고 자료

- 상위 계획: `docs/plans/2026-04-08-community-fork-integration.md`
- Phase 1 조사: `.hxsk/docs/phase1-research.md`
- 하드웨어 제약: `.hxsk/docs/hardware-constraints.md`
- 로드맵: `.hxsk/docs/integration-roadmap.md`
- 아키텍처: `.hxsk/ARCHITECTURE.md`
