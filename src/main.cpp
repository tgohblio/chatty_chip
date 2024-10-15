#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "Audio.h"
#include "ESP32Servo.h"
#include "wifi_settings.h"

// Digital I/O used
#define I2S_DOUT      25  // DIN connection
#define I2S_BCLK      27  // Bit clock
#define I2S_LRC       26  // Left Right Clock
#define SERVO_IN      13  // Servo pwm signal

Audio speaker;
Servo servo;
WiFiClientSecure client;

String ssid =     WIFI_SSID;
String password = WIFI_PASSWORD;

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
 * @brief Send HTTP GET request to the specified URL and host.
 * 
 * @param url The URL to send the request to.
 * @param host The host name of the server.
 * @param buffer A buffer to store the response in.
 * @return String string payload if successful, else empty string 
 */
String sendHttpsGET(String url, String host)
{
    String retVal;

    if(client.connect(host.c_str(), 443))
    {
        Serial.println("Connected to " + host);
        client.print("GET " + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: ESP32\r\n" + "Connection: close\r\n\r\n");
        while(client.connected())
        {
            String header = client.readStringUntil('\n');
            Serial.println(header);
            if (header == "\r")
                break;
        }
        retVal = client.readStringUntil('\n');
    }
    else
    {
        Serial.println("Connection failed!");
    }
    return retVal;
}

/** 
* @brief Send API to obtain external IP address for the board.
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

    // Set CA cert for this specific host
    client.setCACert(rootCA_ipify);

    Serial.println("\nStarting connection to server...");
    String result = sendHttpsGET("https://api.ipify.org/", "api.ipify.org");
    if(result != "")
    {
        retVal = true;
        Serial.println(result);
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
}

// put your main code here, to run repeatedly
void loop()
{

}
