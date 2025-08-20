#include "empty_frame_page.h"
#include <M5Unified.h>
#include <SD.h>

// Helper function to load PNG using file handle (same as flipcard_page)
bool loadPngFromFile_EmptyFrame(const char* filename, int x, int y, int width, int height, float scale_x = 1.0f, float scale_y = 1.0f) {
  File file = SD.open(filename);
  if (!file) {
    return false;
  }
  
  bool result = M5.Display.drawPng(&file, x, y, width, height, 0, 0, scale_x, scale_y);
  file.close();
  
  return result;
}

// Function to display the empty frame image
void drawEmptyFrame() {
  String imageFile = "/flipcard/empty-frame.png";
  
  // Calculate scale to fit screen (portrait image: 540x960)
  float scale_x = (float)M5.Display.width() / 540.0f;
  float scale_y = (float)M5.Display.height() / 960.0f;
  float scale = max(scale_x, scale_y);
  
  Serial.printf("Drawing empty frame: %s\n", imageFile.c_str());
  Serial.printf("Screen: %dx%d, Scale: %f\n", M5.Display.width(), M5.Display.height(), scale);
  
  if (!loadPngFromFile_EmptyFrame(imageFile.c_str(), 0, 0, 
                                 M5.Display.width(), M5.Display.height(), 
                                 scale, scale)) {
    Serial.println("Failed to load empty frame");
    M5.Display.println("Empty frame not found");
  } else {
    Serial.println("Empty frame displayed successfully");
  }
}