#include "MenuInterface.h"
#include "Graphics.h" 
#include "Attacks.h"

String selectPool[50];
uint8_t selectPoolCount = 0;

MenuInterface::MenuInterface() {

}

// Basic Lateral Scrolling Menu w/ Arrows
MenuInterface::MenuInterface(String menuText, int8_t numArrows) {
  this->menuText = menuText;
  this->numArrows = numArrows;

  // init();
}

void MenuInterface::addLoader(boolean ready, uint8_t timeEstimate) {
  while (!ready){
    ::display.clear();
    updateFooter();
    display.drawString(0,0,"Loading...");

    ::display.display();
  }
}

// ---- screen-adaptive drawing helpers ------------------------------
// All bitmaps are designed for a 128x64 panel. scrPx() scales every
// coordinate so the same layout maps onto taller (1.3" 128x128) panels,
// and navbar/header positions are derived from scrH() instead of the
// hard-coded y=53/54/56 values.

void MenuInterface::addTitle() {
  menuText.toUpperCase();

  // navbar outline sits at the bottom of any screen size
  ::display.drawXbm(scrPx(Navbar_Outline_x_hot),
                    (int)scrH() - scrPx(64 - Navbar_Outline_y_hot),
                    scrPx(Navbar_Outline_width), scrPx(Navbar_Outline_height),
                    Navbar_Outline_bits);

  int arrowY = (int)scrH() - scrPx(64 - Arrow_Left_y_hot);
  ::display.drawXbm(scrPx(Arrow_Left_x_hot), arrowY,
                    scrPx(Arrow_Left_width), scrPx(Arrow_Left_height), Arrow_Left_bits);

  if (numArrows != 1) {
    ::display.drawXbm(scrW() - scrPx(64 - Arrow_Right_x_hot) - scrPx(2), arrowY,
                      scrPx(Arrow_Right_width), scrPx(Arrow_Right_height), Arrow_Right_bits);
  }

  int textW = ::display.getStringWidth(menuText);
  int usable = scrPx(104);
  if (textW < usable) {
    ::display.drawString(((usable - textW) / 2) + scrPx(12),
                         (int)scrH() - scrPx(10), menuText);
  }
}

void MenuInterface::addHeader(String headerText, String secondaryText) {
  this->headerText = headerText;
  this->secondaryText = secondaryText;
  headerEnabled = true;

  ::display.drawXbm(Window_Header_x_hot, Window_Header_y_hot,
                    scrPx(Window_Header_width), scrPx(Window_Header_height),
                    Window_Header_bits);
  ::display.drawString(scrPx(3), Window_Header_y_hot, headerText);
  ::display.drawString(scrW() - scrPx(34), Window_Header_y_hot, secondaryText);
}

bool notInSelectPool(String selectText) {
  for (int i=0; i<selectPoolCount; i++) {
    if (selectText.equals(selectPool[i])) return false;
  }
  return true;  
}

void MenuInterface::addScroller(String* scrollerTextValues, uint8_t numScrollerValues) {
  this->scrollerTextValues = scrollerTextValues;
  this->numScrollerValues = numScrollerValues;

  scrollerMenu = true; scrollerSelectMenu = false;
  scrollerPos = 0;
}

void MenuInterface::addScrollerSelect(String* scrollerTextValues, uint8_t numScrollerValues) {
  this->scrollerTextValues = scrollerTextValues;
  this->numScrollerValues = numScrollerValues;

  if (this->numScrollerValues ==0) {
    Serial.println("0 found");
    scrollerTextValues[this->numScrollerValues] = "Rescan: 0 Found";
    this->numScrollerValues++;
  }

  scrollerTextValues[this->numScrollerValues] = "...";
  scrollerTextValues[this->numScrollerValues+1] = "* START ATTACK *";
  this->numScrollerValues+= 2;

  scrollerMenu = true; scrollerSelectMenu = true;
  scrollerPos = 0;
}

void MenuInterface::addScroller(String* scrollerTextValues, void (**scrollerMethodList)(), uint8_t numScrollerValues) {
  this->scrollerTextValues = scrollerTextValues;
  this->numScrollerValues = numScrollerValues;
  this->scrollerMethodList = scrollerMethodList;

  scrollerMenu = true; scrollerSelectMenu = false;
  scrollerPos = 0;
}

void MenuInterface::updateScroller() {

  uint8_t rows = scrRows();                 // rows that fit current screen
  int rowH    = scrPx(10);
  int contentTop = scrPx(15);               // below the window header strip

  for (uint8_t i = (scrollerPos / rows) * rows; i < ((scrollerPos / rows) * rows) + rows; i++) {
    if (i >= numScrollerValues) break;
    display.drawString(scrPx(4), contentTop + (i % rows) * rowH, scrollerTextValues[i]);
  }

  // draw selector line
  display.drawLine(0, contentTop + (scrollerPos % rows * rowH) + scrPx(2),
                   0, contentTop + (scrollerPos % rows * rowH) + rowH);
}


// add scrolling footer here
void MenuInterface::addFooter(String* footerTextValues, unsigned char* scrollerIconBits[]) {
  this->footerTextValues = footerTextValues;
  this->scrollerIconBits = scrollerIconBits;
  footerEnabled = true; dynamicFooter = true;
}

void MenuInterface::addFooter(String footerText) {
  this->footerText = footerText;
  footerEnabled = true;  dynamicFooter = false;
}

void MenuInterface::addNav(void (*function1)()) {
  this->function1 = function1;
  navCount = 1;
}

void MenuInterface::addNav(void (*function1)(), void (*function2)()) {
  this->function1 = function1;
  this->function2 = function2;
  navCount = 2;
}

void MenuInterface::addDashboard(unsigned char* dbGraphics, String* dbText, uint8_t dbCount) {
  this->dbGraphics = dbGraphics;
  this->dbText = dbText;
  this->dbCount = dbCount;
  ::display.drawXbm(0, scrPx(15), scrPx(128), scrPx(38), dbGraphics); // graphic scales with panel
  // for (int i=0; i<dbCount; i++) {
  //   //
  // }
}

void MenuInterface::updateFooter() {
  if (footerEnabled) {
    int navbarY = (int)scrH() - scrPx(11); // top of bottom navbar strip
    int textY   = (int)scrH() - scrPx(10); // navbar text row (was y=54 on 64px)
    if (dynamicFooter) {
      footerText = footerTextValues[scrollerPos];
      footerText.toUpperCase();

      // divider for arrow
      ::display.drawLine(0, textY, scrW()-1, textY);
      ::display.drawLine(0, textY - 1, scrW()-1, textY - 1);
      ::display.drawLine(scrW() - scrPx(10), textY, scrW() - scrPx(10), (int)scrH()-1);
      ::display.drawLine(scrW() - scrPx(11), textY, scrW() - scrPx(11), (int)scrH()-1);


      ::display.drawXbm(0,0,scrPx(128),scrPx(64),scrollerIconBits[scrollerPos]);
      ::display.drawXbm(scrW() - scrPx(6), textY + scrPx(2),
                        Arrow_Right_width, Arrow_Right_height, Arrow_Right_bits);

      ::display.drawString(0, textY, footerText);

    }
    else {
      footerText.toUpperCase();
      ::display.drawXbm(scrPx(Navbar_Outline_x_hot), navbarY,
                        scrPx(Navbar_Outline_width), scrPx(Navbar_Outline_height), Navbar_Outline_bits);

      if (navCount==2){
        ::display.drawXbm(scrW() - scrPx(6), textY + scrPx(2),
                          Arrow_Right_width, Arrow_Right_height, Arrow_Right_bits);
      }
      ::display.drawXbm(scrPx(Arrow_Left_x_hot), textY + scrPx(2),
                        Arrow_Left_width, Arrow_Left_height, Arrow_Left_bits);

      int usable = scrPx(104);
      if (::display.getStringWidth(footerText) < usable) {
        ::display.drawString(((usable - ::display.getStringWidth(footerText)) / 2) + scrPx(12), textY, footerText);
      }
      else {
        ::display.drawString(scrPx(56), textY, "...");
      }
    }
  }
}

// temporary method
void MenuInterface::addSimpleMonitor(uint8_t monitorCount) {
  this->monitorCount = monitorCount;
  monitorEn = true;
  ::display.drawString(0,scrPx(17),"Deauthing "+(String) monitorCount);
  ::display.drawString(0,scrPx(27),"clients from");
  String tmpBSSID = apList[scrollIndex][0];
  tmpBSSID.replace(":","");
  ::display.drawString(0,scrPx(37),tmpBSSID);
  ::display.drawXbm(scrPx(4),0,scrPx(128),scrPx(64),attack_bits);
  // wifiScanner.deauthClients();

}

void MenuInterface::updateDisplay(bool override) { // override variable doesn't even matter

  updateFooter();
  ::display.display();
  while (true) {
    int8_t press = nuggButtons.getPress();

    if (nuggButtons.ltPressed()) {
      (*function1)();
    }
    // else if (nuggButtons.rtPressed()) {
    //   (*function2)();
    // }

    delay(0);
  }
}

void MenuInterface::updateDisplayManual() {
    // while (true) {
    ::display.clear();

    int8_t press = nuggButtons.getPress();

    if (scrollerMenu) {
      
      // basic up & down scrolling actions
      if (nuggButtons.upPressed() and scrollerPos > 0) {
        scrollerPos--;
        Serial.println("Scroller");
      }
      else if (nuggButtons.dnPressed() and scrollerPos < numScrollerValues - 1) {
        scrollerPos++;
        Serial.println("sdfsdfs");
      }

      // check nav list or check function list 
      
      if (dynamicFooter) { // dynamic Menu
         if (nuggButtons.rtPressed()) {
        (*scrollerMethodList[scrollerPos])();
        }
      }
      else {
         
      }

      if (scrollerSelectMenu){
        if (nuggButtons.ltPressed()) {
          (*function1)();
        }
        else if (nuggButtons.rtPressed()) {
          Serial.println(scrollerTextValues[scrollerPos].indexOf("*"));
          
          if (scrollerPos<numScrollerValues-2 and scrollerTextValues[scrollerPos].indexOf("Rescan")>=0) {
            rescanClients = true;
            Attacks::selectClients();
            // wifiScanner.scanClients(25);
          }
          else if (scrollerPos<numScrollerValues-2 and !(scrollerTextValues[scrollerPos].indexOf("*")>=0)) {
            scrollerTextValues[scrollerPos] = "* "+(scrollerTextValues[scrollerPos]);
            selectPoolCount++;
          }
          else if (scrollerPos<numScrollerValues-2) {
            scrollerTextValues[scrollerPos].replace("* ","");
            selectPoolCount--;
          }
          else if (scrollerPos == numScrollerValues-1 and selectPoolCount>0){
            (*function2)();
          }

        }

      }

     
      
      updateScroller();
    }

        //dynamic footer already points to a function... right? 
        if(!dynamicFooter and !scrollerSelectMenu) {
          if (nuggButtons.ltPressed()) {
            Serial.println("Left press, executing function 1");
            (*function1)();
          }
          else if (nuggButtons.rtPressed() and navCount==2) {
            scrollIndex = scrollerPos; // temporary exposed variable
            Serial.println("Right press, executing function 2");
            (*function2)();
          }
        }

    if (headerEnabled) {
      addHeader(headerText, secondaryText);
    }

    // update to support list refresh
    updateFooter();
    //    addDashboard(dbGraphics, dbText, dbCount);


 if (monitorEn) {
        // Serial.println("monitor enabled!");
        addSimpleMonitor(monitorCount);
      }
    ::display.display();

    delay(0);
  // }
}

void MenuInterface::updateDisplay() { // updates UI display and loops until button input provided
  while (true) {
    ::display.clear();

    int8_t press = nuggButtons.getPress();

    if (scrollerMenu) {
      
      // basic up & down scrolling actions
      if (nuggButtons.upPressed() and scrollerPos > 0) {
        scrollerPos--;
        Serial.println("Scroller");
      }
      else if (nuggButtons.dnPressed() and scrollerPos < numScrollerValues - 1) {
        scrollerPos++;
        Serial.println("sdfsdfs");
      }

      // check nav list or check function list 
      
      if (dynamicFooter) { // dynamic Menu
         if (nuggButtons.rtPressed()) {
        (*scrollerMethodList[scrollerPos])();
        }
      }
      else {
         
      }

      if (scrollerSelectMenu){
        if (nuggButtons.ltPressed()) {
          (*function1)();
        }
        else if (nuggButtons.rtPressed()) {
          Serial.println(scrollerTextValues[scrollerPos].indexOf("*"));
          
          if (scrollerPos<numScrollerValues-2 and scrollerTextValues[scrollerPos].indexOf("Rescan")>=0) {
            rescanClients = true;
            Attacks::selectClients();
            // wifiScanner.scanClients(25);
          }
          else if (scrollerPos<numScrollerValues-2 and !(scrollerTextValues[scrollerPos].indexOf("*")>=0)) {
            scrollerTextValues[scrollerPos] = "* "+(scrollerTextValues[scrollerPos]);
            selectPoolCount++;
          }
          else if (scrollerPos<numScrollerValues-2) {
            scrollerTextValues[scrollerPos].replace("* ","");
            selectPoolCount--;
          }
          else if (scrollerPos == numScrollerValues-1 and selectPoolCount>0){
            (*function2)();
          }

        }

      }

     
      
      updateScroller();
    }

        //dynamic footer already points to a function... right? 
        if(!dynamicFooter and !scrollerSelectMenu) {
          if (nuggButtons.ltPressed()) {
            Serial.println("Left press, executing function 1");
            (*function1)();
          }
          else if (nuggButtons.rtPressed() and navCount==2) {
            scrollIndex = scrollerPos; // temporary exposed variable
            Serial.println("Right press, executing function 2");
            (*function2)();
          }
        }

    if (headerEnabled) {
      addHeader(headerText, secondaryText);
    }

    // update to support list refresh
    updateFooter();
    //    addDashboard(dbGraphics, dbText, dbCount);


 if (monitorEn) {
        // Serial.println("monitor enabled!");
        addSimpleMonitor(monitorCount);
      }
    ::display.display();

    delay(0);
  }
}
