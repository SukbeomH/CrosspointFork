---
title: "Fix text overlapping bug"
tags:
  - bugfix
  - renderer
type: bugfix
created: 2026-04-08T10:09:26Z
contextual_description: "Fix text overlapping issue for Korean SD Font by applying 12.4 fixed-point conversion"
keywords:
  - advanceX
  - SdFont
  - fp4
  - fixed-point
---

## Fix text overlapping bug

GfxRenderer was updated to use 12.4 fixed point for cursor accumulator. SdFont data loading was still passing integer pixels to advanceX. Converted SD pixel width to fp4 fixed point in loadGlyphFromSD and getTextDimensions.
