#pragma once
#include <ArduinoJson.h>
#include <M5Unified.h>

struct CategoryInfo {
    String id;           // "transport", "technology"
    String name;         // "Transportation", "Technology"
    int count;           // Number of cards in category
    int x, y, width, height;  // Touch area
};

void drawCategoryPage(JsonDocument& indexDoc);
void drawCategoryPage(JsonDocument& indexDoc, bool isRandomMode);
bool isTouchOnCategory(int x, int y, String& selectedCategoryId);
String getCategoryIdFromTouch(int x, int y, JsonDocument& indexDoc);
bool isTouchOnCategoryHomeButton(int x, int y);