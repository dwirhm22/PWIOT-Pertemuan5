#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// Konfigurasi WiFi
#define WIFI_SSID "rahma"
#define WIFI_PASSWORD "dwi123490046"

// Konfigurasi Firebase
#define API_KEY "AIzaSyBDyEz9QRv53BGQHxmUCXo950wbPwTTpG4"
#define DATABASE_URL "https://monitoring-suhu-83777-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Konfigurasi Sensor DHT22
#define DHT_PIN 18  // Ganti dengan pin DHT22 yang sesuai
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Konfigurasi WS2812
#define WS2812_PIN 37  // Ganti dengan pin yang sesuai
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

unsigned long sendDataPrevMillis = 0;

void setup() {
    Serial.begin(115200);

    // Koneksi WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    // Konfigurasi Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    Firebase.signUp(&config, &auth, "", "");
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Konfigurasi DHT22
    dht.begin();
    
    // Konfigurasi WS2812
    strip.begin();
    strip.show(); // Matikan semua LED awalnya
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
    strip.setPixelColor(0, strip.Color(red, green, blue));
    strip.show();
}

void loop() {
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000)) {
        sendDataPrevMillis = millis();
        
        // Membaca suhu dan kelembaban dari DHT22
        float temperature = dht.readTemperature(); // Celsius
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Gagal membaca dari sensor DHT!");
            return;
        }

        Serial.print("Suhu: ");
        Serial.print(temperature);
        Serial.print(" °C | Kelembaban: ");
        Serial.print(humidity);
        Serial.println(" %");

        // Logika WS2812 berdasarkan suhu
        if (temperature > 30) {
            setColor(255, 0, 0);  // Merah (Panas)
            Serial.println("Lampu Merah (Suhu Panas)");
        } else if (temperature < 25) {
            setColor(0, 0, 255);  // Biru (Dingin)
            Serial.println("Lampu Biru (Suhu Dingin)");
        } else {
            setColor(0, 255, 0);  // Hijau (Normal)
            Serial.println("Lampu Hijau (Suhu Normal)");
        }

        // Kirim data ke Firebase
        FirebaseJson json;
        json.set("temperature", temperature);
        json.set("humidity", humidity);
        json.set("lamp_status", (temperature > 30) ? "Red" : (temperature < 25) ? "Blue" : "Green");

        if (Firebase.RTDB.setJSON(&fbdo, "/sensor_data/" + String(millis()), &json)) {
            Serial.println("Data berhasil dikirim ke Firebase");
        } else {
            Serial.println("Gagal mengirim data");
            Serial.printf("Error: %s\n", fbdo.errorReason().c_str());
        }
    }
    delay(1000);
}
