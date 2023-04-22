#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#define EEPROM_SIZE 512 // Ukuran EEPROM
#define SSID_LENGTH 32   // Maksimal panjang SSID WiFi
#define PASS_LENGTH 64   // Maksimal panjang password WiFi

String botToken;
String chatID;
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void setup() {
  Serial.begin(115200);

  // Inisialisasi EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Baca SSID dan password dari EEPROM
  String ssid = EEPROM.readString(0);
  String password = EEPROM.readString(SSID_LENGTH + 1);
  botToken = EEPROM.readString(SSID_LENGTH + PASS_LENGTH + 2);
  chatID = EEPROM.readString(SSID_LENGTH + PASS_LENGTH + 2 + botToken.length() + 1);

  // Deklarasi objek WiFiManager
  WiFiManager wifiManager;

  // Buat parameter untuk bot token dan chat ID
  WiFiManagerParameter custom_botToken("botToken", "Bot Token", botToken.c_str(), 64);
  WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID.c_str(), 64);

  // Tambahkan parameter ke WiFiManager
  wifiManager.addParameter(&custom_botToken);
  wifiManager.addParameter(&custom_chatID);

  // Jika SSID dan password tidak kosong, coba terhubung ke WiFi
  if (ssid.length() > 0 && password.length() > 0) {
    Serial.println("Mencoba terhubung ke WiFi: " + ssid);
WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
  // Coba terhubung ke jaringan WiFi yang pernah digunakan
  if (!wifiManager.autoConnect()) {
    Serial.println("Gagal terhubung ke WiFi. Keluar dari program.");
    while (true) {}
  }
    // Tunggu hingga terhubung atau timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(100);
    }

    // Jika berhasil terhubung, tampilkan informasi WiFi dan keluar dari program setup()
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Terhubung ke WiFi. SSID: " + ssid + ", Password: " + password);
      return;
    }
  }

  // Jika tidak berhasil terhubung ke jaringan WiFi, masuk ke mode konfigurasi WiFi
  Serial.println("Gagal terhubung ke WiFi. Masuk ke mode konfigurasi.");

  // // Coba terhubung ke jaringan WiFi yang pernah digunakan
  // if (!wifiManager.autoConnect()) {
  //   Serial.println("Gagal terhubung ke WiFi. Keluar dari program.");
  //   while (true) {}
  // }

  // Ambil nilai dari parameter botToken dan chatID
  botToken = custom_botToken.getValue();
  chatID = custom_chatID.getValue();

  // Simpan SSID, password, botToken, dan chatID ke EEPROM
  EEPROM.writeString(0, WiFi.SSID());
  EEPROM.writeString(SSID_LENGTH + 1, WiFi.psk());
  EEPROM.writeString(SSID_LENGTH + PASS_LENGTH + 2, botToken);
  EEPROM.writeString(SSID_LENGTH + PASS_LENGTH + 2 + botToken.length() + 1, chatID);
  EEPROM.commit();
  EEPROM.end();

  Serial.println("Terhubung ke WiFi. SSID: " + WiFi.SSID() + ", Password: " + WiFi.psk());
}

void loop() {
  // Baca SSID dan password dari EEPROM
  String ssid = EEPROM.readString(0);
  String password = EEPROM.readString(SSID_LENGTH + 1);
String stored_botToken = EEPROM.readString(SSID_LENGTH + PASS_LENGTH + 2);
String stored_chatID = EEPROM.readString(SSID_LENGTH + PASS_LENGTH + 2 + stored_botToken.length() + 1);

Serial.println("Bot Token: " + stored_botToken);
Serial.println("Chat ID: " + stored_chatID);

  // Tampilkan SSID dan password di Serial Monitor
  Serial.println("SSID: " + ssid + ", Password: " + password);

  delay(3000);
}

