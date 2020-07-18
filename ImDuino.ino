// #define IMDUINO_TFT_22_ILI9225
// #define IMDUINO_SSD1306

#include "imgui.h"
texture_alpha8_t fontAtlas;

#ifdef IMDUINO_TFT_22_ILI9225
#  define SCREENX 220
#  define SCREENY 176
#  include "SPI.h"
#  include "TFT_22_ILI9225.h"

const uint8_t TFTLED = 32;
const uint8_t TFTRST = 33;
const uint8_t TFTRS  = 27;
const uint8_t TFTCS  = 15;
const uint8_t TFTCLK = 14;
const uint8_t TFTSDI = 13;

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFTRST, TFTRS, TFTCS, TFTLED, 128);
SPIClass tftspi(HSPI);
texture_color16_t screen;

void screen_init()
{
  tft.begin(tftspi);
  tft.setFont(Terminal6x8);
  tft.setOrientation(3);
  digitalWrite(TFTLED, HIGH);
  screen.init(SCREENX, SCREENY);
}

void screen_draw()
{
  tft.drawBitmap(0, 0, (uint16_t *)screen.pixels, screen.w, screen.h);
}
#endif

#ifdef IMDUINO_SSD1306
#  define SCREENX 128
#  define SCREENY 64
#  include "Adafruit_SSD1306.h"
#  include "Wire.h"

const uint8_t OLEDSDA  = 5;
const uint8_t OLEDSCL  = 4;
const int8_t OLEDRST   = -1;
const uint8_t OLEDADDR = 0x3C;

const uint8_t OLEDROTATION = 0; // = 2;

Adafruit_SSD1306 oled = Adafruit_SSD1306(SCREENX, SCREENY, &Wire, OLEDRST);
texture_alpha8_t screen;

void screen_init()
{
  Wire.begin(OLEDSDA, OLEDSCL);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLEDADDR, true, false))
  {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  screen.init(oled.width(), oled.height());
  oled.setRotation(OLEDROTATION);
}

void screen_draw()
{
  for (int y = 0; y < SCREENY; ++y)
    for (int x = 0; x < SCREENX; ++x)
      oled.drawPixel(
        x, y, screen.at(x, y).a > 0x7F ? SSD1306_WHITE : SSD1306_BLACK);
  oled.display();
}

#endif

unsigned long drawTime;
unsigned long renderTime;
unsigned long rasterTime;

ImGuiContext *context;

void setup()
{
  Serial.begin(115200);

  context = ImGui::CreateContext();

  ImGui_ImplSoftraster_Init(&screen);

  ImGuiStyle &style      = ImGui::GetStyle();
  style.AntiAliasedLines = false;
  style.AntiAliasedFill  = false;
  style.WindowRounding   = 0.0f;

  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->Flags |=
    ImFontAtlasFlags_NoPowerOfTwoHeight | ImFontAtlasFlags_NoMouseCursors;

  uint8_t *pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
  fontAtlas.init(width, height, (alpha8_t *)pixels);
  io.Fonts->TexID = &fontAtlas;

  screen_init();
}

float f         = 0.0f;
unsigned long t = 0;

void loop()
{
  ImGuiIO &io  = ImGui::GetIO();
  io.DeltaTime = 1.0f / 60.0f;

  // io.MousePos = mouse_pos;
  // io.MouseDown[0] = mouse_button_0;
  // io.MouseDown[1] = mouse_button_1;

  /* [0.0f - 1.0f] */
  io.NavInputs[ImGuiNavInput_Activate] =
    0.0f; // activate / open / toggle / tweak value       // e.g. Circle (PS4),
          // A (Xbox), B (Switch), Space (Keyboard)
  io.NavInputs[ImGuiNavInput_Cancel] =
    0.0f; // cancel / close / exit                        // e.g. Cross (PS4),
          // B (Xbox), A (Switch), Escape (Keyboard)
  io.NavInputs[ImGuiNavInput_Input] =
    0.0f; // text input / on-screen keyboard              // e.g. Triang.(PS4),
          // Y (Xbox), X (Switch), Return (Keyboard)
  io.NavInputs[ImGuiNavInput_Menu] =
    0.0f; // tap: toggle menu / hold: focus, move, resize // e.g. Square (PS4),
          // X (Xbox), Y (Switch), Alt (Keyboard)
  io.NavInputs[ImGuiNavInput_DpadLeft] =
    0.0f; // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad
          // Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
  io.NavInputs[ImGuiNavInput_DpadRight] = 0.0f;
  io.NavInputs[ImGuiNavInput_DpadUp]    = 0.0f;
  io.NavInputs[ImGuiNavInput_DpadDown]  = 0.0f;
  io.NavInputs[ImGuiNavInput_TweakSlow] =
    0.0f; // slower tweaks                                // e.g. L1 or L2
          // (PS4), LB or LT (Xbox), L or ZL (Switch)
  io.NavInputs[ImGuiNavInput_TweakFast] =
    0.0f; // faster tweaks                                // e.g. R1 or R2
          // (PS4), RB or RT (Xbox), R or ZL (Switch)

  ImGui_ImplSoftraster_NewFrame();
  ImGui::NewFrame();
  ImGui::SetWindowPos(ImVec2(0.0, 0.0));
  ImGui::SetWindowSize(ImVec2(SCREENX, SCREENY));

  f += 0.05;
  if (f > 1.0f) f = 0.0f;

  unsigned int deltaTime = millis() - t;
  t += deltaTime;

  deltaTime -= (drawTime + renderTime + rasterTime);

  ImGui::Text("Hardware write time %d ms", drawTime);
  ImGui::Text("Render time %d ms", renderTime);
  ImGui::Text("Raster time %d ms", rasterTime);
  ImGui::Text("Remaining time %d ms", deltaTime);
  ImGui::SliderFloat("SliderFloat", &f, 0.0f, 1.0f);

  renderTime = millis();
  ImGui::Render();
  renderTime = millis() - renderTime;

  rasterTime = millis();
  ImGui_ImplSoftraster_RenderDrawData(ImGui::GetDrawData());
  rasterTime = millis() - rasterTime;

  drawTime = millis();
  screen_draw();
  drawTime = millis() - drawTime;
}
