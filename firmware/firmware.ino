/*
   Created by DIYables

   This example code is in the public domain

   Product page:
   - https://diyables.io/products/3.5-tft-lcd-color-touch-screen-shield-for-arduino-uno-mega-320x480-resolution-ili9488-driver-parallel-8-bit-interface-28pin-module-with-touch
   - https://www.amazon.com/dp/B0DQ3NQ3LW
*/

#include <DIYables_TFT_Touch_Shield.h>
#include "fonts.h"
#include <string.h>
#include <math.h>

#define MAGENTA   DIYables_TFT::colorRGB(255, 0, 255)
#define WHITE     DIYables_TFT::colorRGB(255, 255, 255)
#define BLACK     DIYables_TFT::colorRGB(0, 0, 0)

DIYables_TFT_ILI9488_Shield TFT_display;

// Current font settings
sFONT* current_font = &Font8;  // Default font (can change to Font8, Font16, Font20, or Font8)
uint16_t text_color = WHITE;
int cursor_x = 0;
int cursor_y = 0;

// Breathing circle animation variables
int circle_center_x = 0;
int circle_center_y = 0;
int prev_radius = 0;
unsigned long last_animation_time = 0;
const unsigned long animation_interval = 20;  // Update every 20ms for smooth animation
const int min_radius = 20;
const int max_radius = 50;
const float breathing_speed = 0.002;  // Speed of breathing animation

// Function to draw a single character using sFONT format
void drawChar(int x, int y, char c, sFONT* font, uint16_t color) {
  if (c < 32 || c > 126) return;  // Only printable ASCII
  
  // Calculate bytes per row (rounded up to nearest byte)
  uint16_t bytes_per_row = (font->Width + 7) / 8;
  if (bytes_per_row == 0) bytes_per_row = 1;
  
  // Calculate character index (starting from space = 32)
  uint8_t char_index = c - 32;
  
  // Calculate offset in font table for this character (in bytes)
  uint32_t char_offset = char_index * font->Height * bytes_per_row;
  
  // Draw the character
  for (uint16_t row = 0; row < font->Height; row++) {
    uint32_t row_offset = char_offset + row * bytes_per_row;
    
    for (uint16_t col = 0; col < font->Width; col++) {
      bool pixel_set = false;
      
      // Calculate which byte and bit within that byte
      uint16_t byte_index = col / 8;  // Which byte in the row (0, 1, 2, ...)
      uint8_t bit_in_byte = 7 - (col % 8);  // Bit position within byte (MSB first)
      
      // Read the byte containing this pixel
      uint8_t font_byte = pgm_read_byte(&font->table[row_offset + byte_index]);
      
      // Check if the bit is set
      pixel_set = (font_byte & (1 << bit_in_byte)) != 0;
      
      if (pixel_set) {
        TFT_display.drawPixel(x + col, y + row, color);
      }
    }
  }
}

// Function to draw a string
void drawString(int x, int y, const char* str, sFONT* font, uint16_t color) {
  int current_x = x;
  while (*str) {
    drawChar(current_x, y, *str, font, color);
    current_x += font->Width;
    str++;
  }
}

// Wrapper functions to match standard TFT library interface
void setCursor(int x, int y) {
  cursor_x = x;
  cursor_y = y;
}

void setTextColor(uint16_t color) {
  text_color = color;
}

void setFont(sFONT* font) {
  current_font = font;
}

// Print functions
void printChar(char c) {
  drawChar(cursor_x, cursor_y, c, current_font, text_color);
  cursor_x += current_font->Width;
}

void printString(const char* str) {
  drawString(cursor_x, cursor_y, str, current_font, text_color);
  cursor_x += strlen(str) * current_font->Width;
}

void printFloat(float value, int decimals) {
  char buffer[20];
  dtostrf(value, 0, decimals, buffer);
  printString(buffer);
}

void println() {
  cursor_x = 0;
  cursor_y += current_font->Height;
}

void setup() {

  Serial.println(F("Arduino TFT Touch LCD Display - show text and float number"));

  TFT_display.begin();

  // Set the rotation (0 to 3)
  TFT_display.setRotation(3);  // Rotate screen 90 degrees
  TFT_display.fillScreen(WHITE);

  // Set text color and custom font (can use Font8, Font16, Font20, or Font8)
  setTextColor(BLACK);
  setFont(&Font8);

  // Sample temperature value
  float temperature = 23.5;
  float humidity = 78.6;

  // Display temperature with degree symbol
  setCursor(20, 20);    // Set cursor position (x, y)
  printString("Temperature: ");
  printFloat(temperature, 1);  // Print temperature with 1 decimal place
  printChar(char(247));  // Degree symbol
  printString("C");
  println();

  // Display humidity
  setCursor(20, 60);    // Set cursor position (x, y)
  printString("Humidity: ");
  printFloat(humidity, 1);   // Print humidity with 1 decimal place
  printString("%");

  // Calculate center of screen for breathing circle
  circle_center_x = TFT_display.width() / 2;
  circle_center_y = TFT_display.height() / 2;
  
  last_animation_time = millis();
}

// Function to update breathing circle by only changing pixels that need to change
void updateBreathingCircle(int current_radius) {
  // Calculate bounding box around the circle (only need to check pixels within max_radius)
  int min_x = circle_center_x - max_radius;
  int max_x = circle_center_x + max_radius;
  int min_y = circle_center_y - max_radius;
  int max_y = circle_center_y + max_radius;
  
  // Clamp to screen bounds
  min_x = (min_x < 0) ? 0 : min_x;
  max_x = (max_x > TFT_display.width() - 1) ? TFT_display.width() - 1 : max_x;
  min_y = (min_y < 0) ? 0 : min_y;
  max_y = (max_y > TFT_display.height() - 1) ? TFT_display.height() - 1 : max_y;
  
  // Iterate through pixels in bounding box
  for (int y = min_y; y <= max_y; y++) {
    for (int x = min_x; x <= max_x; x++) {
      // Calculate distance from center (using integer math for speed)
      int dx = x - circle_center_x;
      int dy = y - circle_center_y;
      // Use squared distance to avoid sqrt calculation
      int dist_squared = dx * dx + dy * dy;
      int current_radius_squared = current_radius * current_radius;
      int prev_radius_squared = prev_radius * prev_radius;
      
      // Determine if pixel should be on or off based on current radius
      bool should_be_on = (dist_squared <= current_radius_squared);
      bool was_on = (prev_radius > 0 && dist_squared <= prev_radius_squared);
      
      // Only update pixels that changed state
      if (should_be_on != was_on) {
        TFT_display.drawPixel(x, y, should_be_on ? BLACK : WHITE);
      }
    }
  }
}

void loop(void) {
  // Animate the breathing circle
  unsigned long current_time = millis();
  
  if (current_time - last_animation_time >= animation_interval) {
    // Calculate breathing radius using sine wave for smooth animation
    float time_factor = current_time * breathing_speed;
    float sine_value = sin(time_factor);
    
    // Map sine wave (-1 to 1) to radius range (min_radius to max_radius)
    int current_radius = min_radius + (int)((sine_value + 1.0) * 0.5 * (max_radius - min_radius));
    
    // Update circle by only changing pixels that need to change
    updateBreathingCircle(current_radius);
    
    prev_radius = current_radius;
    last_animation_time = current_time;
  }
}
