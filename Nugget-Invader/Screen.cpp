#include "Screen.h"

// Safe fallback: 1.27" SH1106 128x64 (Hackheld Vega 2 stock panel).
// Even if detection fails, the UI keeps working on the stock screen.
ScreenProfile screenProfile = {
  SCREEN_127, "1.27\" SH1106 128x64", 128, 64, 1.0f, true, 0x3C
};

static bool sendCmd(uint8_t addr, uint8_t cmd) {
  Wire.beginTransmission(addr);
  Wire.write(0x00);          // control byte: command mode
  Wire.write(cmd);
  return (Wire.endTransmission() == 0);
}

static bool sendData(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(addr);
  Wire.write(0x40);          // control byte: data mode
  Wire.write(data);
  return (Wire.endTransmission() == 0);
}

static bool probeAck(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

// --------------------------------------------------------------------
// Controller family test (SSD1306 vs SH1106), no read-back needed:
//
// Both chips ACK every I2C transfer, but they differ in GDDRAM size:
// SSD1306 = 1KB (128x64) or 2KB (128x128 variant), SH1106 = 1KB spread
// over 132 columns. We use the wrap behaviour of horizontal addressing:
//   - position the write pointer at the LAST column of page 7
//     (col 127 for SSD1306; for SH1106 we use col 131 which is its last)
//   - push two data bytes.
// On a 128x64 device the pointer wraps to page 0 and overwrites visible
// top-row RAM; on a 128x128 device it continues into page 8 (invisible
// from a 64-tall viewport but harmless).
// Since we cannot observe GDDRAM directly, we instead distinguish the
// families by their *required* init sequence side effects:
//   - SH1106 ignores the SSD1306 boost-sequence commands entirely and
//     stays responsive; both ACK, so the only reliable differentiator
//     available without read-back is the compile-time board default.
// Therefore: geometry/family come from the board config below, while the
// runtime scan guarantees a panel is present and picks its address.
// Users with a non-stock panel define one of:
//   SCREEN_IS_096      -> 0.96"  SSD1306 128x64
//   SCREEN_IS_130_SH   -> 1.3"   SH1106  128x64
//   SCREEN_IS_130_128  -> 1.3"   SSD1306 128x128
// --------------------------------------------------------------------
static ScreenModel configuredModel() {
#if defined(SCREEN_IS_096)
  return SCREEN_096;
#elif defined(SCREEN_IS_130_SH)
  return SCREEN_130_64;
#elif defined(SCREEN_IS_130_128)
  return SCREEN_130_128;
#else
  return SCREEN_127; // Vega 2 stock OLED: 1.27" SH1106
#endif
}

bool detectScreen() {
  // ---- find the panel on the bus ---------------------------------
  uint8_t addr = 0;
  for (uint8_t a : {(uint8_t)0x3C, (uint8_t)0x3D}) {
    if (probeAck(a)) { addr = a; break; }
  }
  if (addr == 0) return false; // nothing attached -> keep defaults

  screenProfile.addr = addr;

  // ---- apply the selected profile --------------------------------
  switch (configuredModel()) {
    case SCREEN_096:
      screenProfile.model  = SCREEN_096;
      screenProfile.name   = "0.96\" SSD1306 128x64";
      screenProfile.width  = 128;
      screenProfile.height = 64;
      screenProfile.scale  = 1.0f;
      screenProfile.sh1106 = false;
      break;
    case SCREEN_130_64:
      screenProfile.model  = SCREEN_130_64;
      screenProfile.name   = "1.3\" SH1106 128x64";
      screenProfile.width  = 128;
      screenProfile.height = 64;
      screenProfile.scale  = 1.0f;
      screenProfile.sh1106 = true;
      break;
    case SCREEN_130_128:
      screenProfile.model  = SCREEN_130_128;
      screenProfile.name   = "1.3\" SSD1306 128x128";
      screenProfile.width  = 128;
      screenProfile.height = 128;
      screenProfile.scale  = 2.0f; // taller canvas: layout stretches 2x
      screenProfile.sh1106 = false;
      break;
    case SCREEN_127:
    default:
      screenProfile.model  = SCREEN_127;
      screenProfile.name   = "1.27\" SH1106 128x64";
      screenProfile.width  = 128;
      screenProfile.height = 64;
      screenProfile.scale  = 1.0f;
      screenProfile.sh1106 = true;
      break;
  }
  return true;
}
