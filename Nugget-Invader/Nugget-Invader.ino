// ============================================================
// Hackheld Vega 2 (Spacehuhn) button layout
// ============================================================
// The Vega 2 has 4 user buttons on the face of the device:
//
//        [ UP ]
//   [ A ]          [ B ]
//        [ DOWN ]
//
// Sketch mapping (up/dn/lt/rt used by MenuInterface & Buttons class):
//   up -> UP button    : scroll menu upward
//   dn -> DOWN button  : scroll menu downward
//   lt -> A button     : "back" / cancel action
//   rt -> B button     : "select" / confirm action
//
// ESP8266 GPIO notes:
//   - D0 (GPIO16) is avoided because it has no internal pull-up/down and
//     cannot generate normal pin interrupts.
//   - D3/D4 (GPIO9/GPIO10) are reserved for the SPI flash on ESP-12 modules.
//   - D8 (GPIO15) must be LOW at boot, so it stays free for the NeoPixel.
#define up D5 // UP button   (GPIO14 / D5)
#define dn D6 // DOWN button (GPIO12 / D6)
#define lt D1 // A button    (GPIO5  / D1)
#define rt D2 // B button    (GPIO4  / D2)

#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "Buttons.h"
#include "MenuInterface.h"
#include "Graphics.h"
#include "WiFiScanner.h"
#include <Adafruit_NeoPixel.h>

#include "Attacks.h"
#include "Monitor.h"

Adafruit_NeoPixel pixels {1, D8, NEO_GRB + NEO_KHZ800 }; // initialize 1 NeoPixel on D8

// ---- screen objects (0.96" .. 1.3" auto-select at boot) ----------
// SH1106Wire: 1.27"/1.3" 128x64 panels (Vega 2 stock OLED)
// SSD1306Wire: 0.96"/1.3" 128x64 and 128x128 panels
// Both drivers share the OLEDDisplay base class, so a single global
// reference is enough for all drawing calls in the project.
SH1106Wire  sh1106Display(0x3C, SDA, SCL);
SSD1306Wire ssd1306Display(0x3C, SDA, SCL);
OLEDDisplay& display = sh1106Display; // rebound below if SSD1306 detected

void initDetectedDisplay() {
  // probe the I2C bus (0x3C / 0x3D) and identify size + controller
  bool found = detectScreen();
  Serial.println(found ? ("Display detected: " + String(screenProfile.name))
                       : "No display found, using stock 1.27\" SH1106 defaults");

  // rebind the shared display reference to the matching driver
  if (screenProfile.sh1106) display = sh1106Display;
  else                       display = ssd1306Display;

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10); // 10px font fits every 0.96"-1.3" panel
}
Buttons nuggButtons(up,dn,lt,rt);

MenuInterface invaderMenu; // blank menu canvas
MenuInterface invaderAboutMenu;

MenuInterface attacksSelectAP;
MenuInterface attacksSelectClients;
MenuInterface attacksDeauthStatus;

MenuInterface monitorPackets;

Attacks attackTool;
Monitor monitorTool;

bool rescan;
bool rescanClients;
uint8_t scrollIndex;

// Nugget Invader tool options
String invaderMenuText[] = {"Attack","Monitor","About"};
String invaderMenuDesc[] {
  "Run WiFi Attacks",
  "Monitor Attacks",
  "Nugget Invader"
};

unsigned char* invaderMenuIcons[] = {
  invader_attack_bits,
  invader_monitor_bits,
  invader_about_bits
};

void invaderAttacks () {Attacks::init();}
void invaderMonitor () {monitorTool.main();}
void invaderAbout() {
  invaderAboutMenu.addFooter("About");
  invaderAboutMenu.addNav(invaderToolMenu);

  // re implement as text method
  display.drawString(0,0,"Nugget Invader v1.0");
  display.drawString(0,10,"by Alex Lynd");
  display.drawString(0,30,"wifinugget.com/");
  display.drawString(0,40,"invader");

  display.display();

  invaderAboutMenu.updateDisplay(false); // override clear option
}

// function array
void (*invaderTools[])(void) = {
  invaderAttacks,
  invaderMonitor,
  invaderAbout,
};

void setup() {
  Serial.begin(115200); delay(3000);
  pixels.begin(); pixels.clear(); 
  pixels.setBrightness(100);

  Serial.println("\nNugget Invader v1.0");
  Serial.println("by Alex Lynd");
  Serial.println("--------------------------");

  // auto-detect screen size (0.96" - 1.3") and initialize the right driver
  initDetectedDisplay();

  display.drawXbm(0,0,scrPx(128),scrPx(64),invader_bits);
  display.display();
  delay(1500);
  display.clear(); display.display();
  
  // startup tool select menu

  // add header, dynamic scroller position update to scale with screen 
  // add loader menu
  invaderToolMenu();
}

void invaderToolMenu() {
  rescan = true;
  rescanClients = true;
  // Serial.println("# entered invader tool menu");
  invaderMenu.addScroller(invaderMenuText, invaderTools, 3);
  invaderMenu.addFooter(invaderMenuDesc, invaderMenuIcons);
  invaderMenu.updateDisplay();
}

void loop() {
  return;
}
