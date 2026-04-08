# Xteink X4 하드웨어 제약 및 성능 가이드라인

> 모든 기능 개발 시 이 문서를 참조. 위반 시 크래시 또는 UX 저하.

## 핵심 수치

| 리소스 | 스펙 | 실질 가용 | 비고 |
|--------|------|-----------|------|
| **RAM** | ~400KB SRAM | **~120-180KB free heap** (런타임) | WiFi 활성 시 추가 ~60KB 소모 |
| **Flash** | 16MB | ~7.5MB 앱 파티션 | 폰트/코드/상수 공유 |
| **CPU** | RISC-V 160MHz **싱글 코어** | FreeRTOS 멀티태스킹 | 무거운 연산은 루프 블로킹 |
| **Display** | e-ink 480×800 4-level grayscale | 풀 리프레시 ~1초 | 부분 리프레시 가능하지만 고스팅 |
| **Storage** | SD Card (SPI) | 읽기 ~1MB/s | 대량 랜덤 읽기 느림 |
| **Battery** | ~수 주 | WiFi/BLE가 주 소모원 | deep sleep ~5μA |

## 메모리 예산 (힙 사용)

코드에서 확인된 최소 힙 요구량:

| 기능 | 최소 힙 | 비고 |
|------|---------|------|
| CSS 파싱 | 64KB | `MIN_HEAP_FOR_CSS_PARSING` |
| CSS 스타일 적용 | 48KB | `MIN_FREE_HEAP_FOR_CSS` |
| JPEG 디코딩 | ~디코더 + 16KB | `MIN_FREE_HEAP_FOR_JPEG` |
| PNG 디코딩 | ~디코더 + 16KB | `MIN_FREE_HEAP_FOR_PNG` |
| SD 폰트 로딩 후 잔여 | 16KB | `MIN_FREE_HEAP_AFTER_LOAD` |

**실질적 의미:** 새 기능이 상주 메모리를 10KB 이상 추가하면 이미지 렌더링/CSS 파싱이 실패할 수 있음.

## 설계 원칙 (커뮤니티 + 아키텍처 문서 종합)

### 1. SD-First 캐싱
- RAM에 데이터를 보유하지 않는다. 계산 결과는 SD에 캐시.
- 반복 접근 시 SD 캐시에서 로드 (재파싱 없음).
- 캐시 무효화 조건 명확히 정의 (설정 변경 시).

### 2. 할당 최소화
- `new`/`malloc` 최소 사용. 스택 변수 선호.
- 큰 할당은 반드시 `ESP.getFreeHeap()` 확인 후 진행.
- 할당 실패 시 graceful degradation (크래시 금지).

### 3. 루프 반응성
- 메인 루프 1회 < 100ms 권장.
- 무거운 작업은 FreeRTOS 태스크로 분리.
- watchdog 안전: 긴 루프에서 `yield()` 또는 `vTaskDelay()`.

### 4. e-ink 리프레시 최적화
- 풀 리프레시 최소화 (고스팅 관리와 균형).
- 부분 업데이트로 체감 속도 개선.
- 불필요한 화면 갱신 억제 (`updateRequired` 플래그 패턴).

### 5. 네트워크 절약
- WiFi는 필요 시에만 활성화.
- WiFi sleep 비활성화는 웹서버 실행 중에만.
- OTA 다운로드 중 watchdog 피딩 필수.

### 6. 배터리 수명
- deep sleep 적극 활용 (설정 가능한 자동 절전).
- WiFi/BLE off 시 수 주 사용 가능.
- 백그라운드 작업은 sleep/power 로직과 협력.

## 기능별 메모리 영향 추정

| 기능 | 예상 상주 RAM | SD 사용 | 위험도 |
|------|---------------|---------|--------|
| Dark Mode | ~4B (설정) | 없음 | **안전** — 렌더링 로직만 변경 |
| Bookmarks | ~100B (포인터) | `bookmarks.bin` per book | **안전** — SD 저장 |
| Text Darkness | ~4B (설정) | 없음 | **안전** |
| 도서별 설정 | ~200B (현재 도서) | `setting.bin` per book | **안전** — SD 저장 |
| Reading Stats | ~500B (현재 세션) | `statistics.bin` per book | **주의** — 세션 타이머 상주 |
| 상태 바 커스텀 | ~50B (설정) | 없음 | **안전** |
| 테스트 인프라 | 0 (빌드타임) | 없음 | **안전** |
| ScriptDetector | ~200B (코드) | 없음 | **안전** — 기존 코드 대체 |
| StreamingEpdFont | **-45KB** (절약) | 캐시 변경 | **개선** — RAM 확보 |
| Achievements | ~2KB (62개 상태) | `achievements.bin` | **주의** — 상태 상주 |
| EPUB 최적화 강화 | 0 (클라이언트 JS) | 없음 | **안전** — 브라우저 처리 |

## 성능 벤치마크 기준

| 작업 | 허용 기준 | 현재 참고 |
|------|-----------|-----------|
| 페이지 넘김 (캐시 히트) | < 500ms | ~200-300ms |
| 페이지 넘김 (캐시 미스) | < 3s | ~1-2s |
| 챕터 인덱싱 | < 10s | 가변 |
| 부트 → 읽기 복귀 | < 5s | ~3s |
| WiFi 웹서버 응답 | < 2s | ~1s |
| 이미지 렌더링 | < 5s/page | 최적화 후 ~1-2s |

## 커뮤니티 팁 (readme.club 성능 섹션)

- EPUB에서 불필요 이미지 제거 → 렌더링 속도 개선
- 100MB+ TXT 파일은 분할
- 펌웨어 업데이트 후 재인덱싱
- 폴더 정리 → 메뉴 탐색 랙 방지
- 읽기 기록 주기적 정리 → 메뉴 성능 유지
- Calibre로 EPUB 정리 (Repair HTML, Delete unused CSS) → 파싱 부담 감소
