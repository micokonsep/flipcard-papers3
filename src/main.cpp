#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <ArduinoJson.h>
#include <vector>
#include "pages/empty_frame_page.h"
#include "pages/flipcard_page.h"
#include "pages/grid_page.h"
#include "pages/category_page.h"
#include "pages/menu_page.h"
#include "pages/option_page.h"

#define SD_SPI_CS_PIN   47
#define SD_SPI_SCK_PIN  39
#define SD_SPI_MOSI_PIN 38
#define SD_SPI_MISO_PIN 40

// Global state variables
int currentLanguageIndex = 0;  // Index in enabled languages array
int currentCardIndex = 0;      // Index in cards array (0-based)
int totalCards = 0;            // Total number of cards loaded from JSON
int maxCardIndex = 0;          // Maximum index (totalCards - 1)

// Page navigation state
enum PageMode { MENU_MODE, CATEGORY_MODE, GRID_MODE, FLIPCARD_MODE, OPTION_MODE, LANGUAGE_SELECTION_MODE };
PageMode currentPageMode = MENU_MODE;  // Start with menu page
int currentGridPage = 0;      // Current page in grid view (0-based)
int totalGridPages = 0;       // Total pages in grid view
String selectedCategory = ""; // Selected category for filtering grid

// Random mode state
bool isRandomMode = false;    // Track if we're in random mode
String lastRandomCardId = ""; // Track last random card to avoid duplicates

// Language configuration
JsonDocument languageDoc;     // Document to hold enabled languages array
JsonArray enabledLanguages;   // Array of enabled language keys
String defaultLanguage;       // Default language from config

// JSON documents
JsonDocument indexDoc;
JsonDocument currentCardDoc;
JsonDocument configDoc;

// Sleep functionality variables
unsigned long lastActivityTime = 0;
const unsigned long SLEEP_TIMEOUT = 3 * 60 * 1000; // 3 minutes in milliseconds

// Function to reset language index to default language
void resetToDefaultLanguage() {
  currentLanguageIndex = 0; // fallback to first language
  for (int i = 0; i < enabledLanguages.size(); i++) {
    if (enabledLanguages[i] == defaultLanguage) {
      currentLanguageIndex = i;
      break;
    }
  }
  Serial.printf("Reset to default language: %s (index %d)\n", defaultLanguage.c_str(), currentLanguageIndex);
}

// Function to load config.json
bool loadConfig() {
  File file = SD.open("/flipcard/config.json");
  if (!file) {
    Serial.println("Failed to open config.json");
    return false;
  }
  
  DeserializationError error = deserializeJson(configDoc, file);
  file.close();
  
  if (error) {
    Serial.printf("Failed to parse config.json: %s\n", error.c_str());
    return false;
  }
  
  // Load default language
  defaultLanguage = configDoc["languages"]["default"].as<String>();
  
  // Create enabled languages array in persistent document
  enabledLanguages = languageDoc.add<JsonArray>();
  JsonObject supportedLangs = configDoc["languages"]["supported"];
  
  for (JsonPair lang : supportedLangs) {
    if (lang.value()["enabled"]) {
      enabledLanguages.add(lang.key().c_str());
    }
  }
  
  // Set current language index to default language
  resetToDefaultLanguage();
  
  Serial.printf("Loaded config: %d enabled languages\n", enabledLanguages.size());
  Serial.printf("Default language: %s (index %d)\n", defaultLanguage.c_str(), currentLanguageIndex);
  
  return true;
}

// Function to load index.json
bool loadIndex() {
  File file = SD.open("/flipcard/index.json");
  if (!file) {
    Serial.println("Failed to open index.json");
    return false;
  }
  
  DeserializationError error = deserializeJson(indexDoc, file);
  file.close();
  
  if (error) {
    Serial.printf("Failed to parse index.json: %s\n", error.c_str());
    return false;
  }
  
  totalCards = indexDoc["metadata"]["total_cards"];
  maxCardIndex = totalCards - 1;
  
  // Calculate grid pages (15 thumbnails per page)
  totalGridPages = (totalCards + 14) / 15; // Ceiling division
  
  Serial.printf("Loaded index: %d cards, %d grid pages\n", totalCards, totalGridPages);
  return true;
}

// Function to load individual card JSON
bool loadCard(int cardIndex) {
  if (cardIndex < 0 || cardIndex >= totalCards) {
    Serial.printf("Invalid card index: %d\n", cardIndex);
    return false;
  }
  
  String cardId = indexDoc["cards"][cardIndex]["id"];
  String folder = indexDoc["cards"][cardIndex]["folder"];
  String cardFile = "/flipcard/" + folder + "/card.json";
  
  File file = SD.open(cardFile);
  if (!file) {
    Serial.printf("Failed to open %s\n", cardFile.c_str());
    return false;
  }
  
  DeserializationError error = deserializeJson(currentCardDoc, file);
  file.close();
  
  if (error) {
    Serial.printf("Failed to parse %s: %s\n", cardFile.c_str(), error.c_str());
    return false;
  }
  
  Serial.printf("Loaded card: %s\n", cardId.c_str());
  return true;
}

// Function to get current card ID from index
String getCurrentCardId() {
  if (currentCardIndex >= 0 && currentCardIndex < totalCards) {
    return indexDoc["cards"][currentCardIndex]["id"];
  }
  // Dynamic fallback: return first card if available
  if (totalCards > 0) {
    return indexDoc["cards"][0]["id"];
  }
  return ""; // empty if no cards available
}

// Function to get current card folder from index
String getCurrentCardFolder() {
  if (currentCardIndex >= 0 && currentCardIndex < totalCards) {
    return indexDoc["cards"][currentCardIndex]["folder"];
  }
  // Dynamic fallback: return first card folder if available
  if (totalCards > 0) {
    return indexDoc["cards"][0]["folder"];
  }
  return ""; // empty if no cards available
}

// Function to get current language key
String getCurrentLanguage() {
  if (currentLanguageIndex >= 0 && currentLanguageIndex < enabledLanguages.size()) {
    return enabledLanguages[currentLanguageIndex];
  }
  return defaultLanguage; // fallback
}

// Function to get random card index from category (excluding specific card)
int getRandomCardFromCategory(String category, String excludeCardId) {
  std::vector<int> availableIndices;
  JsonArray cards = indexDoc["cards"];
  
  // Collect all card indices in the category except excluded one
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (card["category"].as<String>() == category) {
      String cardId = card["id"].as<String>();
      if (cardId != excludeCardId) {
        availableIndices.push_back(i);
      }
    }
  }
  
  // If no other cards available, return current card index
  if (availableIndices.empty()) {
    // Find current card index as fallback
    for (int i = 0; i < cards.size(); i++) {
      if (cards[i]["id"].as<String>() == excludeCardId) {
        return i;
      }
    }
    return 0; // ultimate fallback
  }
  
  // Return random index from available cards
  int randomIndex = random(availableIndices.size());
  return availableIndices[randomIndex];
}

// Function to go to menu page mode
void goToMenuMode() {
  currentPageMode = MENU_MODE;
  isRandomMode = false; // Reset random mode when going back to menu
  selectedCategory = ""; // Clear category selection
  Serial.println("Switched to menu mode");
  drawMenuPage();
}

// Function to go to option page mode
void goToOptionMode() {
  currentPageMode = OPTION_MODE;
  Serial.println("Switched to option mode");
  drawOptionPage();
}

// Function to go to language selection mode
void goToLanguageSelectionMode() {
  currentPageMode = LANGUAGE_SELECTION_MODE;
  Serial.println("Switched to language selection mode");
  drawLanguageSelectionPage(configDoc);
}

// Function to go to category page mode
void goToCategoryMode() {
  currentPageMode = CATEGORY_MODE;
  if (isRandomMode) {
    Serial.println("Switched to category mode (Random)");
    drawCategoryPage(indexDoc, true);
  } else {
    Serial.println("Switched to category mode");
    drawCategoryPage(indexDoc, false);
  }
}

// Function to go to grid page mode
void goToGridMode() {
  currentPageMode = GRID_MODE;
  // Reset to first page when entering grid mode
  currentGridPage = 0;
  
  // Calculate total grid pages based on filtering
  int filteredCardCount;
  if (selectedCategory != "") {
    filteredCardCount = getFilteredCardCount(indexDoc, selectedCategory);
  } else {
    filteredCardCount = totalCards;
  }
  totalGridPages = (filteredCardCount + 14) / 15; // 15 cards per page
  if (totalGridPages < 1) totalGridPages = 1;
  
  Serial.printf("Switched to grid mode, page %d/%d", currentGridPage + 1, totalGridPages);
  if (selectedCategory != "") {
    Serial.printf(" (category: %s, %d cards)", selectedCategory.c_str(), filteredCardCount);
  }
  Serial.println();
  
  // Use filtered or normal grid drawing
  if (selectedCategory != "") {
    drawGridPageFiltered(indexDoc, currentGridPage, totalGridPages, selectedCategory);
  } else {
    drawGridPage(indexDoc, currentGridPage, totalGridPages);
  }
}

// Function to go to flipcard mode
void goToFlipcardMode(int cardIndex) {
  currentPageMode = FLIPCARD_MODE;
  currentCardIndex = cardIndex;
  
  // Load the selected card data
  if (loadCard(currentCardIndex)) {
    String folderPath = getCurrentCardFolder();
    resetToDefaultLanguage(); // Reset to default language
    String currentLang = getCurrentLanguage();
    
    Serial.printf("Switched to flipcard mode, card %s\n", getCurrentCardId().c_str());
    
    // Draw flipcard
    drawEmptyFrame();
    drawFlipcard(currentCardDoc, folderPath, currentLang);
  } else {
    Serial.println("Failed to load selected card");
  }
}

// Function to navigate grid pages
void goToPreviousGridPage() {
  currentGridPage--;
  if (currentGridPage < 0) {
    currentGridPage = totalGridPages - 1; // Loop to last page
  }
  Serial.printf("Grid page: %d/%d\n", currentGridPage + 1, totalGridPages);
  
  // Use filtered or normal grid drawing
  if (selectedCategory != "") {
    drawGridPageFiltered(indexDoc, currentGridPage, totalGridPages, selectedCategory);
  } else {
    drawGridPage(indexDoc, currentGridPage, totalGridPages);
  }
}

void goToNextGridPage() {
  currentGridPage++;
  if (currentGridPage >= totalGridPages) {
    currentGridPage = 0; // Loop to first page
  }
  Serial.printf("Grid page: %d/%d\n", currentGridPage + 1, totalGridPages);
  
  // Use filtered or normal grid drawing
  if (selectedCategory != "") {
    drawGridPageFiltered(indexDoc, currentGridPage, totalGridPages, selectedCategory);
  } else {
    drawGridPage(indexDoc, currentGridPage, totalGridPages);
  }
}

// Helper functions for filtered card navigation
int getFilteredCardIndex(int globalCardIndex) {
  if (selectedCategory == "") {
    return globalCardIndex; // No filtering
  }
  
  JsonArray cards = indexDoc["cards"];
  int filteredIndex = 0;
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (card["category"].as<String>() == selectedCategory) {
      if (i == globalCardIndex) {
        return filteredIndex;
      }
      filteredIndex++;
    }
  }
  return -1; // Not found in filter
}

int getGlobalCardIndexFromFiltered(int filteredIndex) {
  if (selectedCategory == "") {
    return filteredIndex; // No filtering
  }
  
  JsonArray cards = indexDoc["cards"];
  int count = 0;
  for (int i = 0; i < cards.size(); i++) {
    JsonObject card = cards[i];
    if (card["category"].as<String>() == selectedCategory) {
      if (count == filteredIndex) {
        return i;
      }
      count++;
    }
  }
  return -1; // Not found
}

int getFilteredCardCount() {
  if (selectedCategory == "") {
    return totalCards;
  }
  
  int count = 0;
  JsonArray cards = indexDoc["cards"];
  for (JsonObject card : cards) {
    if (card["category"].as<String>() == selectedCategory) {
      count++;
    }
  }
  return count;
}

// Function to cycle to next language
void cycleToNextLanguage() {
  Serial.printf("cycleToNextLanguage called. enabledLanguages.size(): %d\n", enabledLanguages.size());
  if (enabledLanguages.size() > 1) {
    int oldIndex = currentLanguageIndex;
    currentLanguageIndex = (currentLanguageIndex + 1) % enabledLanguages.size();
    String currentLang = getCurrentLanguage();
    Serial.printf("Language switched from index %d to %d: %s\n", oldIndex, currentLanguageIndex, currentLang.c_str());
  } else {
    Serial.println("Cannot cycle languages: only 1 or 0 languages enabled");
  }
}

// Function to navigate to previous card (circular, respects category filter)
void goToPreviousCard() {
  if (selectedCategory == "") {
    // No filtering - use original logic
    currentCardIndex--;
    if (currentCardIndex < 0) {
      currentCardIndex = maxCardIndex; // Loop to last card
      Serial.printf("Reached beginning, looping to last card index: %d\n", currentCardIndex);
    }
  } else {
    // With filtering - navigate within filtered cards only
    int currentFilteredIndex = getFilteredCardIndex(currentCardIndex);
    if (currentFilteredIndex == -1) {
      Serial.println("Current card not in filter - staying put");
      return;
    }
    
    int filteredCardCount = getFilteredCardCount();
    currentFilteredIndex--;
    if (currentFilteredIndex < 0) {
      currentFilteredIndex = filteredCardCount - 1; // Loop to last filtered card
    }
    
    currentCardIndex = getGlobalCardIndexFromFiltered(currentFilteredIndex);
    if (currentCardIndex == -1) {
      Serial.println("Failed to find filtered card - staying put");
      return;
    }
    
    Serial.printf("Filtered navigation: previous card (filtered index %d -> global index %d)\n", 
                  currentFilteredIndex, currentCardIndex);
  }
  
  // Load the new card data
  if (loadCard(currentCardIndex)) {
    String folderPath = getCurrentCardFolder();
    resetToDefaultLanguage(); // Reset to default language when changing cards
    String currentLang = getCurrentLanguage();
    
    Serial.printf("Navigate to previous card: %s\n", getCurrentCardId().c_str());
    
    // Redraw full flipcard using JSON data
    drawEmptyFrame();
    drawFlipcard(currentCardDoc, folderPath, currentLang);
  } else {
    Serial.println("Failed to load previous card");
  }
}

// Function to navigate to next card (circular, respects category filter)
void goToNextCard() {
  if (selectedCategory == "") {
    // No filtering - use original logic
    currentCardIndex++;
    if (currentCardIndex > maxCardIndex) {
      currentCardIndex = 0; // Loop to first card
      Serial.printf("Reached end, looping to first card index: %d\n", currentCardIndex);
    }
  } else {
    // With filtering - navigate within filtered cards only
    int currentFilteredIndex = getFilteredCardIndex(currentCardIndex);
    if (currentFilteredIndex == -1) {
      Serial.println("Current card not in filter - staying put");
      return;
    }
    
    int filteredCardCount = getFilteredCardCount();
    currentFilteredIndex++;
    if (currentFilteredIndex >= filteredCardCount) {
      currentFilteredIndex = 0; // Loop to first filtered card
    }
    
    currentCardIndex = getGlobalCardIndexFromFiltered(currentFilteredIndex);
    if (currentCardIndex == -1) {
      Serial.println("Failed to find filtered card - staying put");
      return;
    }
    
    Serial.printf("Filtered navigation: next card (filtered index %d -> global index %d)\n", 
                  currentFilteredIndex, currentCardIndex);
  }
  
  // Load the new card data
  if (loadCard(currentCardIndex)) {
    String folderPath = getCurrentCardFolder();
    resetToDefaultLanguage(); // Reset to default language when changing cards
    String currentLang = getCurrentLanguage();
    
    Serial.printf("Navigate to next card: %s\n", getCurrentCardId().c_str());
    
    // Redraw full flipcard using JSON data
    drawEmptyFrame();
    drawFlipcard(currentCardDoc, folderPath, currentLang);
  } else {
    Serial.println("Failed to load next card");
  }
}

// Function to display lock screen before sleep
void displayLockScreen() {
  // Clear screen first
  M5.Display.fillScreen(TFT_WHITE);
  
  // Try to display screensaver image from SD card
  if (SD.exists("/flipcard/screensaver/Thousand-Miles1.png")) {
    // Draw the PNG image from top-left corner (0,0)
    M5.Display.drawPngFile(SD, "/flipcard/screensaver/Thousand-Miles1.png", 0, 0);
  } else {
    // Fallback: display simple text if image not found
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("FLIPCARD", M5.Display.width() / 2, M5.Display.height() / 2 - 40);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Sleep Mode", M5.Display.width() / 2, M5.Display.height() / 2);
    M5.Display.drawString("Touch to wake up", M5.Display.width() / 2, M5.Display.height() / 2 + 40);
  }
  
  // Give 2 seconds for image to display before deep sleep
  delay(2000);
}

// Function to check for deep sleep timeout
void checkDeepSleep() {
  unsigned long currentTime = millis();
  if (currentTime - lastActivityTime > SLEEP_TIMEOUT) {
    Serial.println("Inactivity timeout reached - going to sleep");
    
    // Display lock screen image before deep sleep
    displayLockScreen();
    
    // Go to deep sleep
    M5.Power.deepSleep();
  }
}

// Function to go to random card in category
void goToRandomCard() {
  if (selectedCategory == "") {
    Serial.println("No category selected for random mode");
    return;
  }
  
  // Get random card from current category (excluding current card)
  String currentCardId = getCurrentCardId();
  int randomCardIndex = getRandomCardFromCategory(selectedCategory, currentCardId);
  
  // Update tracking
  lastRandomCardId = currentCardId;
  currentCardIndex = randomCardIndex;
  
  // Load the new card data
  if (loadCard(currentCardIndex)) {
    String folderPath = getCurrentCardFolder();
    resetToDefaultLanguage(); // Reset to default language when changing cards
    String currentLang = getCurrentLanguage();
    
    Serial.printf("Navigate to random card: %s\n", getCurrentCardId().c_str());
    
    // Redraw full flipcard using JSON data
    drawEmptyFrame();
    drawFlipcard(currentCardDoc, folderPath, currentLang);
  } else {
    Serial.println("Failed to load random card");
  }
}

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);

  M5.Display.setRotation(2); // Portrait mode
  M5.Display.clear();

  Serial.println("Starting simple image display...");
  
  // Initialize SD card with more robust error handling
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  delay(300); // Increase delay for SD card to stabilize
  
  int sdRetries = 0;
  while (!SD.begin(SD_SPI_CS_PIN, SPI, 10000000)) {
    Serial.println("SD card failed! Retrying...");
    M5.Display.println("SD card error! Retrying...");
    delay(500);
    sdRetries++;
    if (sdRetries >= 3) {
      Serial.println("SD card failed after retries!");
      M5.Display.println("SD card error! Please restart.");
      while(1) delay(100); // Stop execution
    }
  }
  
  Serial.println("SD card OK");
  
  // Add more delay and stabilization before loading files
  // delay(500);  
  // M5.Display.clear();
  // M5.Display.println("Initializing...");
  // delay(100);
  
  // Initialize last activity time
  lastActivityTime = millis();
  
  // Load config.json first
  if (!loadConfig()) {
    Serial.println("Failed to load config.json - using fallback");
    M5.Display.println("Config load error!");
    return;
  }
  
  // Load index.json
  if (!loadIndex()) {
    Serial.println("Failed to load index.json - using fallback");
    M5.Display.println("JSON load error!");
    return;
  }
  
  // Start with menu mode
  Serial.printf("Starting in menu mode with %d cards loaded\n", totalCards);
  goToMenuMode();
  M5.update();
}

void loop() {
  M5.update();
  
  // Check for touch input
  if (M5.Touch.getCount()) {
    auto t = M5.Touch.getDetail();
    if (t.wasPressed()) {
      // Reset activity timer on any touch
      lastActivityTime = millis();
      
      int touchX = t.x;
      int touchY = t.y;
      
      Serial.printf("Touch detected at (%d, %d) in %s mode\n", 
                    touchX, touchY, 
                    currentPageMode == MENU_MODE ? "MENU" :
                    (currentPageMode == CATEGORY_MODE ? "CATEGORY" : 
                    (currentPageMode == GRID_MODE ? "GRID" : 
                    (currentPageMode == FLIPCARD_MODE ? "FLIPCARD" :
                    (currentPageMode == OPTION_MODE ? "OPTION" : "LANGUAGE_SELECTION")))));
      
      if (currentPageMode == MENU_MODE) {
        // Handle menu page touch
        int buttonPressed = handleMenuTouch(touchX, touchY);
        if (buttonPressed == 1) {
          // Category button was pressed, go to normal category mode
          isRandomMode = false;
          goToCategoryMode();
        } else if (buttonPressed == 2) {
          // Random button was pressed, go to random category mode
          isRandomMode = true;
          lastRandomCardId = ""; // Reset random history
          goToCategoryMode();
        } else if (buttonPressed == 3) {
          // Option button was pressed, go to option mode
          goToOptionMode();
        }
        
      } else if (currentPageMode == CATEGORY_MODE) {
        // Handle category page touch
        // Check home button first
        if (isTouchOnCategoryHomeButton(touchX, touchY)) {
          Serial.println("Category: Home button touched - returning to menu");
          goToMenuMode();
        } else {
          // Check category selection
          String categoryId = getCategoryIdFromTouch(touchX, touchY, indexDoc);
          if (categoryId != "") {
            Serial.printf("Category: Selected category %s\n", categoryId.c_str());
            selectedCategory = categoryId;
            
            if (isRandomMode) {
              // Random mode: skip grid, go directly to random flipcard
              Serial.println("Random mode: going directly to flipcard");
              // Get first random card from category
              int randomCardIndex = getRandomCardFromCategory(selectedCategory, "");
              goToFlipcardMode(randomCardIndex);
              lastRandomCardId = getCurrentCardId();
            } else {
              // Normal mode: go to grid
              goToGridMode();
            }
          }
        }
        
      } else if (currentPageMode == GRID_MODE) {
        // Handle grid page touch
        String buttonType;
        if (isTouchOnGridNavButton(touchX, touchY, buttonType)) {
          if (buttonType == "left") {
            if (totalGridPages > 1) {
              Serial.println("Grid: Previous page");
              goToPreviousGridPage();
            } else {
              Serial.println("Grid: Left button pressed but only one page - no action");
            }
          } else if (buttonType == "right") {
            if (totalGridPages > 1) {
              Serial.println("Grid: Next page");
              goToNextGridPage();
            } else {
              Serial.println("Grid: Right button pressed but only one page - no action");
            }
          } else if (buttonType == "home") {
            Serial.println("Grid: Home button - back to category");
            selectedCategory = ""; // Clear category filter
            goToCategoryMode();
          }
        } else {
          // Check if touch is on a thumbnail
          int cardIndex;
          if (selectedCategory != "") {
            cardIndex = getTouchedThumbnailIndexFiltered(touchX, touchY, currentGridPage, indexDoc, selectedCategory);
          } else {
            cardIndex = getTouchedThumbnailIndex(touchX, touchY, currentGridPage);
          }
          
          if (cardIndex >= 0 && cardIndex < totalCards) {
            Serial.printf("Grid: Selected card %d\n", cardIndex);
            goToFlipcardMode(cardIndex);
          }
        }
        M5.update();
        
      } else if (currentPageMode == OPTION_MODE) {
        // Handle option page touch
        // Check home button first
        if (isTouchOnOptionHomeButton(touchX, touchY)) {
          Serial.println("Option: Home button touched - returning to menu");
          goToMenuMode();
        } else {
          // Check option buttons
          int buttonPressed = handleOptionTouch(touchX, touchY);
          if (buttonPressed == 1) {
            // Language button was pressed
            goToLanguageSelectionMode();
          } else if (buttonPressed == 2) {
            // Root Menu button was pressed (not implemented yet)
            Serial.println("Root Menu not implemented yet");
          }
        }
        
      } else if (currentPageMode == LANGUAGE_SELECTION_MODE) {
        // Handle language selection touch
        // Check home button first
        if (isTouchOnOptionHomeButton(touchX, touchY)) {
          Serial.println("Language Selection: Home button touched - returning to options");
          goToOptionMode();
        } else {
          // Check language selection
          String selectedLang = handleLanguageSelectionTouch(touchX, touchY, configDoc);
          if (selectedLang != "") {
            Serial.printf("Language Selected: %s\n", selectedLang.c_str());
            
            // Save new default language
            if (saveDefaultLanguage(selectedLang)) {
              Serial.println("Successfully saved new default language");
              
              // Reload config to update global state
              if (loadConfig()) {
                Serial.println("Config reloaded successfully");
                Serial.printf("New default language is now: %s (index %d)\n", defaultLanguage.c_str(), currentLanguageIndex);
              } else {
                Serial.println("Failed to reload config");
              }
              
              // Return to option page
              goToOptionMode();
            } else {
              Serial.println("Failed to save new default language");
            }
          }
        }
        
      } else { // FLIPCARD_MODE
        // Handle flipcard touch (existing logic)
        int screenWidth = M5.Display.width();
        int buttonSize = 80;
        int buttonMargin = 20;
        
        // Button positions
        int leftButtonX = buttonMargin + 25;
        int leftButtonY = buttonMargin + 25;
        int rightButtonX = screenWidth - buttonSize - buttonMargin - 25;
        int rightButtonY = buttonMargin + 25;
        int homeButtonX = (screenWidth - buttonSize) / 2;
        int homeButtonY = buttonMargin + 25;
        
        // Check navigation buttons
        bool touchOnLeftButton = (touchX >= leftButtonX && touchX <= leftButtonX + buttonSize && 
                                 touchY >= leftButtonY && touchY <= leftButtonY + buttonSize);
        bool touchOnRightButton = (touchX >= rightButtonX && touchX <= rightButtonX + buttonSize && 
                                  touchY >= rightButtonY && touchY <= rightButtonY + buttonSize);
        bool touchOnHomeButton = (touchX >= homeButtonX && touchX <= homeButtonX + buttonSize && 
                                 touchY >= homeButtonY && touchY <= homeButtonY + buttonSize);
        
        if (touchOnLeftButton) {
          if (isRandomMode) {
            Serial.println("Flipcard: Random previous card");
            goToRandomCard();
          } else {
            Serial.println("Flipcard: Previous card");
            goToPreviousCard();
          }
        } else if (touchOnRightButton) {
          if (isRandomMode) {
            Serial.println("Flipcard: Random next card");
            goToRandomCard();
          } else {
            Serial.println("Flipcard: Next card");
            goToNextCard();
          }
        } else if (touchOnHomeButton) {
          if (isRandomMode) {
            Serial.println("Flipcard: Home button - back to categories (random mode)");
            goToCategoryMode();
          } else {
            Serial.println("Flipcard: Home button - back to grid");
            goToGridMode();
          }
        } else {
          // Define center area bounds (big and small image areas)
          int bigWidth = 400, bigHeight = 150;
          int smallWidth = 400, smallHeight = 80;
          
          int bigX = (screenWidth - bigWidth) / 2;
          int smallX = (screenWidth - smallWidth) / 2;
          int bigY = 180;
          int smallY = bigY + bigHeight + 5;
          
          // Check if touch is in the center area (big or small image)
          bool touchInCenter = (touchX >= bigX && touchX <= bigX + bigWidth && 
                               touchY >= bigY && touchY <= smallY + smallHeight);
          
          if (touchInCenter) {
            Serial.printf("Touch detected in center area at (%d, %d)\n", touchX, touchY);
            // Cycle to next language
            cycleToNextLanguage();
            
            // Refresh only the language images using JSON data (more efficient)
            String folderPath = getCurrentCardFolder();
            String currentLang = getCurrentLanguage();
            refreshLanguageImages(currentCardDoc, folderPath, currentLang);
          }
        }
        M5.update();
      }
    }
  }
  
  // Check for sleep timeout
  checkDeepSleep();
  
  delay(50);
}