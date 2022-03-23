#include "Arduino.h"
#include <stdlib.h>
#include "Audio.h"
#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "stations.h"
#include "encoder.h"
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// I2S
#define I2S_DOUT      23 // 17 // 22  // DIN connection
#define I2S_BCLK      26 // 18 // 26  // Bit clock
#define I2S_LRC       25 // 19 // 25  // Left Right Clock

// OLED
#define OLED_RESET -1 // no reset pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C

// VOLUME
#define VOLUME_PIN 39

//WiFi
// String ssid =     "ChillsideManor";
// String password = "chilldog";
 
Audio audio;
uint8_t volume = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int current_station = 2;
bool captive_portal_on = false;
bool wifi_connected = false;
bool buffering_audio = false;

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
        log_i("pot: %d %d", pot, volume);
    }

    // float new_volume_f = 21 - (((float)new_pot / 4095) * 21);
    // uint8_t new_volume = (uint8_t)new_volume_f;
    // log_i("nvf:%f nv:%d", new_volume_f, new_volume);
    // if(abs(new_volume - volume) > 0) // if the volume has changed by at least 1
    // {
    //     pot = 4096 - ((new_volume * 195) + 98); // place the pot value in the median value for that range
    //     volume = new_volume;
    //     audio.setVolume(volume); // 0...21
    //     log_i("pot: %d %d", pot, volume);
    // }
}

void on_radio_encoder(bool up)
{
    int new_station = current_station;
    if(up)
        new_station += (current_station < (NUM_STATIONS - 1));
    else
        new_station -= (current_station > 0);
    if(new_station != current_station)
        current_station = new_station;
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
    for(;;)
    {
        if(captive_portal_on)
        {
            display.setFont(&FreeSans9pt7b);
            display.setTextWrap(false);
            display.clearDisplay();
            display.setCursor(0, 22);
            display.println("SETUP WIFI");
            display.display();
        }
        else if(!wifi_connected)
        {
            // // display.setFont(&FreeSans9pt7b);
            // // display.setTextWrap(false);
            display.clearDisplay();
            // display.setCursor(0, 12);
            // display.println("Connecting to");
            // display.setCursor(0, 26);
            // display.println("WiFi");
            display.display();
        }
        else if(current_station != last_station)
        {
            last_station = current_station;
            display.clearDisplay();
            display.setFont(&FreeSans18pt7b);
            display.setCursor(10, 28);
            display.println(stations[current_station].callsign);
            display.display();
        }
        if( buffering_audio || !wifi_connected || show_dot )
        {
            display.fillRoundRect(display.width() - 10, display.height() / 2, 6, 6, 3, SSD1306_INVERSE);
            display.display();
            show_dot = !show_dot;
        }
        readVolume();
        vTaskDelay(10);
    }
}

static void audio_task(void* arg)
{
    static int last_station = -1;
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(0);
    for(;;)
    {
        if(current_station != last_station)
        {
            last_station = current_station;
            buffering_audio = true;
            audio.connecttohost(stations[last_station].URL);
            audio.setVolume(volume);
            buffering_audio = false;
        }
        audio.loop();
    }
}

void setup()
{
    disableCore0WDT();
    disableCore1WDT();    

    Serial.begin(115200);
    pinMode(VOLUME_PIN, INPUT);
    encoder_init(34,35);
    on_encoder = on_radio_encoder;

    xTaskCreatePinnedToCore(ui_task, "ui_task", 2048, NULL, 3, NULL, 1);

    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    // wm.resetSettings();
    wm.setAPCallback(configModeCallback);
    if(!wm.autoConnect("Campus Radio Radio", "campusradio"))
    {
        ESP.restart();
    }
    else
    {
        Serial.println("connected...yeey :)");
        captive_portal_on = false;
        wifi_connected = true;
        readVolume();
        xTaskCreatePinnedToCore(audio_task, "audio_task", 5000, NULL, 3  | portPRIVILEGE_BIT, NULL, 0);
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
