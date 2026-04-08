# CrossPoint Korean Fork — 기능 통합 로드맵

> Created: 2026-04-08
> Base: CrossPoint Reader 1.2.0 → 1.2.0-ko.1

## 전략

CrossPoint를 베이스로 유지하면서 3개 커뮤니티 포크의 우수 기능을 선별 통합.

```
CrossPoint 1.2.0 (upstream, 3275★)
    │
    ├─ Korean Fork (우리)
    │   ├─ 한국어 i18n (22개 언어 중 하나)
    │   ├─ Character wrap (CJK 줄바꿈)
    │   ├─ Paragraph indent
    │   ├─ 한국어 내장 폰트 (Pretendard, KoPub)
    │   ├─ SD 카드 커스텀 폰트
    │   └─ KOReader/OPDS/WebDAV
    │
    ├─ ← CPR-vCodex (franssjz, 43★, 1.2.0 베이스 동일)
    │   └─ Dark Mode, Bookmarks, Stats, Heatmap, Achievements, Text Darkness
    │
    ├─ ← inx (obijuankenobiii, 36★, CrossPoint 포크)
    │   └─ 도서별 설정, .metadata 캐시, 상태 바 커스터마이징
    │
    └─ ← Papyrix (bigbag, 223★, 독자 진화)
        └─ 테스트 인프라, ScriptDetector, StreamingEpdFont
```

---

## Phase 1: Quick Wins (난이도 낮음)

**출처: CPR-vCodex** (같은 1.2.0 베이스 → cherry-pick 가능)

### 1.1 Dark Mode
- 글로벌 흑백 반전 (UI + 리더)
- `Settings > Display > Dark Mode` 토글
- 관련 커밋: `e682951` (feat: Add dark mode support)
- 예상 작업: 설정 추가 + 렌더러 반전 로직

### 1.2 Bookmarks
- EPUB 페이지 북마크 추가/제거
- 글로벌 북마크 목록 앱
- 관련: vCodex Bookmarks feature, inx도 유사 구현

### 1.3 Text Darkness
- AA 텍스트 진하게 렌더링
- crosspet 포크에서 원래 개발, vCodex가 차용
- 렌더링 파이프라인 수정 (GfxRenderer)

---

## Phase 2: Core Features (난이도 중)

### 2.1 도서별 설정 (출처: inx)
- 각 책마다 폰트/레이아웃/여백 별도 저장
- `.metadata/<hash>/setting.bin` 구조
- CrossPointSettings에 per-book override 레이어 추가
- 가장 실용적인 기능 — 기술서/소설 설정 자동 전환

### 2.2 Reading Stats + Heatmap (출처: vCodex)
- 읽기 세션 추적 (3분 이상만 카운트)
- 총 시간, 일별/7D/30D 트렌드
- 월간 히트맵 캘린더
- NTP WiFi 날짜 동기화 (Sync Day)
- `statistics.bin` per book
- Daily Goal (15/30/45/60분)

### 2.3 상태 바 커스터마이징 (출처: inx)
- 좌/중/우 3섹션 독립 설정
- 12가지 옵션: 페이지, 퍼센트, 챕터, 배터리, 진행 바, 제목, 저자 등
- 현재 CrossPoint는 고정 레이아웃

---

## Phase 3: Architecture (난이도 중~높)

**출처: Papyrix** (라이브러리 단위 포팅)

### 3.1 테스트 인프라
- CMake 기반 호스트 빌드 테스트 (79개 파일)
- Arduino, SdFat, FreeRTOS, EInkDisplay 모킹
- `test/CMakeLists.txt`, `test/mocks/`, `test/common/`
- 가장 큰 장기적 가치 — 리팩토링 안전망

### 3.2 ScriptDetector 라이브러리
- CJK, Thai, Arabic, Latin 스크립트 분류
- Hangul Syllables (U+AC00-D7AF) 포함
- 현재 inline CJK 체크를 라이브러리로 분리
- `lib/ScriptDetector/src/ScriptDetector.{h,cpp}`

### 3.3 StreamingEpdFont
- SD 카드에서 폰트 비트맵 스트리밍
- LRU 글리프 캐시 (O(1) 해시 룩업)
- ~45KB RAM 절약 (현재 ~70KB → ~25KB)
- ESP32-C3 메모리 제약 직접 해결

---

## Phase 4: Nice-to-have

### 4.1 Achievements (출처: vCodex)
- 62개 게이미피케이션 업적
- 읽기 데이터 기반 자동 해제
- Reading Stats 의존 (Phase 2.2 이후)

### 4.2 Home/Apps 커스터마이징 (출처: vCodex)
- 숏컷 위치/순서/표시 설정
- Lyra Custom 기본 테마

### 4.3 Sleep Tools (출처: vCodex)
- 슬립 화면 폴더 선택
- 순차/셔플 모드
- 프리뷰

---

## 의존성 그래프

```
Phase 1 (Quick Wins)
├── 1.1 Dark Mode
├── 1.2 Bookmarks
└── 1.3 Text Darkness

Phase 2 (Core Features)
├── 2.1 도서별 설정
├── 2.2 Reading Stats ← NTP Sync Day 필요
│   └── 2.2a Heatmap
└── 2.3 상태 바 커스터마이징

Phase 3 (Architecture)
├── 3.1 테스트 인프라 (독립, 언제든 착수 가능)
├── 3.2 ScriptDetector (독립)
└── 3.3 StreamingEpdFont ← SD 폰트 시스템 리팩토링 필요

Phase 4 (Nice-to-have)
├── 4.1 Achievements ← 2.2 의존
├── 4.2 Home/Apps 커스터마이징
└── 4.3 Sleep Tools
```

---

## 버전 계획

| 버전 | 내용 |
|------|------|
| `1.2.0-ko.1` | 현재 (upstream 1.2.0 동기화 완료) |
| `1.2.0-ko.2` | Phase 1 (Dark Mode, Bookmarks, Text Darkness) |
| `1.2.0-ko.3` | Phase 2 (도서별 설정, Stats, 상태 바) |
| `1.3.0-ko.1` | Phase 3 (테스트, ScriptDetector, StreamingFont) + upstream 동기화 |

---

## 출처 레포

| 포크 | URL | 베이스 | 핵심 가치 |
|------|-----|--------|-----------|
| CPR-vCodex | `franssjz/cpr-vcodex` | CP 1.2.0 | UX (Stats, Dark Mode, Bookmarks) |
| inx | `obijuankenobiii/inx` | CP 포크 | 데이터 (도서별 설정, .metadata 캐시) |
| Papyrix | `bigbag/papyrix-reader` | CP 독자 진화 | 아키텍처 (테스트, ScriptDetector, StreamingFont) |
| CN-type | `icannotttt/crosspoint-chinesetype` | CP 1.0.0 | CJK (TXT 챕터 감지) — 비공개 전환 예고 |
