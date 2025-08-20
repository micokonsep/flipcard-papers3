#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

// Function declarations for JSON-driven flipcard display
void drawFlipcard(JsonDocument& cardData, String folderPath, String currentLanguage);
void refreshLanguageImages(JsonDocument& cardData, String folderPath, String currentLanguage);

// Navigation function
void drawNavigationButtons();

// Helper function
bool loadPngFromFile(const char* filename, int x, int y, int width, int height);