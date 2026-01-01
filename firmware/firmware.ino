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
const unsigned long animation_interval = 50;  // Update every 50ms for smooth animation
const int min_radius = 20;
const int max_radius = 50;
const float breathing_speed = 0.002;  // Speed of breathing animation

// Optimized animation state tracking
char prev_text[32] = "";  // Store previous text string
float prev_time_param = 0.0;  // Store previous time parameter
int prev_text_center_x = 0;
int prev_text_center_y = 0;
int prev_text_width = 0;
int prev_text_height = 0;

// Breathing circle font variables
const int BREATHING_FONT_MAX_RADIUS = 4;  // Maximum radius for breathing circles (controls spacing)
const float NOISE_DENSITY = 0.03;  // Density of noise sampling
const float NOISE_SPEED = 0.03;    // Speed of noise animation
const float NOISE_VARIANCE = 2.0;  // Variance in radius from noise

// Permutation table for simplified Perlin noise (from standard Perlin noise)
const uint8_t PERMUTATION[512] PROGMEM = {
  151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
  129,22,39,253,9,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
  49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  // Duplicate for wrapping
  151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
  129,22,39,253,9,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
  49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// Gradient vectors for 3D noise
const int8_t GRAD3[12][3] PROGMEM = {
  {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
  {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
  {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

// Simplified 3D Perlin noise function (returns value from -1 to 1)
float noise3(float x, float y, float z) {
  // Simple hash-based approach for performance on Arduino
  // This is a simplified version that approximates Perlin noise
  
  // Get integer coordinates
  int X = (int)floor(x) & 255;
  int Y = (int)floor(y) & 255;
  int Z = (int)floor(z) & 255;
  
  // Get fractional parts
  x -= floor(x);
  y -= floor(y);
  z -= floor(z);
  
  // Fade curves (smoothstep)
  float u = x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
  float v = y * y * y * (y * (y * 6.0 - 15.0) + 10.0);
  float w = z * z * z * (z * (z * 6.0 - 15.0) + 10.0);
  
  // Hash coordinates of the 8 cube corners
  int A = pgm_read_byte(&PERMUTATION[X]) + Y;
  int AA = pgm_read_byte(&PERMUTATION[A]) + Z;
  int AB = pgm_read_byte(&PERMUTATION[A + 1]) + Z;
  int B = pgm_read_byte(&PERMUTATION[X + 1]) + Y;
  int BA = pgm_read_byte(&PERMUTATION[B]) + Z;
  int BB = pgm_read_byte(&PERMUTATION[B + 1]) + Z;
  
  // Get gradients
  int g0 = pgm_read_byte(&PERMUTATION[AA]) % 12;
  int g1 = pgm_read_byte(&PERMUTATION[BA]) % 12;
  int g2 = pgm_read_byte(&PERMUTATION[AB]) % 12;
  int g3 = pgm_read_byte(&PERMUTATION[BB]) % 12;
  int g4 = pgm_read_byte(&PERMUTATION[AA + 1]) % 12;
  int g5 = pgm_read_byte(&PERMUTATION[BA + 1]) % 12;
  int g6 = pgm_read_byte(&PERMUTATION[AB + 1]) % 12;
  int g7 = pgm_read_byte(&PERMUTATION[BB + 1]) % 12;
  
  // Read gradient vectors
  int8_t g0x = pgm_read_byte(&GRAD3[g0][0]);
  int8_t g0y = pgm_read_byte(&GRAD3[g0][1]);
  int8_t g0z = pgm_read_byte(&GRAD3[g0][2]);
  int8_t g1x = pgm_read_byte(&GRAD3[g1][0]);
  int8_t g1y = pgm_read_byte(&GRAD3[g1][1]);
  int8_t g1z = pgm_read_byte(&GRAD3[g1][2]);
  int8_t g2x = pgm_read_byte(&GRAD3[g2][0]);
  int8_t g2y = pgm_read_byte(&GRAD3[g2][1]);
  int8_t g2z = pgm_read_byte(&GRAD3[g2][2]);
  int8_t g3x = pgm_read_byte(&GRAD3[g3][0]);
  int8_t g3y = pgm_read_byte(&GRAD3[g3][1]);
  int8_t g3z = pgm_read_byte(&GRAD3[g3][2]);
  int8_t g4x = pgm_read_byte(&GRAD3[g4][0]);
  int8_t g4y = pgm_read_byte(&GRAD3[g4][1]);
  int8_t g4z = pgm_read_byte(&GRAD3[g4][2]);
  int8_t g5x = pgm_read_byte(&GRAD3[g5][0]);
  int8_t g5y = pgm_read_byte(&GRAD3[g5][1]);
  int8_t g5z = pgm_read_byte(&GRAD3[g5][2]);
  int8_t g6x = pgm_read_byte(&GRAD3[g6][0]);
  int8_t g6y = pgm_read_byte(&GRAD3[g6][1]);
  int8_t g6z = pgm_read_byte(&GRAD3[g6][2]);
  int8_t g7x = pgm_read_byte(&GRAD3[g7][0]);
  int8_t g7y = pgm_read_byte(&GRAD3[g7][1]);
  int8_t g7z = pgm_read_byte(&GRAD3[g7][2]);
  
  // Calculate dot products
  float n0 = g0x * x + g0y * y + g0z * z;
  float n1 = g1x * (x - 1.0) + g1y * y + g1z * z;
  float n2 = g2x * x + g2y * (y - 1.0) + g2z * z;
  float n3 = g3x * (x - 1.0) + g3y * (y - 1.0) + g3z * z;
  float n4 = g4x * x + g4y * y + g4z * (z - 1.0);
  float n5 = g5x * (x - 1.0) + g5y * y + g5z * (z - 1.0);
  float n6 = g6x * x + g6y * (y - 1.0) + g6z * (z - 1.0);
  float n7 = g7x * (x - 1.0) + g7y * (y - 1.0) + g7z * (z - 1.0);
  
  // Interpolate
  float n01 = n0 + u * (n1 - n0);
  float n23 = n2 + u * (n3 - n2);
  float n45 = n4 + u * (n5 - n4);
  float n67 = n6 + u * (n7 - n6);
  
  float n0123 = n01 + v * (n23 - n01);
  float n4567 = n45 + v * (n67 - n45);
  
  return n0123 + w * (n4567 - n0123);
}

// Function to draw a filled circle (optimized)
void drawFilledCircle(int center_x, int center_y, int radius, uint16_t color) {
  if (radius <= 0) return;
  
  int radius_squared = radius * radius;
  
  // Draw circle using optimized algorithm
  for (int y = -radius; y <= radius; y++) {
    int y_squared = y * y;
    for (int x = -radius; x <= radius; x++) {
      // Check if point is inside circle
      if (x * x + y_squared <= radius_squared) {
        int px = center_x + x;
        int py = center_y + y;
        // Check bounds
        if (px >= 0 && px < TFT_display.width() && py >= 0 && py < TFT_display.height()) {
          TFT_display.drawPixel(px, py, color);
        }
      }
    }
  }
}

// Function to draw a single character using breathing circles (based on Font16)
void drawCharBreathing(int x, int y, char c, sFONT* font, uint16_t color, float time_param) {
  if (c < 32 || c > 126) return;  // Only printable ASCII
  
  // Calculate bytes per row (rounded up to nearest byte)
  uint16_t bytes_per_row = (font->Width + 7) / 8;
  if (bytes_per_row == 0) bytes_per_row = 1;
  
  // Calculate character index (starting from space = 32)
  uint8_t char_index = c - 32;
  
  // Calculate offset in font table for this character (in bytes)
  uint32_t char_offset = char_index * font->Height * bytes_per_row;
  
  // Draw the character with breathing circles
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
        // Calculate center position for this pixel, scaled by max radius spacing
        // Adjacent pixels should be 2 * BREATHING_FONT_MAX_RADIUS apart
        int spacing = 2 * BREATHING_FONT_MAX_RADIUS;
        int circle_x = x + col * spacing;
        int circle_y = y + row * spacing;
        
        // Calculate noise value for this position
        float noise_x = col * NOISE_DENSITY;
        float noise_y = row * NOISE_DENSITY;
        float noise_z = time_param * NOISE_SPEED;
        float noise_val = noise3(noise_x, noise_y, noise_z);
        
        // Calculate radius: base radius + noise variation
        float base_radius = BREATHING_FONT_MAX_RADIUS / 2.0;
        float radius = base_radius + noise_val * NOISE_VARIANCE;
        
        // Clamp radius to valid range
        if (radius < 1) radius = 1;
        if (radius > BREATHING_FONT_MAX_RADIUS) radius = BREATHING_FONT_MAX_RADIUS;
        
        // Draw the breathing circle
        drawFilledCircle(circle_x, circle_y, (int)radius, color);
      }
    }
  }
}

// Function to draw a string with breathing circles
void drawStringBreathing(int x, int y, const char* str, sFONT* font, uint16_t color, float time_param) {
  int current_x = x;
  // Scale character spacing by max radius (2 * BREATHING_FONT_MAX_RADIUS per font pixel)
  int spacing = 2 * BREATHING_FONT_MAX_RADIUS;
  int char_width_scaled = font->Width * spacing;
  while (*str) {
    drawCharBreathing(current_x, y, *str, font, color, time_param);
    current_x += char_width_scaled;
    str++;
  }
}

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

  Serial.println(F("Arduino TFT Touch LCD Display - Breathing Circle Font"));

  TFT_display.begin();

  // Set the rotation (0 to 3)
  TFT_display.setRotation(3);  // Rotate screen 90 degrees
  TFT_display.fillScreen(WHITE);

  // Initialize previous text state
  prev_text[0] = '\0';
  prev_time_param = 0.0;
  prev_text_center_x = 0;
  prev_text_center_y = 0;
  prev_text_width = 0;
  prev_text_height = 0;
  
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

// Optimized function to update a single breathing circle (only changes pixels that need to change)
void updateSingleBreathingCircle(int circle_x, int circle_y, int prev_radius_val, int current_radius_val, uint16_t color) {
  // Calculate bounding box around both circles (union of previous and current)
  int max_radius_check = (prev_radius_val > current_radius_val) ? prev_radius_val : current_radius_val;
  int min_x = circle_x - max_radius_check;
  int max_x = circle_x + max_radius_check;
  int min_y = circle_y - max_radius_check;
  int max_y = circle_y + max_radius_check;
  
  // Clamp to screen bounds
  min_x = (min_x < 0) ? 0 : min_x;
  max_x = (max_x > TFT_display.width() - 1) ? TFT_display.width() - 1 : max_x;
  min_y = (min_y < 0) ? 0 : min_y;
  max_y = (max_y > TFT_display.height() - 1) ? TFT_display.height() - 1 : max_y;
  
  int prev_radius_squared = prev_radius_val * prev_radius_val;
  int current_radius_squared = current_radius_val * current_radius_val;
  
  // Iterate through pixels in bounding box
  for (int y = min_y; y <= max_y; y++) {
    for (int x = min_x; x <= max_x; x++) {
      // Calculate distance from center
      int dx = x - circle_x;
      int dy = y - circle_y;
      int dist_squared = dx * dx + dy * dy;
      
      // Determine if pixel should be on or off based on current radius
      bool should_be_on = (dist_squared <= current_radius_squared);
      bool was_on = (prev_radius_val > 0 && dist_squared <= prev_radius_squared);
      
      // Only update pixels that changed state
      if (should_be_on != was_on) {
        TFT_display.drawPixel(x, y, should_be_on ? color : WHITE);
      }
    }
  }
}

// Optimized function to update breathing circle text (only updates changed pixels)
void updateBreathingText(int x, int y, const char* str, sFONT* font, uint16_t color, float prev_time, float current_time) {
  int spacing = 2 * BREATHING_FONT_MAX_RADIUS;
  int current_char_x = x;
  
  // Calculate bytes per row
  uint16_t bytes_per_row = (font->Width + 7) / 8;
  if (bytes_per_row == 0) bytes_per_row = 1;
  
  // Process each character
  while (*str) {
    char c = *str;
    if (c >= 32 && c <= 126) {
      uint8_t char_index = c - 32;
      uint32_t char_offset = char_index * font->Height * bytes_per_row;
      
      // Process each pixel in the character
      for (uint16_t row = 0; row < font->Height; row++) {
        uint32_t row_offset = char_offset + row * bytes_per_row;
        
        for (uint16_t col = 0; col < font->Width; col++) {
          // Check if this pixel should have a circle
          uint16_t byte_index = col / 8;
          uint8_t bit_in_byte = 7 - (col % 8);
          uint8_t font_byte = pgm_read_byte(&font->table[row_offset + byte_index]);
          bool pixel_set = (font_byte & (1 << bit_in_byte)) != 0;
          
          if (pixel_set) {
            // Calculate circle position
            int circle_x = current_char_x + col * spacing;
            int circle_y = y + row * spacing;
            
            // Calculate noise values for current and previous time
            float noise_x = col * NOISE_DENSITY;
            float noise_y = row * NOISE_DENSITY;
            float noise_z_prev = prev_time * NOISE_SPEED;
            float noise_z_curr = current_time * NOISE_SPEED;
            
            float noise_val_prev = noise3(noise_x, noise_y, noise_z_prev);
            float noise_val_curr = noise3(noise_x, noise_y, noise_z_curr);
            
            // Calculate radii
            float base_radius = BREATHING_FONT_MAX_RADIUS / 2.0;
            float radius_prev = base_radius + noise_val_prev * NOISE_VARIANCE;
            float radius_curr = base_radius + noise_val_curr * NOISE_VARIANCE;
            
            // Clamp radii
            if (radius_prev < 1) radius_prev = 1;
            if (radius_prev > BREATHING_FONT_MAX_RADIUS) radius_prev = BREATHING_FONT_MAX_RADIUS;
            if (radius_curr < 1) radius_curr = 1;
            if (radius_curr > BREATHING_FONT_MAX_RADIUS) radius_curr = BREATHING_FONT_MAX_RADIUS;
            
            // Only update if radius changed
            int prev_radius_int = (int)radius_prev;
            int curr_radius_int = (int)radius_curr;
            
            if (prev_radius_int != curr_radius_int || prev_time == 0.0) {
              updateSingleBreathingCircle(circle_x, circle_y, prev_radius_int, curr_radius_int, color);
            }
          }
        }
      }
    }
    
    current_char_x += font->Width * spacing;
    str++;
  }
}

void loop(void) {
  // Animate the breathing circle font
  unsigned long current_time = millis();
  
  if (current_time - last_animation_time >= animation_interval) {
    // Get current text (in a real implementation, this would be the actual time)
    const char* time_str = "17:38";
    int spacing = 2 * BREATHING_FONT_MAX_RADIUS;
    int text_width = strlen(time_str) * Font16.Width * spacing;
    int text_height = Font16.Height * spacing;
    int center_x = (TFT_display.width() - text_width) / 2;
    int center_y = (TFT_display.height() - text_height) / 2;
    
    float time_param = current_time / 1000.0;  // Convert to seconds
    
    // Check if text has changed
    bool text_changed = (strcmp(time_str, prev_text) != 0);
    
    if (text_changed) {
      // Text changed - need to clear old area and draw new text structure
      // Clear previous text area if it exists
      if (strlen(prev_text) > 0) {
        int padding = BREATHING_FONT_MAX_RADIUS * 2;
        TFT_display.fillRect(prev_text_center_x - padding, prev_text_center_y - padding,
                             prev_text_width + padding * 2, prev_text_height + padding * 2, WHITE);
      }
      
      // Clear new text area
      int padding = BREATHING_FONT_MAX_RADIUS * 2;
      TFT_display.fillRect(center_x - padding, center_y - padding,
                           text_width + padding * 2, text_height + padding * 2, WHITE);
      
      // Draw new text structure (all circles at initial state)
      drawStringBreathing(center_x, center_y, time_str, &Font16, BLACK, time_param);
      
      // Update tracking variables
      strncpy(prev_text, time_str, sizeof(prev_text) - 1);
      prev_text[sizeof(prev_text) - 1] = '\0';
      prev_text_center_x = center_x;
      prev_text_center_y = center_y;
      prev_text_width = text_width;
      prev_text_height = text_height;
      prev_time_param = time_param;
    } else {
      // Text unchanged - only update breathing circles efficiently
      updateBreathingText(center_x, center_y, time_str, &Font16, BLACK, prev_time_param, time_param);
      prev_time_param = time_param;
    }
    
    last_animation_time = current_time;
  }
}
