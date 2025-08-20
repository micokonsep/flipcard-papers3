#include "flipcard_page.h"
#include <M5Unified.h>
#include <SD.h>
#include <ArduinoJson.h>

// Helper function to load PNG using file handle
bool loadPngFromFile(const char* filename, int x, int y, int width, int height) {
  File file = SD.open(filename);
  if (!file) {
    return false;
  }
  
  bool result = M5.Display.drawPng(&file, x, y, width, height);
  file.close();
  
  return result;
}

// Function to draw navigation buttons (separated for modularity)
void drawNavigationButtons() {
  int screenWidth = M5.Display.width();
  
  // Draw navigation buttons (80x80 pixels)
  int buttonSize = 80;
  int buttonMargin = 20;
  
  // Left button position (add 25px to both top and left)
  int leftButtonX = buttonMargin + 25;  // 45px from left edge
  int leftButtonY = buttonMargin + 25;  // 45px from top edge
  
  // Right button position (add 25px to both top and right)
  int rightButtonX = screenWidth - buttonSize - buttonMargin - 25;  // 45px from right edge
  int rightButtonY = buttonMargin + 25;  // 45px from top edge
  
  // Home button position (center between left and right)
  int homeButtonX = (screenWidth - buttonSize) / 2;  // Centered horizontally
  int homeButtonY = buttonMargin + 25;  // Same Y as other buttons (45px from top)
  
  // Draw left button
  Serial.println("Loading left button: /flipcard/Left.png");
  if (!loadPngFromFile("/flipcard/Left.png", leftButtonX, leftButtonY, buttonSize, buttonSize)) {
    Serial.println("Failed to load left button");
    M5.Display.fillRect(leftButtonX, leftButtonY, buttonSize, buttonSize, 0x07E0); // Green fallback
    M5.Display.drawRect(leftButtonX, leftButtonY, buttonSize, buttonSize, 0x0000);
  }
  
  // Draw right button
  Serial.println("Loading right button: /flipcard/Right.png");
  if (!loadPngFromFile("/flipcard/Right.png", rightButtonX, rightButtonY, buttonSize, buttonSize)) {
    Serial.println("Failed to load right button");
    M5.Display.fillRect(rightButtonX, rightButtonY, buttonSize, buttonSize, 0xF800); // Red fallback
    M5.Display.drawRect(rightButtonX, rightButtonY, buttonSize, buttonSize, 0x0000);
  }
  
  // Draw home button (Home.png)
  Serial.println("Loading home button: /flipcard/Home.png");
  if (!loadPngFromFile("/flipcard/Home.png", homeButtonX, homeButtonY, buttonSize, buttonSize)) {
    Serial.println("Failed to load home button");
    M5.Display.fillRect(homeButtonX, homeButtonY, buttonSize, buttonSize, 0x001F); // Blue fallback
    M5.Display.drawRect(homeButtonX, homeButtonY, buttonSize, buttonSize, 0x0000);
  }
}

// Main flipcard display function (JSON-driven)
void drawFlipcard(JsonDocument& cardData, String folderPath, String currentLanguage) {
  int screenWidth = M5.Display.width();
  int screenHeight = M5.Display.height();
  
  // Get filenames from JSON using dynamic language key
  String bigImageFile = cardData["languages"][currentLanguage]["big_file"];
  String smallImageFile = cardData["languages"][currentLanguage]["small_file"];
  String mainImageFile = cardData["main_image"];
  
  // Build full paths
  String bigImagePath = "/flipcard/" + folderPath + "/" + bigImageFile;
  String smallImagePath = "/flipcard/" + folderPath + "/" + smallImageFile;
  String mainImagePath = "/flipcard/" + folderPath + "/" + mainImageFile;
  
  Serial.println("=== Drawing Flipcard Layout (JSON) ===");
  Serial.printf("Card: %s\n", cardData["title"].as<String>().c_str());
  Serial.printf("Language: %s\n", currentLanguage.c_str());
  Serial.printf("Big: %s\n", bigImagePath.c_str());
  Serial.printf("Small: %s\n", smallImagePath.c_str());
  Serial.printf("Main: %s\n", mainImagePath.c_str());
  
  // Calculate positions (same as before)
  int bigWidth = 400, bigHeight = 150;
  int smallWidth = 400, smallHeight = 80;
  int mainWidth = 400, mainHeight = 400;
  
  int bigX = (screenWidth - bigWidth) / 2;
  int smallX = (screenWidth - smallWidth) / 2;
  int mainX = (screenWidth - mainWidth) / 2;
  
  int bigY = 180;
  int smallY = bigY + bigHeight + 5;
  int mainY = screenHeight - mainHeight - 75;
  
  // Draw navigation buttons
  drawNavigationButtons();
  
  // Draw big image
  if (!loadPngFromFile(bigImagePath.c_str(), bigX, bigY, bigWidth, bigHeight)) {
    Serial.println("Failed to load big image");
    M5.Display.fillRect(bigX, bigY, bigWidth, bigHeight, 0xF800);
  } else {
    Serial.println("Big image loaded successfully");
  }
  
  // Draw small image
  if (!loadPngFromFile(smallImagePath.c_str(), smallX, smallY, smallWidth, smallHeight)) {
    Serial.println("Failed to load small image");
    M5.Display.fillRect(smallX, smallY, smallWidth, smallHeight, 0x07E0);
  } else {
    Serial.println("Small image loaded successfully");
  }
  
  // Draw main image
  if (!loadPngFromFile(mainImagePath.c_str(), mainX, mainY, mainWidth, mainHeight)) {
    Serial.println("Failed to load main image");
    M5.Display.fillRect(mainX, mainY, mainWidth, mainHeight, 0x001F);
  } else {
    Serial.println("Main image loaded successfully");
  }
  
  Serial.println("=== Flipcard Layout Complete (JSON) ===");
}

// Language refresh function (JSON-driven)
void refreshLanguageImages(JsonDocument& cardData, String folderPath, String currentLanguage) {
  int screenWidth = M5.Display.width();
  
  // Get filenames from JSON using dynamic language key
  String bigImageFile = cardData["languages"][currentLanguage]["big_file"];
  String smallImageFile = cardData["languages"][currentLanguage]["small_file"];
  
  // Build full paths
  String bigImagePath = "/flipcard/" + folderPath + "/" + bigImageFile;
  String smallImagePath = "/flipcard/" + folderPath + "/" + smallImageFile;
  
  // Calculate positions (same as main function)
  int bigWidth = 400, bigHeight = 150;
  int smallWidth = 400, smallHeight = 80;
  
  int bigX = (screenWidth - bigWidth) / 2;
  int smallX = (screenWidth - smallWidth) / 2;
  int bigY = 180;
  int smallY = bigY + bigHeight + 5;
  
  Serial.printf("Refreshing language images (JSON) to: %s\n", currentLanguage.c_str());
  Serial.printf("Big: %s\n", bigImagePath.c_str());
  Serial.printf("Small: %s\n", smallImagePath.c_str());
  
  // Clear and redraw big image area
  M5.Display.fillRect(bigX, bigY, bigWidth, bigHeight, 0xFFFF);
  if (!loadPngFromFile(bigImagePath.c_str(), bigX, bigY, bigWidth, bigHeight)) {
    Serial.println("Failed to load big image");
    M5.Display.fillRect(bigX, bigY, bigWidth, bigHeight, 0xF800);
  }
  
  // Clear and redraw small image area
  M5.Display.fillRect(smallX, smallY, smallWidth, smallHeight, 0xFFFF);
  if (!loadPngFromFile(smallImagePath.c_str(), smallX, smallY, smallWidth, smallHeight)) {
    Serial.println("Failed to load small image");
    M5.Display.fillRect(smallX, smallY, smallWidth, smallHeight, 0x07E0);
  }
}

