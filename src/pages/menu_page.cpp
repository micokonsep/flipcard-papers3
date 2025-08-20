#include "menu_page.h"
#include <M5Unified.h>
#include <SD.h>

// Button regions - button positions in menu.png
int categoryBtnX = 50;     // Category button on left: x1=50, x2=190
int categoryBtnY = 630;    // Y position adjusted to image
int categoryBtnW = 140;    // Category button width (190-50=140)
int categoryBtnH = 115;    // Category button height (630 + 115 = 745)

int randomBtnX = 204;      // Random button in center: x1=204, x2=334
int randomBtnY = 630;      // Same Y position
int randomBtnW = 130;      // Random button width (334-204=130)
int randomBtnH = 115;      // Random button height (same as button 1)

int optionBtnX = 354;      // Option button on right: x1=354, x2=487
int optionBtnY = 630;      // Same Y position
int optionBtnW = 133;      // Option button width (487-354=133)
int optionBtnH = 115;      // Option button height (same as button 1)

// Load PNG from SD card and display it
bool loadPngFromFile(const char* filename) {
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.printf("Failed to open file: %s\n", filename);
    return false;
  }
  
  bool result = M5.Display.drawPng(&file, 0, 0);
  file.close();
  
  if (!result) {
    Serial.printf("Failed to draw PNG: %s\n", filename);
    return false;
  }
  
  return true;
}

void drawMenuPage() {
  M5.Display.clear();
  
  // Load and display menu image
  if (!loadPngFromFile("/flipcard/menu.png")) {
    // Fallback: draw simple menu if image fails to load
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(3);
    
    // Draw title
    M5.Display.drawString("Flipcard Menu", 140, 100);
    
    // Draw buttons
    M5.Display.fillRect(categoryBtnX, categoryBtnY, categoryBtnW, categoryBtnH, TFT_BLUE);
    M5.Display.fillRect(randomBtnX, randomBtnY, randomBtnW, randomBtnH, TFT_GREEN);
    M5.Display.fillRect(optionBtnX, optionBtnY, optionBtnW, optionBtnH, TFT_RED);
    
    // Button labels
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Category", categoryBtnX + 10, categoryBtnY + 45);
    M5.Display.drawString("Random", randomBtnX + 20, randomBtnY + 45);
    M5.Display.drawString("Option", optionBtnX + 20, optionBtnY + 45);
  }
}

int handleMenuTouch(int x, int y) {
  Serial.printf("Menu touch: x=%d, y=%d\n", x, y);
  
  // Check if Category button was touched
  if (x >= categoryBtnX && x <= categoryBtnX + categoryBtnW &&
      y >= categoryBtnY && y <= categoryBtnY + categoryBtnH) {
    Serial.println("Category button touched!");
    return 1; // Category mode
  }
  
  // Check Random button
  if (x >= randomBtnX && x <= randomBtnX + randomBtnW &&
      y >= randomBtnY && y <= randomBtnY + randomBtnH) {
    Serial.println("Random button touched!");
    return 2; // Random mode
  }
  
  // Check Option button
  if (x >= optionBtnX && x <= optionBtnX + optionBtnW &&
      y >= optionBtnY && y <= optionBtnY + optionBtnH) {
    Serial.println("Option button touched!");
    return 3; // Option mode (not implemented yet)
  }
  
  return 0; // No button touched
}