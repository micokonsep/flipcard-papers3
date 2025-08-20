#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

// Function declarations for grid thumbnail page
void drawGridPage(JsonDocument& indexData, int gridPage, int totalGridPages);
void drawGridPageFiltered(JsonDocument& indexData, int gridPage, int totalGridPages, String categoryFilter);
int getTouchedThumbnailIndex(int touchX, int touchY, int gridPage);
int getTouchedThumbnailIndexFiltered(int touchX, int touchY, int gridPage, JsonDocument& indexData, String categoryFilter);
bool isTouchOnGridNavButton(int touchX, int touchY, String& buttonType);

// Helper functions for filtering
int getFilteredCardCount(JsonDocument& indexData, String categoryFilter);
int getFilteredCardGlobalIndex(JsonDocument& indexData, String categoryFilter, int filteredIndex);

// Helper function
bool loadThumbnailFromCard(const char* folderPath, const char* thumbnailFile, int x, int y, int size);