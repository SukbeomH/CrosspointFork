# Xteink X4 Ecosystem Reference

> Source: readme.club (2026-04-08 수집)
> 비공식 커뮤니티 허브. Xteink X4 e-paper 리더 관련 리소스, 팁, 펌웨어, 가이드 종합.

---

## 1. Alternative Firmwares

| Firmware | Version | Stars | 특징 |
|----------|---------|-------|------|
| **Xteink (Official)** | V5.0.3 | — | 공식 펌웨어, 앱 연동 |
| **CrossPoint** | 1.2.0 | 3,271 | 오픈소스, 최대 커뮤니티, 22개 언어 |
| **Papyrix** | v1.19.0 | 223 | FB2/MD 지원, 미니앱, 아랍어/태국어, 79개 테스트 |
| **SUMI** | 0.4.2_dev | 108 | — |
| **Microreader** | — | 97 | — |
| **CrossPet** | v1.8.0 | 88 | — |
| **TernOS** | v0.4.0 | 71 | — |
| **CPR-vCodex** | 1.2.0.7 | 43 | CrossPoint 기반 |
| **inx** | 1.0.5 | 36 | — |
| **CrossPoint BLE** | 1.2-personal | 25 | BLE 확장 |
| **CrossWordle** | v1.1.1 | 2 | Wordle 게임 포함 |

---

## 2. Tools & Converters

### 파일 변환
| 도구 | 기능 |
|------|------|
| **EPUB2XTC** | EPUB → XTC (TOC, 챕터 필터링) |
| **cr2xt** | 멀티포맷 → XTC (fb2, epub, rtf, doc, docx, odt, md, mobi, txt) |
| **EPub → XTC Converter** | 브라우저 기반, 커스텀 폰트, 읽기 진행 바 |
| **PDF to XTC** | PDF → XTC (품질 설정 가능) |
| **XTC to PDF** | XTC → PDF 역변환 |
| **CBZ to XTC** | 만화/코믹스 변환 (2-bit 그레이스케일) |

### 이미지/미디어
| 도구 | 기능 |
|------|------|
| **Baseline JPEG Converter** | PNG/GIF/WebP/BMP → baseline JPEG (480×800 자동 스케일링) |
| **X4 Wallpaper Converter** | X4/X3 배경화면 배치 처리 |
| **Image to BMP Converter** | 스크린세이버용 BMP 변환 |

### 폰트
| 도구 | 기능 |
|------|------|
| **ePaper Font Converter (Dotink)** | iPhone 앱, TTF → .bin (e-ink 프리뷰) |
| **Web Font Maker** | 브라우저 기반 TTF/OTF → .bin (굵기, 간격, 정렬 조절) |

### 텍스트/콘텐츠
| 도구 | 기능 |
|------|------|
| **Wikipedia to TXT** | Wikipedia 글 → .txt 다운로드 |
| **Endnotes to parenthetical** | EPUB 각주 → 괄호 형식 변환 |
| **Markdown to ePub** | Python 터미널, md → epub |
| **Flashcards Club** | EPUB 플래시카드 생성 |

---

## 3. Companion Apps

| 앱 | 플랫폼 | 기능 |
|----|--------|------|
| **Send to X4** | iOS/Android | 웹 글, 노트, EPUB → WiFi 전송 (클라우드 불필요) |
| **Send to X4 Extension** | Chrome/Firefox | 웹 글 → EPUB → WiFi hotspot 전송 |
| **Bookshelf** | iOS | 읽기 진도 WiFi 동기화, 홈 위젯 |
| **Xteink Official App** | iOS/Android | 공식 앱, V5.0.3+ 필요 |
| **XTLibre** | Docker | EPUB → XTC, 로컬 라이브러리, OPDS 노출 |

### Calibre 플러그인
- **baseline_jpg_converter** — EPUB 이미지 → baseline JPEG (CrossPoint 호환)
- **EPUB Workflow Plugin** — 원클릭 배치 처리 (수리, 최적화, 정리, 리사이즈)

---

## 4. Hardware Specs Quick Reference

| 항목 | 스펙 |
|------|------|
| MCU | ESP32-C3 (RISC-V, 160MHz, single core) |
| 디스플레이 | E-ink, 480×800, 4-level 그레이스케일 |
| 무게 | 74g |
| 크기 | 4.49 × 2.72 inches |
| 저장 | microSD 32GB 포함 (exFAT 권장) |
| 충전 | USB-C, 2-3시간 완충 |
| 배터리 | 수 주 사용 가능 |
| 연결 | WiFi (STA/AP), Bluetooth |
| 입력 | 전면 4버튼 + 측면 2버튼 (터치스크린 없음) |
| 자석 | 내장 (MagSafe 호환) |

---

## 5. Tips & Best Practices (핵심 발췌)

### 설정
- 구매 후 즉시 펌웨어 업데이트 (Settings → About → System Update)
- microSD: exFAT 포맷 권장, FAT32는 4GB 제한
- WiFi는 필요할 때만 켜서 배터리 절약

### 읽기 환경
- EPUB 권장, TXT도 지원
- Calibre로 EPUB 정리: "Repair HTML → Beautify → Delete unused CSS"
- 커스텀 폰트: TTF → .bin 변환 후 SD 카드에 배치
- EPUB 표지가 잠금 화면으로 표시

### 폰트/언어
- 내장 폰트 2개 + .bin 커스텀 폰트 지원
- 1,800+ 문자 추가 (V5.0.3)
- 일본어 표시 가능 (세로 쓰기 미지원)
- 독일어 ß 등 특수문자는 커스텀 폰트 필요

### 파일 관리
- 루트 또는 'Books' 폴더에 파일 배치
- 32GB에 약 700-1000권 저장 가능
- UTF-8 및 GBK 인코딩 지원

### 문제 해결
- "Reading History Error" → Calibre로 EPUB 정리 또는 TXT 변환
- 네모(□) 표시 → 해당 글리프 포함 폰트로 교체
- microSD 인식 불가 → 컴퓨터에서 exFAT 재포맷
- 소프트 리셋 (microSD 옆 버튼) → 90% 소프트웨어 문제 해결

### 성능 최적화
- EPUB에서 불필요 이미지 제거
- 100MB+ TXT 파일은 분할
- 펌웨어 업데이트 후 재인덱싱
- 읽기 기록 주기적 정리

---

## 6. New Owner Guide (Day-1 체크리스트)

- [ ] 완충 후 펌웨어 업데이트
- [ ] 공식 구매처 확인
- [ ] V5.0.3+ 업데이트
- [ ] 언어, 시간, 전원 설정
- [ ] USB로 테스트 EPUB 로드 → 레이아웃 확인
- [ ] WiFi/Bluetooth 설정
- [ ] (선택) Xteink App 연결

---

## 7. Community Resources

| 리소스 | URL/정보 |
|--------|----------|
| Reddit (공식) | r/xteinkereader |
| Reddit (커뮤니티) | r/xteink |
| Discord | 개발자/사용자 허브 |
| readme.club | 커뮤니티 리소스 허브 |
| 커뮤니티 허브 | xteink-community-hub.replit.app |
| 지원 | support@xteink.com |

---

## 8. CrossPoint Fork 관점에서의 시사점

### 통합 가능한 도구
- **EPUB Workflow Plugin** 패턴 → 내장 EPUB 최적화 기능
- **Send to X4** 패턴 → WebServer 엔드포인트 확장
- **Font Converter** → SD 폰트 변환 파이프라인 내장

### 경쟁 펌웨어 참고
- **Papyrix**: FB2/MD, 테스트 79개, 스크립트 감지 (별도 분석 완료)
- **SUMI/TernOS**: 기능 차별화 포인트 조사 필요
- **CPR-vCodex**: CrossPoint 기반 포크, 추가 기능 확인 필요

### 사용자 니즈 (팁에서 추출)
- 커스텀 폰트 쉬운 설치 (현재 .bin 변환 필요)
- 배치 EPUB 정리/최적화
- 더 많은 파일 포맷 지원 (PDF, FB2, MD)
- Bluetooth 페이지 터너 안정성
