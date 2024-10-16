#include "Arduino.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "Audio.h"
#include "ESP32Servo.h"
#include "wifi_settings.h"

// Digital I/O used
#define I2S_DOUT      25  // DIN connection
#define I2S_BCLK      27  // Bit clock
#define I2S_LRC       26  // Left Right Clock
#define SERVO_IN      13  // Servo pwm signal

// REST API
#define SERVER_URL     "https://rag-chatbot-ddfu.onrender.com"
#define SERVER_HOST    "rag-chatbot-ddfu.onrender.com"

// Globals
Audio speaker;
Servo servo;
HTTPClient https;

String ssid =     WIFI_SSID;
String password = WIFI_PASSWORD;
String prevFile = "";
String mp3File = "";
int errorCode = 0;
JsonDocument resp;
bool isNew = false;
bool isPlay = false;

// certificate for https://rag-chatbot-ddfu.onrender.com
// GlobalSign Root CA, valid until Fri Jan 28 2028, size: 1265 bytes 
// generated from https://projects.petrucci.ch/esp32/
const char* rootCA_onrender = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX\n" \
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n" \
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx\n" \
"NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n" \
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0\n" \
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube\n" \
"Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e\n" \
"WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd\n" \
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd\n" \
"BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN\n" \
"l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw\n" \
"Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v\n" \
"Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG\n" \
"SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ\n" \
"odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY\n" \
"+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs\n" \
"kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep\n" \
"8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1\n" \
"vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl\n" \
"-----END CERTIFICATE-----\n";

// certificate for https://api.ipify.org
// GlobalSign Root CA, valid until Fri Jan 28 2028, size: 1265 bytes
// generated from https://projects.petrucci.ch/esp32/
const char *rootCA_ipify = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX\n" \
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n" \
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx\n" \
"NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n" \
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0\n" \
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube\n" \
"Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e\n" \
"WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd\n" \
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd\n" \
"BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN\n" \
"l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw\n" \
"Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v\n" \
"Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG\n" \
"SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ\n" \
"odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY\n" \
"+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs\n" \
"kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep\n" \
"8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1\n" \
"vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl\n" \
"-----END CERTIFICATE-----\n" \
"";

/** 
* @brief Send API to obtain external IP address.
* 
* Sample output:
* =============
* Starting connection to server...
* Connected to server!
* HTTP/1.1 200 OK
* Date: Tue, 15 Oct 2024 03:49:52 GMT
* Content-Type: text/plain
* Content-Length: 15
* Connection: close
* Vary: Origin
* CF-Cache-Status: DYNAMIC
* Server: cloudflare
* CF-RAY: 8d2cd7781f70a11f-SIN
* 
* 260.255.234.152
*/
bool getExternalIP( void )
{
    bool retVal = false;

    Serial.print("\nStarting connection to server...");
    if (https.begin("https://api.ipify.org/", rootCA_ipify))
    {
        retVal = true;
        Serial.println(" connected.");
        // start connection and send HTTP header
        int httpCode = https.GET();
        // httpCode will be negative on error
        if (httpCode > 0)
        {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
            // HTTP header has been send and Server response header has been handled
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                // print server response payload
                String payload = https.getString();
                Serial.println(payload);
            }
        }
        else 
        {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
    }
    else
    {
        Serial.println(" failed!");
    }
    return retVal;
}

void sendGETHealth( void )
{
    String api = String("/api/heartbeat");
    String url = String(SERVER_URL) + api;

    Serial.printf("\n[HTTPS] GET %s... ", api.c_str());
    if(https.begin(url, rootCA_onrender))
    {
        int httpCode = https.GET();
        if(httpCode == HTTP_CODE_OK)
        {
            String payload = https.getString();
            deserializeJson(resp, payload);
            if(resp["status"] == "running")
                Serial.println("200 OK");
        }
        else
        {
            Serial.printf("failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
    }
    else
    {
        Serial.println("failed!");
    }
}

bool sendGETLatestAudioResponse( void )
{
    bool retVal= false;
    String api = String("/api/audio/latest");
    String url = String(SERVER_URL) + api;

    Serial.printf("\n[HTTPS] GET %s... ", api.c_str());
    if(https.begin(url, rootCA_onrender))
    {
        int httpCode = https.GET();
        if(httpCode == HTTP_CODE_OK)
        {
            Serial.println("200 OK");
            String payload = https.getString();
            deserializeJson(resp, payload);
            if( (payload.isEmpty() == false) && (resp["file"] != "") )
            {
                mp3File = String((const char *)(resp["file"]));

                if(mp3File != prevFile)
                {
                    Serial.printf("\nnew file: %s\r\n", mp3File.c_str());
                    prevFile = mp3File;
                    retVal = true;
                }
            }
        }
        else
        {
            Serial.printf("failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
    }
    else
    {
        Serial.println("failed!");
    }
    return retVal;
}

// put your setup code here, to run once
void setup()
{
    wl_status_t status = WL_DISCONNECTED;
    
    Serial.begin(115200);

    servo.attach(SERVO_IN);
    speaker.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    speaker.setVolume(15); // 0...21

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("Connecting to SSID %s", ssid);
    while(status != WL_CONNECTED)
    {
        Serial.printf(".");
        status = WiFi.status();
        delay(1000);
    }
    Serial.println(" connected");

    getExternalIP();

    // speaker.connecttohost("https://playerservices.streamtheworld.com/api/livestream-redirect/987FM.mp3");
}



// put your main code here, to run repeatedly
void loop()
{
        // // Poll for new mp3 files
        // sendGETHealth();
#if 1
    if(!isPlay)
    {
        isNew = sendGETLatestAudioResponse();

        if(isNew)
        {
            String mp3URL = String(SERVER_URL) + String("/api/stream/") + mp3File;
            Serial.printf("Streaming from %s ...", mp3URL.c_str());
            speaker.connecttohost(mp3URL.c_str());
            isPlay = true;
            isNew = false;
        }
        delay(1000);
    }
    else
    {
        speaker.loop();
    }
#else
        speaker.loop();
#endif
}

void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}

void audio_eof_stream(const char *info){
    Serial.print("eof_stream  ");Serial.println(info);
    isPlay = false;  // trigger to poll for next audio file
}

void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}

void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
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

