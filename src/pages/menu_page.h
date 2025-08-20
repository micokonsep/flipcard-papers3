#pragma once

// Draw menu page with three buttons
void drawMenuPage();

// Handle touch input for menu page
// Returns: 1 = category, 2 = random, 3 = option, 0 = no button
int handleMenuTouch(int x, int y);