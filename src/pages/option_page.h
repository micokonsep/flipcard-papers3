#pragma once
#include <ArduinoJson.h>

// Draw option page with two buttons: Language and Root Menu
void drawOptionPage();

// Handle touch input for option page
// Returns: 1 = language, 2 = root menu, 0 = no button
int handleOptionTouch(int x, int y);

// Draw language selection page
void drawLanguageSelectionPage(JsonDocument& configDoc);

// Handle touch input for language selection
// Returns language key if selected, empty string if home button
String handleLanguageSelectionTouch(int x, int y, JsonDocument& configDoc);

// Check if touch is on option home button
bool isTouchOnOptionHomeButton(int x, int y);

// Save new default language to config.json
bool saveDefaultLanguage(String newDefaultLang);