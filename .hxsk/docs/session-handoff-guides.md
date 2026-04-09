# Session Handoff Guides

> 별도 세션에서 착수할 작업별 가이드라인. 각 세션 시작 시 이 문서의 해당 섹션을 전달.

---

## Session A: Task 7 — 테스트 인프라 구축

### 목표
CMake 기반 호스트 빌드 테스트 환경 + Arduino/FreeRTOS/SdFat 모킹

### 컨텍스트
- 현재 테스트: `test/hyphenation_eval/` 1개뿐 (디바이스 전용)
- Papyrix (`bigbag/papyrix-reader`)에 79개 테스트 + 풀 모킹 인프라
- 우리 프로젝트: PlatformIO + ESP32-C3

### 참고 자료
- Papyrix 테스트: `git show papyrix/main:test/CMakeLists.txt`
- Papyrix 모킹: `git show papyrix/main:test/mocks/`
- 아키텍처: `.hxsk/ARCHITECTURE.md`
- 하드웨어 제약: `.hxsk/docs/hardware-constraints.md`

### 작업 순서
1. `test/CMakeLists.txt` 생성 (호스트 빌드, gcc/clang)
2. `test/mocks/` 생성:
   - `Arduino.h` (millis, delay, Serial, String)
   - `FreeRTOS.h` (xSemaphore, vTaskDelay 스텁)
   - `HalStorage.h` (파일시스템 모킹 — 인메모리)
   - `HalDisplay.h` (디스플레이 스텁)
3. 기본 테스트 5개:
   - `test_ScriptDetector.cpp` — CJK/Hangul 감지
   - `test_BookmarkStore.cpp` — 저장/로드/토글
   - `test_BookSettings.cpp` — 직렬화/역직렬화
   - `test_ReadingStats.cpp` — 세션 추적
   - `test_AchievementStore.cpp` — 업적 해제
4. CI 통합: `.github/workflows/ci.yml`에 `cmake --build && ctest` 추가

### 주의사항
- Papyrix 모킹은 `SdMan` 사용, 우리는 `Storage` (HalStorage) — 어댑터 필요
- ESP32 전용 헤더 (`esp_wifi.h`, `esp_task_wdt.h`) 모킹 필요
- `#ifdef UNIT_TEST` 가드로 테스트/펌웨어 코드 분리
- 테스트는 호스트(macOS/Linux)에서 실행, 디바이스 불필요

### 브랜치
```bash
git checkout -b dev/test-infra release/korean
```

### 완료 조건
- `cmake --build build && cd build && ctest` 로 5개 테스트 통과
- CI에서 자동 실행

---

## Session B: Task 11 — Home/Apps 커스터마이징

### 목표
vCodex 스타일 숏컷 배치/순서/표시 설정

### 컨텍스트
- 현재 HomeActivity: 고정 메뉴 (Browse Files, File Transfer, Settings 등)
- vCodex: 설정에서 Home/Apps 숏컷 위치/순서 변경 가능
- Lyra Custom 테마 기본 적용

### 참고 자료
- vCodex: `git show vcodex/master:src/activities/home/HomeActivity.cpp`
- vCodex 설정: `git show vcodex/master:src/CrossPointSettings.h` — `shortcutOrder`, `shortcutVisibility`
- 현재 HomeActivity: `src/activities/home/HomeActivity.cpp`

### 작업 순서
1. vCodex의 Home/Apps 코드 분석
2. `CrossPointSettings`에 숏컷 관련 설정 추가
3. `HomeActivity` 수정: 설정 기반 메뉴 구성
4. `SettingsList`에 숏컷 설정 UI 추가
5. i18n 키 추가

### 주의사항
- HomeActivity는 upstream에서도 자주 변경됨 — 머지 충돌 가능
- vCodex의 `ActivityManager` 사용 패턴과 우리 코드 차이 주의
- Settings에 배열형 데이터 저장 시 JsonSettingsIO 확장 필요할 수 있음

### 브랜치
```bash
git checkout -b dev/home-customization release/korean
```

---

## Session C: CN-type TXT 챕터 감지 → 한국어 확장

### 목표
TXT 파일에서 챕터 자동 감지 (한국어 패턴 추가)

### 컨텍스트
- CN-type 포크에서 중국어 챕터 감지 코드 확보: `.hxsk/research/cn-type/`
- 중국어: `第N章` 패턴 (UTF-8: 0xE7AC AC ... 0xE7AB A0)
- 한국어 확장: `제N장`, `제N화`, `제N편`, `Chapter N`, `CHAPTER N`
- ScriptDetector 라이브러리 사용 가능 (Hangul 감지)

### 참고 자료
- CN-type 코드: `.hxsk/research/cn-type/cn-type-Txt.cpp` (832줄)
- CN-type TxtReader: `.hxsk/research/cn-type/cn-type-TxtReaderActivity.cpp` (966줄)
- 현재 Txt 라이브러리: `lib/Txt/Txt.cpp`, `lib/Txt/Txt.h`
- 현재 TxtReader: `src/activities/reader/TxtReaderActivity.cpp`
- ScriptDetector: `lib/ScriptDetector/`

### 작업 순서
1. CN-type `isHasChapterPattern()` 분석
2. 다국어 챕터 패턴 정의:
   ```
   Korean: "제" + 숫자 + "장|화|편|회|부"
   Chinese: "第" + 숫자 + "章|节|回"
   English: "Chapter " + 숫자, "CHAPTER " + 숫자
   ```
3. `lib/Txt/Txt.cpp`에 `detectChapters()` 추가
4. TxtReaderActivity에서 챕터 목록 UI 연동
5. 테스트 (다양한 패턴)

### 주의사항
- ESP32 RAM 제약: 큰 TXT 파일 전체를 메모리에 올리지 않음, 스트리밍 스캔 필요
- CN-type은 128바이트 라인 버퍼 사용 — 적절
- 패턴 매칭은 간단 UTF-8 바이트 시퀀스 비교 (정규식 아님)

### 브랜치
```bash
git checkout -b dev/txt-chapter-detection release/korean
```

---

## Session D: upstream 추적 + 다음 동기화

### 목표
upstream master의 1.2.0 이후 변경사항 모니터링 및 다음 동기화 준비

### 컨텍스트
- 현재 베이스: upstream 1.2.0 (태그)
- upstream master에 7개 추가 커밋 (X3 지원, C++20 리팩토링, ISO 639-2 하이픈 등)
- 다음 upstream 릴리즈 (1.3.0?) 시 동기화 필요

### 참고 자료
- 이전 머지 패턴: 메모리 `project_upstream_sync.md`
- 머지 에러 패턴: HAL 래핑, Activity 변경, FontCacheManager 타입
- 하드웨어 제약: `.hxsk/docs/hardware-constraints.md`

### 작업 순서
1. `git fetch upstream && git log --oneline 1.2.0..upstream/master` 로 변경 확인
2. 충돌 예상 분석 (`git merge --no-commit upstream/master`)
3. 한국어 커스터마이징 보존 우선 머지
4. CI 빌드 검증
5. 버전 범프

### 주의사항
- SD 카드 폰트 PR (#1327, #1392) 머지 시 우리 SdFont/StreamingEpdFont와 충돌 가능
- ActivityManager 변경 시 우리 Activity 코드 영향
- upstream이 gen_i18n.py 변경하면 korean.yaml 키 순서 재정렬 필요할 수 있음

### 브랜치
```bash
git checkout -b dev/upstream-sync-next release/korean
```

---

## Session E: GitHub Release + firmware.bin 배포

### 목표
GitHub Releases에 firmware.bin 자동 배포

### 컨텍스트
- CI에서 `firmware.bin` 아티팩트 생성됨
- `.github/workflows/release.yml` 존재하지만 ko 포크용 조정 필요
- 현재 태그: `1.2.0-ko.1`, `1.2.0-ko.2`, `1.2.0-ko.3`, `1.3.0-ko.1`

### 작업 순서
1. `release.yml` 검토 — 태그 푸시 시 자동 릴리즈 생성
2. release workflow 트리거 테스트 (`workflow_dispatch` 또는 태그)
3. Release Notes 자동 생성 (changelog 기반)
4. `firmware.bin` 아티팩트를 Release Assets에 첨부

### 주의사항
- OTA URL이 `SukbeomH/CrosspointFork`로 변경됨 — releases 형식 일치 필요
- release.yml의 `env: gh_release` 환경 사용 (LOG_LEVEL=0)
- slim 빌드도 필요한지 확인

### 브랜치
release/korean에서 직접 작업 (워크플로우 파일만 수정)

---

## 공통 규칙 (모든 세션)

### 필수 확인
- `.hxsk/docs/hardware-constraints.md` 참조 (RAM < 10KB 신규, Flash 99.4%)
- 커밋 시 `.hxsk/` 변경사항 포함
- `./bin/clang-format-fix` 실행 후 커밋
- `.hxsk/research/` 경로는 clang-format 제외됨
- CI에서 빌드 검증 (로컬 빌드 불가 — SK 프록시 SSL 문제)

### 브랜치 전략
```
dev/<feature> → PR → CI pass → merge to release/korean
```

### i18n
- 새 키 추가 시 `english.yaml` + `korean.yaml` 모두 업데이트
- `gen_i18n.py`가 빌드 시 자동 생성 — YAML이 source of truth

### 레포 정보
- Remote: `origin` (SukbeomH/CrosspointFork)
- Remote: `upstream` (crosspoint-reader/crosspoint-reader)
- Remote: `ko-upstream` (crosspoint-reader-ko/crosspoint-reader-ko)
- Remote: `vcodex` (franssjz/cpr-vcodex)
- Remote: `inx` (obijuankenobiii/inx)
- Remote: `papyrix` (bigbag/papyrix-reader)
