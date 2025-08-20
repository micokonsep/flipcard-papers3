#include "category_page.h"
#include <M5Unified.h>
#include <SD.h>
#include <vector>

// Layout constants
const int CATEGORY_ITEM_HEIGHT = 80;
const int CATEGORY_PADDING = 20;
const int CATEGORY_START_Y = 200;  // Moved down to make room for home icon
const int NAV_BUTTON_SIZE = 80;
const int NAV_BUTTON_MARGIN = 20;

static std::vector<CategoryInfo> categories;

void drawCategoryPage(JsonDocument& indexDoc) {
    drawCategoryPage(indexDoc, false);
}

void drawCategoryPage(JsonDocument& indexDoc, bool isRandomMode) {
    auto& display = M5.Display;
    display.clear();
    
    // Setup categories from JSON
    categories.clear();
    
    // Count number of cards per category
    JsonArray cards = indexDoc["cards"];
    JsonObject categoriesObj = indexDoc["categories"];
    
    // Loop through categories in JSON
    for (JsonPair categoryPair : categoriesObj) {
        String categoryId = categoryPair.key().c_str();
        String categoryName = categoryPair.value()["name"].as<String>();
        
        // Count number of cards for this category
        int count = 0;
        for (JsonObject card : cards) {
            if (card["category"].as<String>() == categoryId) {
                count++;
            }
        }
        
        // Add to categories list
        CategoryInfo info;
        info.id = categoryId;
        info.name = categoryName;
        info.count = count;
        
        // Calculate position
        int index = categories.size();
        info.x = CATEGORY_PADDING;
        info.y = CATEGORY_START_Y + (index * (CATEGORY_ITEM_HEIGHT + CATEGORY_PADDING));
        info.width = display.width() - (2 * CATEGORY_PADDING);
        info.height = CATEGORY_ITEM_HEIGHT;
        
        categories.push_back(info);
    }
    
    // Draw home icon at same position as grid page
    int screenWidth = display.width();
    int homeButtonX = (screenWidth - NAV_BUTTON_SIZE) / 2;  // Centered horizontally
    int homeButtonY = NAV_BUTTON_MARGIN + 15;  // Same Y as grid page buttons
    
    // Load home button PNG
    File homeFile = SD.open("/flipcard/Home.png");
    if (homeFile) {
        M5.Display.drawPng(&homeFile, homeButtonX, homeButtonY, NAV_BUTTON_SIZE, NAV_BUTTON_SIZE);
        homeFile.close();
    } else {
        // Fallback: draw simple home icon
        display.fillRoundRect(homeButtonX, homeButtonY, NAV_BUTTON_SIZE, NAV_BUTTON_SIZE, 8, TFT_DARKGREY);
        display.setTextColor(TFT_WHITE);
        display.setTextSize(3);
        display.setCursor(homeButtonX + 20, homeButtonY + 20);
        display.print("H");
    }
    
    // Header (moved down slightly)
    display.setFont(&fonts::efontCN_16);
    display.setTextSize(2);
    display.setTextColor(TFT_BLACK);
    if (isRandomMode) {
        display.drawString("Categories (Random Mode)", 20, 120);
    } else {
        display.drawString("Categories", 20, 120);
    }
    
    // Draw categories
    for (const auto& category : categories) {
        // Background box
        display.fillRoundRect(category.x, category.y, category.width, category.height, 10, TFT_LIGHTGREY);
        
        // Category name and count
        display.setTextColor(TFT_BLACK);
        display.setTextSize(2);
        String text = category.name + " (" + String(category.count) + ")";
        display.drawString(text.c_str(), category.x + 20, category.y + (category.height/2) - 15);
    }
    
    // No back button needed for category page
    
    display.display();
}

bool isTouchOnCategory(int x, int y, String& selectedCategoryId) {
    // Check categories
    for (const auto& category : categories) {
        if (x >= category.x && x <= category.x + category.width &&
            y >= category.y && y <= category.y + category.height) {
            selectedCategoryId = category.id;
            return true;
        }
    }
    
    return false;
}

String getCategoryIdFromTouch(int x, int y, JsonDocument& indexDoc) {
    String categoryId;
    if (isTouchOnCategory(x, y, categoryId)) {
        return categoryId;
    }
    return "";
}

bool isTouchOnCategoryHomeButton(int x, int y) {
    int screenWidth = 540;  // M5.Display.width()
    int homeButtonX = (screenWidth - NAV_BUTTON_SIZE) / 2;  // Centered horizontally
    int homeButtonY = NAV_BUTTON_MARGIN + 15;  // Same Y as grid page buttons
    
    return (x >= homeButtonX && x <= homeButtonX + NAV_BUTTON_SIZE &&
            y >= homeButtonY && y <= homeButtonY + NAV_BUTTON_SIZE);
}