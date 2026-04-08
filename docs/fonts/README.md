# SD Card Fonts

SD 카드 `/.crosspoint/fonts/` 에 복사하여 사용.
Settings > 글꼴 설정에서 선택.

## 포함 폰트

| 폰트 | 파일 | 크기 | 한글 커버리지 | 특징 |
|------|------|------|--------------|------|
| 부크크명조 Light 14pt | `bookkMyungjo_14_regular.epdfont` | 2.2MB | 100% (11,172) | 세리프, 명조체 |
| Hahmlet Medium 14pt | `hahmlet_14_medium.epdfont` | 564KB | 24% (2,788) | 세리프, 가변폭 |

## 변환 방법

```bash
python3 lib/EpdFont/scripts/ttf_to_epdfont.py \
  <name> <size> <font.ttf> \
  --additional-intervals 0xAC00,0xD7AF \
  --2bit \
  -o <output.epdfont>
```

## 라이선스

- 부크크명조: [부크크](https://www.bookk.co.kr/) 제공
- Hahmlet: [Google Fonts](https://fonts.google.com/specimen/Hahmlet), SIL OFL 1.1
