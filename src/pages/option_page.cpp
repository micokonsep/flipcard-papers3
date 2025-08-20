#include "option_page.h"
#include <M5Unified.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <vector>

// Option page button coordinates
int languageBtnX = 70;      // Language button
int languageBtnY = 300;     
int languageBtnW = 400;     
int languageBtnH = 100;     

     

// Home button coordinates (same as other pages)
int optionHomeBtnX = 230;   // Centered
int optionHomeBtnY = 35;    
int optionHomeBtnSize = 80;

// Language selection state
struct LanguageInfo {
    String key;          // "chinese", "english"
    String displayName;  // "中文 (Chinese)"
    bool enabled;        
    int x, y, width, height;  // Touch area
};
static std::vector<LanguageInfo> availableLanguages;

void drawOptionPage() {
    auto& display = M5.Display;
    display.clear();
    
    // Draw home button
    File homeFile = SD.open("/flipcard/Home.png");
    if (homeFile) {
        display.drawPng(&homeFile, optionHomeBtnX, optionHomeBtnY, optionHomeBtnSize, optionHomeBtnSize);
        homeFile.close();
    } else {
        // Fallback home button
        display.fillRoundRect(optionHomeBtnX, optionHomeBtnY, optionHomeBtnSize, optionHomeBtnSize, 8, TFT_BLUE);
        display.setTextColor(TFT_WHITE);
        display.setTextSize(2);
        display.drawString("H", optionHomeBtnX + 30, optionHomeBtnY + 30);
    }
    
    // Header
    display.setFont(&fonts::efontCN_16);
    display.setTextSize(3);
    display.setTextColor(TFT_BLACK);
    display.drawString("Options", 20, 120);
    
    // Language button
    display.fillRect(languageBtnX, languageBtnY, languageBtnW, languageBtnH, TFT_WHITE);
    display.drawRect(languageBtnX, languageBtnY, languageBtnW, languageBtnH, TFT_BLACK);
    display.setTextColor(TFT_BLACK);
    display.setTextSize(2);
    display.drawString("Language Settings", languageBtnX + 80, languageBtnY + 40);
    
}

int handleOptionTouch(int x, int y) {
    Serial.printf("Option touch: x=%d, y=%d\n", x, y);
    
    // Check Language button
    if (x >= languageBtnX && x <= languageBtnX + languageBtnW &&
        y >= languageBtnY && y <= languageBtnY + languageBtnH) {
        Serial.println("Language button touched!");
        return 1; // Language settings
    }
    
    
    return 0; // No button touched
}

bool isTouchOnOptionHomeButton(int x, int y) {
    return (x >= optionHomeBtnX && x <= optionHomeBtnX + optionHomeBtnSize &&
            y >= optionHomeBtnY && y <= optionHomeBtnY + optionHomeBtnSize);
}

void drawLanguageSelectionPage(JsonDocument& configDoc) {
    auto& display = M5.Display;
    display.clear();
    
    // Clear previous languages
    availableLanguages.clear();
    
    // Draw home button
    File homeFile = SD.open("/flipcard/Home.png");
    if (homeFile) {
        display.drawPng(&homeFile, optionHomeBtnX, optionHomeBtnY, optionHomeBtnSize, optionHomeBtnSize);
        homeFile.close();
    } else {
        // Fallback home button
        display.fillRoundRect(optionHomeBtnX, optionHomeBtnY, optionHomeBtnSize, optionHomeBtnSize, 8, TFT_BLUE);
        display.setTextColor(TFT_WHITE);
        display.setTextSize(2);
        display.drawString("H", optionHomeBtnX + 30, optionHomeBtnY + 30);
    }
    
    // Header
    display.setFont(&fonts::efontCN_16);
    display.setTextSize(2);
    display.setTextColor(TFT_BLACK);
    display.drawString("Select Default Language", 20, 120);
    
    // Get current default language
    String currentDefault = configDoc["languages"]["default"].as<String>();
    Serial.printf("Current default language: %s\n", currentDefault.c_str());
    
    // List supported languages
    JsonObject supportedLangs = configDoc["languages"]["supported"];
    int yPos = 180;
    int index = 0;
    
    for (JsonPair langPair : supportedLangs) {
        String langKey = langPair.key().c_str();
        JsonObject langInfo = langPair.value();
        
        // Only show enabled languages
        if (langInfo["enabled"].as<bool>()) {
            LanguageInfo info;
            info.key = langKey;
            
            // Create display name: "Chinese"
            String englishName = langInfo["english_name"].as<String>();
            info.displayName = englishName;
            info.enabled = true;
            
            // Position
            info.x = 50;
            info.y = yPos;
            info.width = 440;
            info.height = 80;
            
            // Draw language option (all with white background)
            bool isDefault = (langKey == currentDefault);
            
            display.fillRect(info.x, info.y, info.width, info.height, TFT_WHITE);
            display.drawRect(info.x, info.y, info.width, info.height, TFT_BLACK);
            
            display.setTextColor(TFT_BLACK);
            display.setTextSize(2);
            
            // Add asterisk (*) for default language
            if (isDefault) {
                String displayText = "* " + info.displayName;
                display.drawString(displayText.c_str(), info.x + 20, info.y + 30);
            } else {
                display.drawString(info.displayName.c_str(), info.x + 20, info.y + 30);
            }
            
            availableLanguages.push_back(info);
            yPos += 100;
            index++;
        }
    }
    
    Serial.printf("Displayed %d enabled languages\n", availableLanguages.size());
}

String handleLanguageSelectionTouch(int x, int y, JsonDocument& configDoc) {
    Serial.printf("Language selection touch: x=%d, y=%d\n", x, y);
    
    // Check each language option
    for (const auto& lang : availableLanguages) {
        if (x >= lang.x && x <= lang.x + lang.width &&
            y >= lang.y && y <= lang.y + lang.height) {
            Serial.printf("Selected language: %s\n", lang.key.c_str());
            return lang.key;
        }
    }
    
    return ""; // No language selected
}

bool saveDefaultLanguage(String newDefaultLang) {
    Serial.printf("Attempting to save new default language: %s\n", newDefaultLang.c_str());
    
    // Read current config
    File file = SD.open("/flipcard/config.json", FILE_READ);
    if (!file) {
        Serial.println("Failed to open config.json for reading");
        return false;
    }
    
    JsonDocument configDoc;
    DeserializationError error = deserializeJson(configDoc, file);
    file.close();
    
    if (error) {
        Serial.printf("Failed to parse config.json: %s\n", error.c_str());
        return false;
    }
    
    // Update default language
    configDoc["languages"]["default"] = newDefaultLang;
    Serial.printf("Updated default language to: %s\n", newDefaultLang.c_str());
    
    // Write back to file
    file = SD.open("/flipcard/config.json", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open config.json for writing");
        return false;
    }
    
    if (serializeJson(configDoc, file) == 0) {
        Serial.println("Failed to write config.json");
        file.close();
        return false;
    }
    
    file.close();
    Serial.println("Successfully saved new default language to config.json");
    return true;
}