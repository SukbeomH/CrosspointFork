# CrossPoint Reader Korean Fork

Xteink X4 e-paper 리더를 위한 한국어 최적화 오픈소스 펌웨어.
[CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) 기반 + 커뮤니티 포크 기능 통합.

![](./docs/images/cover.jpg)

## 이 포크의 특징

### 한국어 지원
- 한국어 UI 번역 (320+ 문자열)
- 내장 한국어 폰트: Pretendard (UI), KoPub Batang (리더)
- 글자 단위 줄바꿈 (Character Wrap) — CJK 텍스트 최적화
- 단락 들여쓰기 (Paragraph Indent)
- SD 카드 커스텀 폰트 지원

### 커뮤니티 포크 기능 통합
- **Dark Mode** — 글로벌 흑백 반전 (출처: CPR-vCodex)
- **EPUB Bookmarks** — 페이지 북마크 추가/제거 (출처: CPR-vCodex)
- **Text Darkness** — 3단계 텍스트 진하게 (출처: CPR-vCodex/crosspet)
- **도서별 설정** — 책마다 폰트/여백/상태 바 개별 저장 (출처: inx)

### CrossPoint 1.2.0 기반
- EPUB 최적화, 배터리 충전 표시, 챕터 프리인덱싱
- Kerning/Ligature 지원, CSS display:none
- 숨김 디렉토리 표시, sleep 루틴 개선
- 22개 언어 지원 (한국어 포함)

## 업스트림 관계

```
crosspoint-reader/crosspoint-reader (upstream, 3275 stars)
    +-- crosspoint-reader-ko/crosspoint-reader-ko (한국어 포크)
        +-- SukbeomH/CrosspointFork (이 레포)
            +-- CPR-vCodex (Dark Mode, Bookmarks, Stats)
            +-- inx (도서별 설정, 상태 바)
            +-- Papyrix (테스트, ScriptDetector) [계획]
```

## 설치

### Web (펌웨어 플래싱)

1. Xteink X4를 USB-C로 컴퓨터에 연결
2. [Releases](https://github.com/SukbeomH/CrosspointFork/releases)에서 `firmware.bin` 다운로드
3. https://xteink.dve.al/ 에서 "OTA fast flash controls"로 플래싱

공식 펌웨어로 복원: https://xteink.dve.al/ 에서 공식 펌웨어 플래싱 또는 "Swap boot partition" 사용.

### 수동 빌드

[Development](#development) 참조.

## 기능 목록

- [x] EPUB 파싱 및 렌더링 (EPUB 2, EPUB 3)
- [x] EPUB 내 이미지 지원 (JPG, PNG)
- [x] 읽기 위치 저장
- [x] 파일 탐색기 (중첩 폴더 지원)
- [x] 커스텀 잠금 화면 (표지, 사용자 지정)
- [x] WiFi 파일 업로드 + EPUB 최적화
- [x] WiFi OTA 업데이트
- [x] KOReader 동기화
- [x] OPDS 브라우저
- [x] WebDAV 파일 전송
- [x] Calibre 무선 전송
- [x] 화면 회전
- [x] 다크 모드
- [x] EPUB 북마크
- [x] 텍스트 진하게 (3단계)
- [x] 도서별 설정 (폰트, 여백, 상태 바)
- [x] 한국어 UI + 22개 언어

## Development

### 필수 도구

* **PlatformIO Core** (`pio`) 또는 **VS Code + PlatformIO IDE**
* Python 3.8+
* USB-C 케이블
* Xteink X4

### 코드 체크아웃

```bash
git clone --recursive https://github.com/SukbeomH/CrosspointFork.git
cd CrosspointFork

# 서브모듈 미포함 시:
git submodule update --init --recursive
```

### 빌드 & 플래싱

```sh
pio run --target upload
```

### 디버깅

```bash
python3 -m pip install pyserial colorama matplotlib
python3 scripts/debugging_monitor.py /dev/cu.usbmodem2101  # macOS
```

## 데이터 캐시

```
.crosspoint/
+-- epub_<hash>/
    +-- progress.bin      # 읽기 진도
    +-- settings.bin      # 도서별 설정
    +-- bookmarks.bin     # 북마크
    +-- cover.bmp         # 표지 이미지
    +-- book.bin          # 메타데이터
    +-- sections/         # 챕터 레이아웃 캐시
        +-- 0.bin
        +-- ...
```

## 로드맵

| 버전 | 내용 | 상태 |
|------|------|------|
| `1.2.0-ko.1` | upstream 1.2.0 동기화 | 완료 |
| `1.2.0-ko.2` | Phase 1: Dark Mode, Bookmarks, Text Darkness | 완료 |
| `1.2.0-ko.3` | Phase 2: 도서별 설정, Reading Stats, 상태 바 | 진행 중 |
| `1.3.0-ko.1` | Phase 3: 테스트 인프라, ScriptDetector, StreamingFont | 계획 |

상세: [`docs/plans/2026-04-08-community-fork-integration.md`](docs/plans/2026-04-08-community-fork-integration.md)

## 크레딧

- [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) — 원본 프로젝트
- [crosspoint-reader-ko](https://github.com/crosspoint-reader-ko/crosspoint-reader-ko) — 한국어 포크 원작자
- [CPR-vCodex](https://github.com/franssjz/cpr-vcodex) — Dark Mode, Bookmarks, Text Darkness
- [inx](https://github.com/obijuankenobiii/inx) — 도서별 설정, 상태 바
- [Papyrix](https://github.com/bigbag/papyrix-reader) — ScriptDetector, StreamingEpdFont, 테스트 인프라
- [diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader) — 영감

---

이 프로젝트는 **Xteink 또는 X4 하드웨어 제조사와 무관**한 커뮤니티 프로젝트입니다.
