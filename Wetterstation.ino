
#include <FastLED.h>
#include <ArduinoJson.h>
#include <math.h>
#include <WiFiManager.h>

#define BRIGHTNESS 64
#define DATA_PIN D3
#define NUM_LEDS 6
//THUNDER Light preference. 0 is default light. 1 is randomBlink animation
int thunderLight = 1;

CRGB leds[NUM_LEDS];

WiFiManager wifiManager;
WiFiClient client;

// ========================  hier deinen API-Key eintragen!!!  ============================================================================================================
const String city = "";
const String api_key = "";    // dein Open-Weather-Map-API-Schluessel, kostenlos beziehbar ueber https://openweathermap.org/
// ========================================================================================================================================================================

// Variablen Deklarationen
int weatherID = 0;
int weatherID_shortened = 0;
String weatherforecast_shortened = " ";
int temperature_Celsius_Int = 0;
unsigned long lastcheck = 0;
//for randomBlink animation
unsigned long lastTime = 0;
int waitTime = 0;

//Testing: Use testweatherID/testweatherID_shortened instead of default values
boolean testing = true;
int testweatherID = 701;
int testweatherID_shortened = 6;
//2 ist Sturm
//3 ist Niesel
//5 ist Regen
//6 ist Schnee
//8 ist Wolken

// =====================================================================setup()===================================================================================================

//Alles wie in der letzten Abgabe
void setup() {
  Serial.begin(115200);
  //LED wird initialisiert
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 400);
  FastLED.clear();
  FastLED.show();
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB( 253, 184, 19);
  }
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.show();

  wifiManager.autoConnect("deineWetterLampe");

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB( 0, 0, 0);
  }
  FastLED.show();
  delay(2000);

  getCurrentWeatherConditions();
}

// =======================================================================loop()=================================================================================================


void loop() {                                 //Loop methode spielt Animationen ab, und prüft jede 30min Wetter

  if (millis() - lastcheck >= 1800000) {      //if Anweisung prüft ob 30min vergangen sind
    getCurrentWeatherConditions();            //Wetter Daten werden erfasst
    lastcheck = millis();
    topdown(0,0,0);                           //clears LEDs after switch
  }
  if (weatherID == 800) {
    LED_effect_clearSky();
    Serial.println(weatherforecast_shortened); delay(200); //weatherforecast_shortened = "Sun" falls weatherID = 800
  } else if (weatherID == 801 ) {
    LED_effect_sunClouds();
    Serial.println(weatherforecast_shortened); delay(200); //weatherforecast_shortened = "Clouds with Sun" weatherID = 801
  } else if (weatherID == 615 || weatherID == 616) {
    LED_effect_snowRain();
    Serial.println(weatherforecast_shortened); delay(200); //weatherforecast_shortened = "Snow with Rain" weatherID = 615/616
  } else if (weatherID == 200 || weatherID == 201 || weatherID == 202) {
    LED_effect_stormRain();
    Serial.println(weatherforecast_shortened); delay(200);//weatherforecast_shortened = "Storm with Rain" weatherID = 200/201/202
  } else if (weatherID == 230 || weatherID == 231 || weatherID == 232) {
    LED_effect_stormDrizzle();
    Serial.println(weatherforecast_shortened); delay(200); //weatherforecast_shortened = "Storm with Drizzle" weatherID = 230/231/232
  } else if (weatherID == 701 || weatherID == 741) {
    LED_effect_drizzle();
    Serial.println(weatherforecast_shortened); delay(200);//weatherforecast_shortened = "Fog or Mist" weatherID = 701/741
  } else {
    switch (weatherID_shortened) {
      case 2: LED_effect_thunder(); Serial.println(weatherforecast_shortened); delay(200);  break;
      case 3: LED_effect_drizzle(); Serial.println(weatherforecast_shortened); delay(200); break;
      case 5: LED_effect_rain(); Serial.println(weatherforecast_shortened); delay(200); break;
      case 6: LED_effect_snow(); Serial.println(weatherforecast_shortened); delay(200); break;
      case 8: LED_effect_cloudy(); Serial.println(weatherforecast_shortened); delay(200); break;
      default: LED_effect_default(); break;
    }

  }
}
// ======================================================================getCurrentWeatherConditions()==================================================================================================

void getCurrentWeatherConditions() {                                                                           //Verbindet zu OpenWeather, und dann werden Wetter Daten in Variablen gespeichert
  int WeatherData;
  Serial.print("connecting to "); Serial.println("api.openweathermap.org");
  if (client.connect("api.openweathermap.org", 80)) {                                                         //WifiManager versucht Verbindung zu OpenWeather
    client.println("GET /data/2.5/weather?q=" + city + ",DE&units=metric&lang=de&APPID=" + api_key);          //Stadt und API-key werden eingegeben
    client.println("Host: api.openweathermap.org");                                                           //Hostname gegeben
    client.println("Connection: close");                                                                      //client wird gesagt, dass die Verbindung geschlossen wird
    client.println();
  } else {     //Falls Verbindung fehlschlägt
    Serial.println("connection failed");
  }
  //Konstante der Größe der JSON Datei wird angegeben
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(14) + 360;
  DynamicJsonDocument doc(capacity);            //Json Doc (doc) wird erstellt, der Größe capacity
  deserializeJson(doc, client);                 // doc derserialisiert den Daten von OpenWeather
  client.stop();                                //Verbindung zu OpenWeather wird geschlossen

  weatherID = doc["weather"][0]["id"];                 //weatherID wird von doc gelesen
  int temperature_Celsius = doc["main"]["temp"];       //temperatur wird von doc gelesen
  temperature_Celsius_Int = (int)temperature_Celsius;  //temperatur wird als int gecastet
  weatherID_shortened = weatherID / 100;               //verkürzte weatherID wird berechnet

  switch (weatherID_shortened) {                       //switch Anweisung um weatherforecast_shortened zu bestimmen
    case 2: weatherforecast_shortened = "Stormy"; break;
    case 3: weatherforecast_shortened = "Drizzle"; break;
    case 5: weatherforecast_shortened = "Rain"; break;
    case 6: weatherforecast_shortened = "Snow"; break;
    case 8: weatherforecast_shortened = "Clouds"; break;
    default: weatherforecast_shortened = "no data"; break;
  }

  switch (weatherID) {
    case 800: weatherforecast_shortened = "Sun"; break;  
    case 801: weatherforecast_shortened = "Clouds with Sun"; break;
    case 701:
    case 741: weatherforecast_shortened = "Fog or Mist"; break;
    case 615:
    case 616: weatherforecast_shortened = "Snow with Rain"; break;
    case 621: weatherforecast_shortened = "Light Snow"; break;
    case 622: weatherforecast_shortened = "Heavy Snow"; break;
    case 200:
    case 201:
    case 202: weatherforecast_shortened = "Storm with Rain"; break;
    case 230:
    case 231:
    case 232: weatherforecast_shortened = "Storm with Drizzle"; break;
  }
}
// =======================================================================Animationen=================================================================================================

void LED_effect_clearSky() { // Effect for case 800
  leds[3] = CRGB(255, 204, 0);
  FastLED.setBrightness(200);
  FastLED.show();
}

void LED_effect_sunClouds() {  //Effect for case 801
  leds[2] = CRGB::White;
  FastLED.show();
  FastLED.setBrightness(100);
  fade(3, 5, 20, 255, 204, 0, 0, 0, 0);
  fade(3, 5, 20, 0, 0, 0, 255, 204, 0);
}

void LED_effect_snowRain() { //Effect for cases 615, 616
  leds[1] = CRGB::Blue;
  leds[5] = CRGB::White;
  FastLED.show();
  FastLED.setBrightness(200);
}

void LED_effect_stormRain() { //Effect for cases 200,201,202
  randomBlink(0, 43, 2579, 7, 253, 208, 35); //led 0, duration 43, between 2579, times 7, and then RGB values
  leds[1] = CRGB::Blue;
  FastLED.show();
  FastLED.setBrightness(200);
}

void LED_effect_stormDrizzle() { //Effect for cases 230,231,232
  randomBlink(0, 43, 2579, 7, 253, 208, 35); //led 0, duration 43, between 2579, times 7, and then RGB values
  leds[4] = CRGB::White;
  FastLED.show();
  FastLED.setBrightness(100);
}

void LED_effect_cloudy() { //Effect for cases {802..}
  leds[2] = CRGB::White;
  FastLED.show();
  FastLED.setBrightness(200);

}

void LED_effect_rain() {       //Effect for case 5
  switch (weatherID) { //use testweatherID for testing
    case 500:
    case 520: leds[1] = CRGB(0, 0, 20); break; //Light Rain
    case 501:
    case 521: leds[1] = CRGB(0, 0, 150); break; //Medium Rain
    case 502:
    case 503:
    case 504:
    case 522: leds[1] = CRGB(0, 0, 255); break; //Heavy Rain
    default: leds[1] = CRGB(0, 0, 200); break; //Other Rain
  }
  FastLED.show();
}

void LED_effect_snow() {      // Effect for case 6
  switch (weatherID) { //use testweatherID for testing
    case 600:
    case 620: fade(5, 5, 20, 80, 80, 80, 0, 0, 0); delay(20); fade(5, 5, 20, 0, 0, 0, 80, 80, 80); break;       //Light Snow
    case 601:
    case 621: fade(5, 5, 20, 180, 180, 180, 0, 0, 0); delay(20); fade(5, 5, 20, 0, 0, 0, 180, 180, 180); break; //Medium Snow
    case 602:
    case 622: leds[5] = CRGB(255, 255, 255); break; //Heavy Snow
    default: fade(5, 5, 20, 255, 255, 255, 0, 0, 0); delay(20); fade(5, 5, 20, 0, 0, 0, 255, 255, 255); break; //Other Snow
  }
  FastLED.show();
}

void LED_effect_drizzle() {  // Effect for cases 3, 701, 741
  leds[4] = CRGB::White;
  FastLED.show();
  FastLED.setBrightness(100);
}

void LED_effect_thunder() {
  if (thunderLight == 1) {
    randomBlink(0, 43, 2579, 7, 253, 208, 35); //led 0, duration 43, between 2579, times 7, and then RGB values
  } else {
    leds[0] = CRGB(253, 208, 35);
    FastLED.show();
    FastLED.setBrightness(200);
  }
}

void LED_effect_default() {
  topdown(255, 0, 0);
}


// ====================================================================EXTRA====================================================================================================

void topdown(int r, int g, int b) {
  for (int j = NUM_LEDS - 1; j >= 0; j--) {
    delay(50);
    leds[j].red = r;
    leds[j].green = g;
    leds[j].blue = b;
    FastLED.show();

  }
}

void downtop(int r, int g, int b) {
  for (int j = 0; j <= NUM_LEDS - 1; j++) {
    delay(50);
    leds[j].red = r;
    leds[j].green = g;
    leds[j].blue = b;
    FastLED.show();

  }
}

void fade(int led_position, uint16_t duration, uint16_t delay_val, uint16_t startR, uint16_t startG, uint16_t startB, uint16_t endR, uint16_t endG, uint16_t endB) {
  int16_t redDiff = endR - startR;
  int16_t greenDiff = endG - startG;
  int16_t blueDiff = endB - startB;
  int16_t steps = duration * 1000 / delay_val;
  int16_t redValue, greenValue, blueValue;
  for (int16_t i = 0 ; i < steps - 1 ; ++i) {
    redValue = (int16_t)startR + (redDiff * i / steps);
    greenValue = (int16_t)startG + (greenDiff * i / steps);
    blueValue = (int16_t)startB + (blueDiff * i / steps);
    leds[led_position] = CRGB(redValue, greenValue, blueValue);
    FastLED.show();
    delay(delay_val);
  }
  leds[led_position] = CRGB(endR, endG, endB);
}

//used Idea from robtillart https://forum.arduino.cc/index.php?topic=105030.0
void randomBlink(int led_position, uint16_t duration, uint16_t between, uint16_t times, int16_t redValue, int16_t greenValue, int16_t blueValue) {
  if (millis() - waitTime > lastTime)  // time for a new flash
  {
    lastTime += waitTime;
    waitTime = random(between);

    for (int i = 0; i < random(times); i++)
    {
      leds[led_position] = CRGB(redValue, greenValue, blueValue);
      FastLED.show();
      delay(20 + random(duration));
      leds[led_position] = CRGB::Black;
      FastLED.show();
      delay(10);
    }
  }
}
