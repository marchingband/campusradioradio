#include "Arduino.h"
#include <stdlib.h>
#include "Audio.h"
#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "encoder.h"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// ENCODER
#define ENC_PUSH 19
#define ENC_A 34
#define ENC_B 35

// OLED
#define OLED_RESET -1 // no reset pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
#define DISPLAY_ON_SECONDS 15
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// SDA pin 21
// SCL pin 22

// AUDIO
#define I2S_DOUT      23 // 17 // 22  // DIN connection
#define I2S_BCLK      26 // 18 // 26  // Bit clock
#define I2S_LRC       25 // 19 // 25  // Left Right Clock
#define VOLUME_PIN 39
Audio audio;
uint8_t volume = 0;

// STATIONS
#define JSON_HOST "https://marchingband.github.io/campusradioradio/data/rowans_stations.json" // canada
// #define JSON_HOST "https://marchingband.github.io/campusradioradio/data/stations-idaho.json" // idaho
Preferences preferences;
struct SpiRamAllocator {
    void* allocate(size_t size) {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }
    void deallocate(void* pointer) {
        heap_caps_free(pointer);
    }
    void* reallocate(void* ptr, size_t new_size) {
        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
};
typedef BasicJsonDocument<SpiRamAllocator> SpiRamJsonDocument;
SpiRamJsonDocument *stations;

// VARS
unsigned int current_station;
bool captive_portal_on = false;
bool wifi_connected = false;
bool buffering_audio = false;
bool fetching_stations = false;
unsigned int last_display_update = 0;
bool should_wake_display = false;
int num_stations = 0;

void use_screen(void){
    last_display_update = millis();
}

bool should_sleep(void){
    return(millis() - last_display_update > (1000 * DISPLAY_ON_SECONDS));
}

void readVolume(void)
{
    static int pot = 0;
    static int reads = 0;
    
    int new_pot = analogRead(VOLUME_PIN);

    // discharge the adc pin
    pinMode(VOLUME_PIN,OUTPUT);
    digitalWrite(VOLUME_PIN,HIGH);
    pinMode(VOLUME_PIN, INPUT);

    // debounce the pot
    if(abs(new_pot - pot) > 186){ // is it relevant?
        if(reads++ < 5){ // not enough in a row
            return;
        } else { // a real reading, continue
            reads = 0;
        }
    } else { // wasnt a relevant read
        reads = 0;
        return;
    }

    uint8_t new_volume = (4096 - new_pot) / 186;

    if(new_volume != volume){
        volume = new_volume;
        pot = 4096 - (new_volume * 186); // place the pot value in the lower bound for that range
        pot += volume >= 21 ? (-98) : 98; // place the pot value in the median value for that range
        audio.setVolume(volume); // 0...21
        use_screen();
        should_wake_display = true;
        log_i("pot: %d %d", pot, volume);
    }
}

void on_radio_encoder(bool up)
{
    use_screen();
    should_wake_display = true;
    int new_station = current_station;
    if(up)
    {
        new_station += (current_station < (num_stations - 1));
    }
    else
    {
        new_station -= (current_station > 0);
    }
    if(new_station != current_station)
    {
        current_station = new_station;
        preferences.putUInt("station", current_station);
    }
}

void configModeCallback (WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    captive_portal_on = true;
}

static void ui_task(void* arg)
{
    static int last_station  = -1;
    static bool show_dot = false;

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.setFont(&FreeSans18pt7b);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0);
    use_screen();
    for(;;)
    {
        if(captive_portal_on)
        {
            use_screen();
            display.setFont(&FreeSans9pt7b);
            display.setTextWrap(false);
            display.clearDisplay();
            display.setCursor(0, 22);
            display.println("SETUP WIFI");
            display.display();
        }
        else if(fetching_stations)
        {
            use_screen();
            display.setFont(&FreeSans9pt7b);
            display.setTextWrap(false);
            display.clearDisplay();
            display.setCursor(0, 18);
            display.println("loading stations");
            display.display();
        }
        else if(!wifi_connected)
        {
            use_screen();
            display.clearDisplay();
            display.display();
        }
        else if(
            (current_station != last_station) ||
            should_wake_display
        )
        {
            should_wake_display = false;
            use_screen();
            last_station = current_station;

            JsonArray data = stations->as<JsonArray>();
            JsonArray station_data = data[last_station].as<JsonArray>();
            const char* station_callsign = station_data[0];

            display.clearDisplay();
            display.setFont(&FreeSans18pt7b);
            display.setCursor(10, 28);
            display.println(station_callsign);
            display.display();
        }
        if( buffering_audio || !wifi_connected || show_dot )
        {
            use_screen();
            display.fillRoundRect(display.width() - 10, display.height() / 2, 6, 6, 3, SSD1306_INVERSE);
            display.display();
            show_dot = !show_dot;
        }
        readVolume();
        if(should_sleep())
        {
            display.clearDisplay();
            display.display();
        }
        vTaskDelay(10);
    }
}

static void audio_task(void* arg)
{
    static int last_station = -1;
    audio.setBufsize(-1, 3000000); // -1 is no ram, 3MB psram (defualt is 300k)
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(0);
    for(;;)
    {
        // guard against bad station indexes
        if( current_station > num_stations)
        {
            current_station = 0;
        }

        if(current_station != last_station)
        {
            bool connected_to_stream = false;
            while(!connected_to_stream) // keep retrying if connection fails
            {
                last_station = current_station < (num_stations - 1) ? current_station : (num_stations - 1);
                buffering_audio = true;
                JsonArray data = stations->as<JsonArray>();
                JsonArray station_data = data[last_station].as<JsonArray>();
                const char* station_callsign = station_data[0];
                const char* station_host = station_data[1];
                log_i("connecting to %s %s", station_callsign, station_host);
                connected_to_stream = audio.connecttohost(station_host);
                // audio.connecttohost(station_host);
                audio.setVolume(volume);
            }
            buffering_audio = false;
        }
        audio.loop();
    }
}

bool fetch_json(void)
{
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(JSON_HOST);
        int httpResponceCode = http.GET();
        if (httpResponceCode > 0) {
            Serial.println(httpResponceCode);
            DeserializationError error = deserializeJson(*stations, http.getStream());
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                http.end();
                return 1;
            }
            num_stations = stations->as<JsonArray>().size();
            log_i("found %d stations", num_stations);
            return 0;
        } else {
            Serial.print("err:");
            Serial.println(httpResponceCode);
            http.end();
            return 1;
        }
        http.end();
        return 0;
    } else {
        Serial.println("wifi err");
        return 1;
    }
}

void get_json(void)
{
    fetching_stations = true;
    stations = new SpiRamJsonDocument(60000);
    while(fetching_stations)
    {
        fetching_stations = fetch_json();
    }
}

void setup()
{
    disableCore0WDT();
    disableCore1WDT();    

    Serial.begin(115200);

    preferences.begin("eeprom", false);
    current_station = preferences.getUInt("station", 0);

    pinMode(VOLUME_PIN, INPUT);
    pinMode(ENC_PUSH, INPUT_PULLUP);

    encoder_init();
    on_encoder = on_radio_encoder;

    xTaskCreatePinnedToCore(ui_task, "ui_task", 4096, NULL, 3, NULL, 1);

    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    // wm.resetSettings();
    wm.setAPCallback(configModeCallback);
    int normal_mode = digitalRead(ENC_PUSH);
    log_i("enc push %s", normal_mode == 1 ? "high" : "low");
    if(normal_mode == 0)
    {
        captive_portal_on = true;
        wm.startConfigPortal("Campus Radio Radio", "campusradio");
        log_i("config closed");
        delay(1000);
        ESP.restart();
    }

    if(!wm.autoConnect("Campus Radio Radio", "campusradio"))
    {
        ESP.restart();
    }
    else
    {
        Serial.println("connected...yeey :)");
        captive_portal_on = false;
        wifi_connected = true;
        get_json();
        readVolume();
        xTaskCreatePinnedToCore(audio_task, "audio_task", 8000, NULL, 3  | portPRIVILEGE_BIT, NULL, 0);
    }
}

void loop(){
    vTaskDelay(1000);
    // vTaskDelete(NULL);
}
 
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
