#ifndef SCREEN_H
#define SCREEN_H

// ============================================================
// Auto screen-size detection for OLED modules from 0.96" to 1.3"
// (Hackheld Vega 2 / Spacehuhn compatible)
// ============================================================
// Supported panels:
//   0.96"  128x64   SSD1306
//   1.0"   128x64   SSD1306
//   1.27"  128x64   SH1106   <- stock Vega 2 panel
//   1.3"   128x64   SH1106
//   1.3"   128x128  SSD1306
//
// Detection strategy (no user configuration needed):
//   1. I2C scan on 0x3C / 0x3D -> controller found.
//   2. Geometry probe ("last column" test): write a marker byte into
//      column 127 of page 7, then a second marker. On a 128x64 panel
//      that cell is the absolute last byte of GDDRAM, so the pointer
//      wraps back to page 0; on a 128x128 panel it lands mid-buffer.
//      The two cases are made visible and copied back out of the
//      driver's buffer object through getBuffer() to decide height.
//   3. Controller family (SSD1306 vs SH1106) via the charge-pump setup
//      command 0x8D: SSD1306 requires it before display-on works; we
//      combine the observed geometry result with the compile-time board
//      default (Vega 2 = SH1106).
//   4. Physical size string derived from geometry + controller family.
//
// The detected profile feeds a layout scale factor used by the UI so
// menus, icons, navbar and scroller adapt automatically.

#include <Arduino.h>
#include <Wire.h>

enum ScreenModel : uint8_t {
  SCREEN_NONE = 0,
  SCREEN_096,      // 0.96"   128x64   SSD1306
  SCREEN_127,      // 1.27"   128x64   SH1106 (Vega 2 stock)
  SCREEN_130_64,   // 1.3"    128x64   SH1106
  SCREEN_130_128   // 1.3"    128x128  SSD1306
};

struct ScreenProfile {
  ScreenModel model;
  const char* name;    // human readable description
  uint16_t    width;   // px
  uint16_t    height;  // px
  float       scale;   // UI scale relative to the 128x64 design grid
  bool        sh1106;  // true -> SH1106 controller (needs col offset 2)
  uint8_t     addr;    // detected I2C address
};

extern ScreenProfile screenProfile;

// Probe the bus and fill screenProfile. Returns false if no display was
// found (profile then keeps safe 128x64 SH1106 defaults).
bool detectScreen();

// ---- layout helpers used across the UI ---------------------------
inline uint16_t scrW()          { return screenProfile.width; }
inline uint16_t scrH()          { return screenProfile.height; }
inline float    scrScale()      { return screenProfile.scale; }
inline int      scrPx(int v)    { return (int)(v * screenProfile.scale); }
inline uint16_t scrNavbarY()    { return (uint16_t)(scrH() - scrPx(11)); } // navbar strip top
inline uint16_t scrContentH()   { return (uint16_t)(scrH() - scrPx(15)); } // area above navbar
inline uint8_t  scrRows()       { // scroller rows that fit in content area
                    uint8_t r = (uint8_t)(scrContentH() / (uint16_t)scrPx(10));
                    return r ? r : 1;
                  }

#endif // SCREEN_H
