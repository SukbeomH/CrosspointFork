# Community Fork Feature Integration Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** CrossPoint Korean Fork에 커뮤니티 포크(vCodex, inx, Papyrix)의 우수 기능을 단계적으로 통합하여 최고의 한국어 e-reader 펌웨어를 만든다.

**Architecture:** CrossPoint 1.2.0을 베이스로 유지. vCodex는 같은 1.2.0 기반이므로 cherry-pick/머지, inx는 선별 포팅, Papyrix는 라이브러리 단위 포팅. 각 Phase는 독립 dev 브랜치에서 작업 → CI 통과 → pre-release → release 머지.

**Tech Stack:** C++ (gnu++2a), PlatformIO, ESP32-C3, FreeRTOS, Arduino Framework

---

## Phase 1: Quick Wins → `1.2.0-ko.2`

**Branch:** `dev/phase1-quick-wins`
**출처:** CPR-vCodex (`franssjz/cpr-vcodex`, 같은 CP 1.2.0 베이스)
**예상 작업량:** 중소

### Task 1: Dark Mode

**Goal:** 글로벌 흑백 반전 (UI + 리더 렌더링)
**출처 레포:** `franssjz/cpr-vcodex` — dark mode 관련 커밋들
**세부 구현:** Phase 1 시작 시 vCodex 코드 분석 후 구체화

**Verify:**
- Settings에서 Dark Mode 토글 동작
- UI 화면 반전 렌더링 확인
- EPUB 리더 텍스트/이미지 반전 확인
- CI 빌드 통과

**Commit:** `feat: add dark mode toggle`

---

### Task 2: EPUB Bookmarks

**Goal:** 페이지 북마크 추가/제거 + 북마크 목록 뷰
**출처 레포:** `franssjz/cpr-vcodex` — bookmark 관련 코드
**세부 구현:** Phase 1 시작 시 구체화

**Verify:**
- OK 버튼 길게 눌러 북마크 추가
- 북마크 목록에서 선택하여 해당 페이지로 이동
- 북마크 데이터 SD 카드 저장/로드
- CI 빌드 통과

**Commit:** `feat: add EPUB bookmark support`

---

### Task 3: Text Darkness

**Goal:** AA 텍스트 진하게 렌더링 (가독성 개선)
**출처 레포:** `franssjz/cpr-vcodex` (원본: `trilwu/crosspet`)
**세부 구현:** Phase 1 시작 시 구체화

**Verify:**
- Settings에서 Text Darkness 토글
- 리더에서 텍스트 렌더링 비교 (on/off)
- 한국어 폰트(KoPub/Pretendard)와의 호환성
- CI 빌드 통과

**Commit:** `feat: add text darkness option`

---

### Phase 1 완료 조건
- [ ] 3개 기능 모두 Settings에서 접근 가능
- [ ] 한국어 i18n에 새 설정 문자열 추가
- [ ] CI 전체 통과 (build, clang-format, cppcheck)
- [ ] `korean.yaml`에 새 키 번역 추가
- [ ] 버전 범프 `1.2.0-ko.2`
- [ ] release/korean 머지 + 태그

---

## Phase 2: Core Features → `1.2.0-ko.3`

**Branch:** `dev/phase2-core-features`
**출처:** inx + CPR-vCodex
**의존:** Phase 1 완료

### Task 4: 도서별 설정 (Per-Book Settings)

**Goal:** 책마다 폰트/레이아웃/여백 개별 저장
**출처 레포:** `obijuankenobiii/inx` — `setting.bin` per book
**세부 구현:** Phase 2 시작 시 구체화

**Verify:**
- 책 A에서 폰트 변경 → 책 B는 기본 설정 유지
- 책 A 재진입 시 변경된 설정 자동 로드
- 설정 없는 책은 글로벌 설정 fallback
- CI 빌드 통과

---

### Task 5: Reading Stats + Heatmap

**Goal:** 읽기 시간 추적, 통계 대시보드, 월간 히트맵
**출처 레포:** `franssjz/cpr-vcodex` — Stats/Heatmap/Sync Day
**의존:** NTP 날짜 동기화 구현 필요
**세부 구현:** Phase 2 시작 시 구체화

**Verify:**
- 3분 이상 읽기 세션 자동 기록
- Stats 화면에서 총 시간/도서 수 표시
- Heatmap에서 월간 읽기 강도 시각화
- CI 빌드 통과

---

### Task 6: 상태 바 커스터마이징

**Goal:** 좌/중/우 3섹션 독립 설정 (12가지 옵션)
**출처 레포:** `obijuankenobiii/inx` — status bar customization
**세부 구현:** Phase 2 시작 시 구체화

**Verify:**
- Settings에서 각 섹션별 표시 항목 선택
- 리더에서 설정된 대로 상태 바 표시
- CI 빌드 통과

---

### Phase 2 완료 조건
- [ ] 도서별 설정 동작 확인
- [ ] Stats/Heatmap 데이터 수집 및 표시
- [ ] 상태 바 커스터마이징 동작
- [ ] 한국어 번역 추가
- [ ] CI 전체 통과
- [ ] 버전 범프 `1.2.0-ko.3`

---

## Phase 3: Architecture → `1.3.0-ko.1`

**Branch:** `dev/phase3-architecture`
**출처:** Papyrix
**의존:** Phase 2 완료 + upstream 동기화

### Task 7: 테스트 인프라

**Goal:** CMake 기반 호스트 빌드 테스트 환경 구축
**출처 레포:** `bigbag/papyrix-reader` — `test/` 디렉토리
**세부 구현:** Phase 3 시작 시 구체화

**Verify:**
- `cmake --build` + `ctest`로 호스트에서 테스트 실행
- Arduino/FreeRTOS/SdFat 모킹 동작
- 최소 5개 테스트 (ScriptDetector, Settings, UTF-8 등)

---

### Task 8: ScriptDetector 라이브러리

**Goal:** CJK/한글 감지 코드를 재사용 가능한 라이브러리로 분리
**출처 레포:** `bigbag/papyrix-reader` — `lib/ScriptDetector/`
**세부 구현:** Phase 3 시작 시 구체화

**Verify:**
- `ScriptDetector::isCjkCodepoint()` 한글 범위 정확
- 기존 inline CJK 체크 코드 ScriptDetector로 대체
- 테스트 통과

---

### Task 9: StreamingEpdFont

**Goal:** SD 카드 폰트 스트리밍으로 RAM 절약
**출처 레포:** `bigbag/papyrix-reader` — `lib/EpdFont/src/StreamingEpdFont.*`
**세부 구현:** Phase 3 시작 시 구체화

**Verify:**
- 기존 SD 폰트 로딩 대비 RAM 사용량 감소 확인
- 한국어 폰트 렌더링 품질 유지
- 페이지 넘김 성능 저하 없음

---

### Phase 3 완료 조건
- [ ] 호스트 테스트 환경 동작
- [ ] ScriptDetector 라이브러리 분리 완료
- [ ] StreamingEpdFont 동작 + RAM 절약 확인
- [ ] upstream 최신 동기화
- [ ] 버전 범프 `1.3.0-ko.1`

---

## Phase 4: Nice-to-have (선택)

독립적으로 착수 가능. 우선순위에 따라 선택.

| Task | 기능 | 출처 | 의존 |
|------|------|------|------|
| 10 | Achievements (62개 업적) | vCodex | Task 5 |
| 11 | Home/Apps 커스터마이징 | vCodex | 없음 |
| 12 | Sleep Tools | vCodex | 없음 |
| 13 | EPUB 최적화 강화 (CSS/폰트 정리) | bigbag converter | 없음 |

---

## 공통 규칙

### 브랜치 전략
```
dev/<phase>  →  pre-release/korean  →  release/korean
```

### 각 Phase 시작 시
1. 출처 레포 코드 분석 (Agent 파견)
2. 세부 구현 계획 구체화 (파일 경로, 코드, 테스트)
3. dev 브랜치 생성
4. 구현 → CI 통과 → 코드 리뷰 → 머지

### 커밋 규칙
- Conventional Commits (`feat:`, `fix:`, `chore:`)
- `.hxsk/` 변경사항 항상 포함
- CI 통과 후 머지

### 한국어 지원
- 새 설정 추가 시 `korean.yaml`에 번역 키 추가
- `english.yaml`에도 영어 키 추가 (upstream 호환)

---

## 참고 자료

- 로드맵: `.hxsk/docs/integration-roadmap.md`
- 생태계: `.hxsk/docs/xteink-ecosystem.md`
- 아키텍처: `.hxsk/ARCHITECTURE.md`
- 스택: `.hxsk/STACK.md`
- 출처 레포 비교: 메모리 `reference_fork_features.md`, `reference_papyrix_comparison.md`
