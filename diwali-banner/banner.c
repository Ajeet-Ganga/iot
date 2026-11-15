#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// ===============================
// HARDWARE CONFIGURATION
// ===============================
#define LED_PIN       18
#define MATRIX_WIDTH   8
#define MATRIX_HEIGHT 32

#define BRIGHTNESS    25
// #define BRIGHTNESS    50
#define SCROLL_DELAY  70  // Milliseconds per pixel

const char MESSAGE[] = "HAPPY DIWALI!";

// ===============================
// LED MATRIX CONFIGURATION
// ===============================
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  MATRIX_WIDTH,
  MATRIX_HEIGHT,
  LED_PIN,

  NEO_MATRIX_TOP +
  NEO_MATRIX_LEFT + // Fix
  // NEO_MATRIX_RIGHT + // Fix
  NEO_MATRIX_ROWS +
  NEO_MATRIX_ZIGZAG,

  NEO_GRB + NEO_KHZ800
);

// Default Adafruit font: 5 pixels + 1 space
const int CHAR_WIDTH = 6;
const int MESSAGE_LENGTH = sizeof(MESSAGE) - 1;
const int MESSAGE_WIDTH = MESSAGE_LENGTH * CHAR_WIDTH;

// Temporary canvas for mirroring characters
GFXcanvas1 glyph(CHAR_WIDTH, 8);

int scrollX;
unsigned long frameCount = 0;

// ===============================
// SETUP
// ===============================
void setup() {
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(BRIGHTNESS);
  matrix.setTextWrap(false);
  matrix.setTextSize(1);

  // Preserve existing physical orientation
  matrix.setRotation(-1);

  matrix.fillScreen(0);
  matrix.show();

  // Preserve original physical scroll direction
  scrollX = -MESSAGE_WIDTH;

  Serial.println("Happy Diwali LED Display Started!");
}

// ===============================
// RAINBOW COLORS
// ===============================
uint16_t rainbowColor(int index) {
  // switch (index % 6) {
  //   case 0: return matrix.Color(255, 0, 0);
  //   case 1: return matrix.Color(255, 120, 0);
  //   case 2: return matrix.Color(255, 255, 0);
  //   case 3: return matrix.Color(0, 255, 0);
  //   case 4: return matrix.Color(0, 100, 255);
  //   default: return matrix.Color(200, 0, 255);
  // }
  switch (index % 2) {
    case 0: return matrix.Color(255, 0, 0);
    default: return matrix.Color(0, 255, 0);
  }
}

// ===============================
// DRAW ONE FRAME
// ===============================
void drawFrame() {
  matrix.fillScreen(0);

  for (int i = 0; i < MESSAGE_LENGTH; i++) {

    int x = scrollX + (i * CHAR_WIDTH);

    // Skip characters outside visible screen
    if (x < -CHAR_WIDTH || x >= matrix.width()) {
      continue;
    }

    // Reverse character order to compensate
    // for the physical horizontal mirroring
    char c = MESSAGE[MESSAGE_LENGTH - 1 - i];

    // Preserve original color animation
    int colorIndex = i + (frameCount / 5);
    uint16_t color = rainbowColor(colorIndex);

    // Draw character into a temporary canvas
    glyph.fillScreen(0);
    glyph.drawChar(0, 0, c, 1, 0, 1);

    // Flip character horizontally
    for (int px = 0; px < CHAR_WIDTH; px++) {
      for (int py = 0; py < 8; py++) {

        if (glyph.getPixel(px, py)) {
          matrix.drawPixel(
            x + CHAR_WIDTH - 1 - px,
            py,
            color
          );
        }
      }
    }
  }

  matrix.show();
}

void drawFrameV2() {
  matrix.fillScreen(0);
  matrix.setCursor(scrollX, 0); // The library handles the positioning
  
  for (int i = 0; i < MESSAGE_LENGTH; i++) {
    // Set the color for this specific character
    int colorIndex = i + (frameCount / 5);
    matrix.setTextColor(rainbowColor(colorIndex));
    
    // Print the character naturally
    matrix.print(MESSAGE[i]);
  }
  matrix.show();
}

// ===============================
// MAIN LOOP
// ===============================
void loop() {
  drawFrame();
  // drawFrameV2();

  delay(SCROLL_DELAY);

  // Preserve right-to-left physical scrolling
  scrollX++;
  frameCount++;

  // Restart after message disappears
  if (scrollX > matrix.width()) {
    scrollX = -MESSAGE_WIDTH;
  }
}
