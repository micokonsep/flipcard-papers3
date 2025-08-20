#include "grid_page.h"
#include <M5Unified.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <vector>

// Helper function to load thumbnail using file handle
bool loadThumbnailFromCard(const char* folderPath, const char* thumbnailFile, int x, int y, int size) {
  String fullPath = "/flipcard/" + String(folderPath) + "/" + String(thumbnailFile);
  File file = SD.open(fullPath);
  if (!file) {
    return false;
  }
  
  bool result = M5.Display.drawPng(&file, x, y, size, size);
  file.close();
  
  return result;
}

// Function to draw grid navigation buttons (same as flipcard but different function)
void drawGridNavigationButtons(int currentPage, int totalPages) {
  int screenWidth = M5.Display.width();
  
  // Navigation buttons (80x80 pixels)
  int buttonSize = 80;
  int buttonMargin = 20;
  
  // Button positions (moved higher)
  int leftButtonX = buttonMargin + 25;  // 45px from left edge
  int leftButtonY = buttonMargin + 15;  // 35px from top edge (10px higher)
  int rightButtonX = screenWidth - buttonSize - buttonMargin - 25;  // 45px from right edge
  int rightButtonY = buttonMargin + 15;  // 35px from top edge (10px higher)
  int homeButtonX = (screenWidth - buttonSize) / 2;  // Centered horizontally
  int homeButtonY = buttonMargin + 15;  // Same Y as other buttons (10px higher)
  
  // Draw left button (always show, use grey version for single page)
  String leftButtonFile = (totalPages > 1) ? "/flipcard/Left.png" : "/flipcard/LeftGrey.png";
  Serial.printf("Loading left button: %s\n", leftButtonFile.c_str());
  File leftFile = SD.open(leftButtonFile);
  if (leftFile) {
    if (M5.Display.drawPng(&leftFile, leftButtonX, leftButtonY, buttonSize, buttonSize)) {
      Serial.println("Left button loaded successfully");
    } else {
      Serial.println("Failed to draw left button PNG");
    }
    leftFile.close();
  } else {
    Serial.println("Failed to open left button, drawing fallback");
    // Use gray colors for single page, normal colors for multi-page
    uint16_t bgColor = (totalPages > 1) ? 0x07E0 : 0xBDF7; // Green or light gray
    uint16_t textColor = (totalPages > 1) ? 0x0000 : 0x8410; // Black or dark gray
    
    M5.Display.fillRect(leftButtonX, leftButtonY, buttonSize, buttonSize, bgColor);
    M5.Display.drawRect(leftButtonX, leftButtonY, buttonSize, buttonSize, 0x0000);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(textColor);
    M5.Display.drawString("◀", leftButtonX + 25, leftButtonY + 25);
  }
  
  // Draw right button (always show, use grey version for single page)
  String rightButtonFile = (totalPages > 1) ? "/flipcard/Right.png" : "/flipcard/RightGrey.png";
  Serial.printf("Loading right button: %s\n", rightButtonFile.c_str());
  File rightFile = SD.open(rightButtonFile);
  if (rightFile) {
    if (M5.Display.drawPng(&rightFile, rightButtonX, rightButtonY, buttonSize, buttonSize)) {
      Serial.println("Right button loaded successfully");
    } else {
      Serial.println("Failed to draw right button PNG");
    }
    rightFile.close();
  } else {
    Serial.println("Failed to open right button, drawing fallback");
    // Use gray colors for single page, normal colors for multi-page
    uint16_t bgColor = (totalPages > 1) ? 0xF800 : 0xBDF7; // Red or light gray
    uint16_t textColor = (totalPages > 1) ? 0x0000 : 0x8410; // Black or dark gray
    
    M5.Display.fillRect(rightButtonX, rightButtonY, buttonSize, buttonSize, bgColor);
    M5.Display.drawRect(rightButtonX, rightButtonY, buttonSize, buttonSize, 0x0000);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(textColor);
    M5.Display.drawString("▶", rightButtonX + 25, rightButtonY + 25);
  }
  
  // Draw home button (back to flipcard)
  Serial.println("Loading home button for grid: /flipcard/Home.png");
  File homeFile = SD.open("/flipcard/Home.png");
  if (homeFile) {
    if (M5.Display.drawPng(&homeFile, homeButtonX, homeButtonY, buttonSize, buttonSize)) {
      Serial.println("Home button loaded successfully");
    } else {
      Serial.println("Failed to draw home button PNG");
    }
    homeFile.close();
  } else {
    Serial.println("Failed to open home button, drawing fallback");
    M5.Display.fillRect(homeButtonX, homeButtonY, buttonSize, buttonSize, 0x001F); // Blue
    M5.Display.drawRect(homeButtonX, homeButtonY, buttonSize, buttonSize, 0x0000);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(0xFFFF);
    M5.Display.drawString("H", homeButtonX + 30, homeButtonY + 25);
  }
  
  // Draw page indicator
  if (totalPages > 1) {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0x0000);
    String pageText = "Page " + String(currentPage + 1) + "/" + String(totalPages);
    int textWidth = M5.Display.textWidth(pageText);
    int textX = (screenWidth - textWidth) / 2;
    int textY = homeButtonY + buttonSize + 15;
  }
}

// Main grid display function
void drawGridPage(JsonDocument& indexData, int gridPage, int totalGridPages) {
  int screenWidth = M5.Display.width();
  int screenHeight = M5.Display.height();
  
  // Clear screen with white background
  M5.Display.clear();
  
  Serial.println("=== Drawing Grid Page ===");
  Serial.printf("Grid Page: %d/%d\n", gridPage + 1, totalGridPages);
  
  // Draw navigation buttons first
  drawGridNavigationButtons(gridPage, totalGridPages);
  
  // Grid layout configuration for 540x960 fullscreen
  int thumbnailSize = 120;  // Back to 120px as requested
  int cols = 3;
  int rows = 5;
  int maxThumbnails = cols * rows; // 15 thumbnails per page
  
  // Spacing configuration (calculated for perfect balance)
  // Available width: 540 - (3 × 120) = 180px
  // Divided by 4 spaces (2 margins + 2 gaps) = 45px each
  int horizontalSpacing = 45;
  int verticalSpacing = 30;    // Generous vertical spacing
  
  // Calculate grid area (below navigation buttons, moved lower)
  int gridStartY = 170; // Start below navigation buttons and page indicator (20px lower)
  int gridWidth = (cols * thumbnailSize) + ((cols - 1) * horizontalSpacing);
  int gridHeight = (rows * thumbnailSize) + ((rows - 1) * verticalSpacing);
  int gridStartX = (screenWidth - gridWidth) / 2; // Center horizontally
  
  // Calculate cards for this page
  int totalCards = indexData["metadata"]["total_cards"];
  int startCardIndex = gridPage * maxThumbnails;
  int endCardIndex = min(startCardIndex + maxThumbnails, totalCards);
  
  Serial.printf("Displaying cards %d to %d\n", startCardIndex, endCardIndex - 1);
  
  // Draw all grid slots (15 total)
  for (int gridPos = 0; gridPos < maxThumbnails; gridPos++) {
    int col = gridPos % cols;
    int row = gridPos / cols;
    
    int thumbX = gridStartX + (col * (thumbnailSize + horizontalSpacing));
    int thumbY = gridStartY + (row * (thumbnailSize + verticalSpacing));
    
    int cardIndex = startCardIndex + gridPos; // Global card index
    
    if (cardIndex < totalCards) {
      // Draw actual thumbnail for existing card
      String cardFolder = indexData["cards"][cardIndex]["folder"];
      String cardTitle = indexData["cards"][cardIndex]["title"];
      String thumbnailFile = indexData["cards"][cardIndex]["thumbnail"];
      
      Serial.printf("Loading thumbnail %d: %s/%s\n", cardIndex, cardFolder.c_str(), thumbnailFile.c_str());
      
      // Draw thumbnail
      if (!loadThumbnailFromCard(cardFolder.c_str(), thumbnailFile.c_str(), thumbX, thumbY, thumbnailSize)) {
        Serial.printf("Failed to load thumbnail for card %d, drawing fallback\n", cardIndex);
        // Draw fallback for failed load
        M5.Display.fillRect(thumbX, thumbY, thumbnailSize, thumbnailSize, 0xBDF7); // Light gray
        
        // Draw card number in fallback
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(0x0000);
        String cardNum = String(cardIndex + 1);
        int textX = thumbX + (thumbnailSize - M5.Display.textWidth(cardNum)) / 2;
        int textY = thumbY + (thumbnailSize / 2) - 10;
        M5.Display.drawString(cardNum, textX, textY);
      }
      
      // Draw border around existing thumbnail (black border)
      M5.Display.drawRect(thumbX, thumbY, thumbnailSize, thumbnailSize, 0x0000);
      
    } else {
      // Draw empty grid slot for non-existing cards
      Serial.printf("Drawing empty grid slot at position %d\n", gridPos);
      
      // Draw empty slot with lighter background (polos tanpa icon)
      M5.Display.fillRect(thumbX, thumbY, thumbnailSize, thumbnailSize, 0xF7DE); // Very light gray
      
      // Draw border around empty slot (same black border as existing thumbnails)
      M5.Display.drawRect(thumbX, thumbY, thumbnailSize, thumbnailSize, 0x0000);
    }
  }
  
  Serial.println("=== Grid Page Complete ===");
}

// Function to detect which thumbnail was touched
int getTouchedThumbnailIndex(int touchX, int touchY, int gridPage) {
  // Grid layout configuration (same as drawGridPage)
  int thumbnailSize = 120;
  int cols = 3;
  int rows = 5;
  int screenWidth = M5.Display.width();
  
  // Spacing configuration (same as drawGridPage)
  int horizontalSpacing = 45;
  int verticalSpacing = 30;
  
  // Calculate grid area
  int gridStartY = 170;
  int gridWidth = (cols * thumbnailSize) + ((cols - 1) * horizontalSpacing);
  int gridHeight = (rows * thumbnailSize) + ((rows - 1) * verticalSpacing);
  int gridStartX = (screenWidth - gridWidth) / 2;
  
  // Check if touch is within grid area
  if (touchX < gridStartX || touchX > gridStartX + gridWidth ||
      touchY < gridStartY || touchY > gridStartY + gridHeight) {
    return -1; // Touch outside grid
  }
  
  // Calculate which thumbnail was touched (accounting for spacing)
  int relativeX = touchX - gridStartX;
  int relativeY = touchY - gridStartY;
  
  // Find the column and row considering spacing
  int col = -1, row = -1;
  
  // Check each column
  for (int c = 0; c < cols; c++) {
    int colStart = c * (thumbnailSize + horizontalSpacing);
    int colEnd = colStart + thumbnailSize;
    if (relativeX >= colStart && relativeX <= colEnd) {
      col = c;
      break;
    }
  }
  
  // Check each row
  for (int r = 0; r < rows; r++) {
    int rowStart = r * (thumbnailSize + verticalSpacing);
    int rowEnd = rowStart + thumbnailSize;
    if (relativeY >= rowStart && relativeY <= rowEnd) {
      row = r;
      break;
    }
  }
  
  if (col == -1 || row == -1) {
    return -1; // Touch in spacing area
  }
  
  int gridPos = (row * cols) + col; // 0-14
  int cardIndex = (gridPage * 15) + gridPos; // Global card index
  
  Serial.printf("Touch at (%d,%d) -> Grid pos: %d, Card index: %d\n", 
                touchX, touchY, gridPos, cardIndex);
  
  // Check if this is an empty slot (no card exists)
  // We need to get totalCards from somewhere - let's add it as parameter
  // For now, return cardIndex and let main.cpp handle the bounds check
  return cardIndex;
}

// Function to detect navigation button touches
bool isTouchOnGridNavButton(int touchX, int touchY, String& buttonType) {
  int screenWidth = M5.Display.width();
  int buttonSize = 80;
  int buttonMargin = 20;
  
  // Button positions (same as drawGridNavigationButtons)
  int leftButtonX = buttonMargin + 25;
  int leftButtonY = buttonMargin + 25;
  int rightButtonX = screenWidth - buttonSize - buttonMargin - 25;
  int rightButtonY = buttonMargin + 25;
  int homeButtonX = (screenWidth - buttonSize) / 2;
  int homeButtonY = buttonMargin + 25;
  
  // Check left button
  if (touchX >= leftButtonX && touchX <= leftButtonX + buttonSize && 
      touchY >= leftButtonY && touchY <= leftButtonY + buttonSize) {
    buttonType = "left";
    return true;
  }
  
  // Check right button
  if (touchX >= rightButtonX && touchX <= rightButtonX + buttonSize && 
      touchY >= rightButtonY && touchY <= rightButtonY + buttonSize) {
    buttonType = "right";
    return true;
  }
  
  // Check home button
  if (touchX >= homeButtonX && touchX <= homeButtonX + buttonSize && 
      touchY >= homeButtonY && touchY <= homeButtonY + buttonSize) {
    buttonType = "home";
    return true;
  }
  
  return false;
}

// Helper function to get count of cards in category
int getFilteredCardCount(JsonDocument& indexData, String categoryFilter) {
  if (categoryFilter == "") {
    return indexData["metadata"]["total_cards"].as<int>();
  }
  
  int count = 0;
  JsonArray cards = indexData["cards"];
  for (JsonObject card : cards) {
    if (card["category"].as<String>() == categoryFilter) {
      count++;
    }
  }
  return count;
}

// Helper function to convert filtered index to global card index
int getFilteredCardGlobalIndex(JsonDocument& indexData, String categoryFilter, int filteredIndex) {
  if (categoryFilter == "") {
    return filteredIndex; // No filtering, same index
  }
  
  int count = 0;
  JsonArray cards = indexData["cards"];
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (card["category"].as<String>() == categoryFilter) {
      if (count == filteredIndex) {
        return i; // Return global index
      }
      count++;
    }
  }
  return -1; // Not found
}

// Draw grid page with category filtering
void drawGridPageFiltered(JsonDocument& indexData, int gridPage, int totalGridPages, String categoryFilter) {
  auto& display = M5.Display;
  display.clear();
  
  // Draw navigation buttons
  drawGridNavigationButtons(gridPage, totalGridPages);
  
  // Grid configuration
  int cols = 3;
  int rows = 5;
  int cardsPerPage = cols * rows;
  int thumbnailSize = 120;
  int thumbnailMargin = 45;
  
  // Calculate total filtered cards
  int totalFilteredCards = getFilteredCardCount(indexData, categoryFilter);
  
  JsonArray cards = indexData["cards"];
  
  // Build filtered card list for current page
  std::vector<int> filteredCardIndices;
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (categoryFilter == "" || card["category"].as<String>() == categoryFilter) {
      filteredCardIndices.push_back(i);
    }
  }
  
  // Calculate start index for current page
  int startCardIndex = gridPage * cardsPerPage;
  
  // Draw thumbnails for current page
  for (int slot = 0; slot < cardsPerPage; slot++) {
    int cardIndex = startCardIndex + slot;
    
    // Calculate grid position
    int col = slot % cols;
    int row = slot / cols;
    int x = thumbnailMargin + col * (thumbnailSize + thumbnailMargin);
    int y = 140 + row * (thumbnailSize + thumbnailMargin); // Start below nav buttons
    
    if (cardIndex < filteredCardIndices.size()) {
      // Draw actual card thumbnail
      int globalCardIndex = filteredCardIndices[cardIndex];
      JsonObject card = cards[globalCardIndex];
      
      String folderPath = card["folder"].as<String>();
      String thumbnailFile = card["thumbnail"].as<String>();
      
      if (!loadThumbnailFromCard(folderPath.c_str(), thumbnailFile.c_str(), x, y, thumbnailSize)) {
        // Fallback if thumbnail fails
        display.fillRect(x, y, thumbnailSize, thumbnailSize, TFT_LIGHTGREY);
        display.setTextColor(TFT_BLACK);
        display.setTextSize(1);
        display.setCursor(x + 10, y + 50);
        display.print(card["title"].as<String>());
      }
      
      // Draw border around existing thumbnail (black border)
      display.drawRect(x, y, thumbnailSize, thumbnailSize, TFT_BLACK);
    } else {
      // Draw empty slot (polos tanpa icon)
      display.fillRect(x, y, thumbnailSize, thumbnailSize, TFT_LIGHTGREY);
      
      // Draw border around empty slot (same black border as existing thumbnails)
      display.drawRect(x, y, thumbnailSize, thumbnailSize, TFT_BLACK);
    }
  }
  
  display.display();
}

// Get touched thumbnail index for filtered cards
int getTouchedThumbnailIndexFiltered(int touchX, int touchY, int gridPage, JsonDocument& indexData, String categoryFilter) {
  // Grid configuration (same as drawGridPageFiltered)
  int cols = 3;
  int rows = 5;
  int cardsPerPage = cols * rows;
  int thumbnailSize = 120;
  int thumbnailMargin = 45;
  int startY = 140; // Start below nav buttons
  
  // Build filtered card list
  std::vector<int> filteredCardIndices;
  JsonArray cards = indexData["cards"];
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (categoryFilter == "" || card["category"].as<String>() == categoryFilter) {
      filteredCardIndices.push_back(i);
    }
  }
  
  // Check each thumbnail position
  for (int slot = 0; slot < cardsPerPage; slot++) {
    int col = slot % cols;
    int row = slot / cols;
    int x = thumbnailMargin + col * (thumbnailSize + thumbnailMargin);
    int y = startY + row * (thumbnailSize + thumbnailMargin);
    
    // Check if touch is within this thumbnail
    if (touchX >= x && touchX <= x + thumbnailSize && 
        touchY >= y && touchY <= y + thumbnailSize) {
      
      int cardIndex = gridPage * cardsPerPage + slot;
      
      // Check if this slot has a card
      if (cardIndex < filteredCardIndices.size()) {
        return filteredCardIndices[cardIndex]; // Return global card index
      }
    }
  }
  
  return -1; // No thumbnail touched
}