#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

const char* ssid = "Hotspot";
const char* password = "12345678";

// inisialisasi Bot Token
#define BOTtoken "5516395910:AAELB0hR25UA6KEVACzyaLyLBtBcduG5qjI"  // Bot Token dari BotFather

// chat id dari @myidbot
#define CHAT_ID "1994541524"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 100;
unsigned long lastTimeBotRan;

int ON = HIGH, OFF = LOW;

const int Pinled[] = { 2, 4, 5, 18, 19 };
bool Statusled[] = { OFF, OFF, OFF, OFF, OFF };
String controls[] = { "/Relay_1", "/Relay_2", "/Relay_3", "/Relay_4", "/Relay_5", "/ALL_OFF" };
int count = sizeof(controls) / sizeof(controls[0]);

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    for (int i = 0; i < (count - 1); i++) {
      if (text == controls[i]) {
        String message = controls[i] + " " + String(Statusled[i] ? "OFF" : "ON");
        bot.sendMessage(chat_id, message.c_str(), "");
        Statusled[i] = !Statusled[i];
        digitalWrite(Pinled[i], Statusled[i]);
      }
    }
    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Control Lednya.\n\n";
      control += "/Relay_1 Untuk ON/OFF Relay 1 \n";
      control += "/Relay_2 Untuk ON/OFF Relay 2 \n";
      control += "/Status Untuk Cek Status semua Relay Saat Ini \n";
      control += "/Tombol";
      bot.sendMessage(chat_id, control, "");
    }

    if (text == F("/Tombol")) {
      String control = "[";
      for (int i = 0; i < count; i++) {
        control += F("[{ \"text\" : \"");
        control += controls[i].substring(1);  //mengambil callback data setelah karakter slash (/)
        control += F("\", \"callback_data\" : \"");
        control += controls[i];
        control += F("\" }");
        if (i < sizeof(controls) / sizeof(controls[0]) - 1) {
          control += F("],");
        }
      }
      control += F(",{ \"text\" : \"Cek Status Relay\", \"callback_data\" : \"/Status\" }]]");
      bot.sendMessageWithInlineKeyboard(chat_id, "\nKontrol Perangkat", "", control);
    }
    // if (text == F("/Tombol")) {
    //         String control  = F("[[{ \"text\" : \"Relay 1\", \"callback_data\" : \"/Relay_1\" }],");
    //                control += F("[{ \"text\" : \"Relay 2\", \"callback_data\" : \"/Relay_2\" }],");
    //                control += F("[{ \"text\" : \"Cek Status Relay\", \"callback_data\" : \"/Status\" }]]");
    //                bot.sendMessageWithInlineKeyboard(chat_id, "\nKontrol Relay", "", control);
    //                }
    if (text == "/Status") {
      String message = "";
      for (int i = 0; i < (count - 1); i++) {
        message += controls[i] + " " + String(Statusled[i] ? "ON" : "OFF") + "\n";
      }
      bot.sendMessage(chat_id, message.c_str(), "");
    }
    // if (text == "/Status") {
    //   String message = " Relay 1 " + String(Statusled[0] ? "ON" : "OFF");
    //   message += "\nRelay 2 " + String(Statusled[1] ? "ON" : "OFF");
    //   bot.sendMessage(chat_id, message.c_str(), "");
    // }

    if (text == "/ALL_OFF") {
      Serial.println("Semuanya Off");
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < count; i++) {
    pinMode(Pinled[i], OUTPUT);
    digitalWrite(Pinled[i], Statusled[i]);
  }
  // Koneksi Ke Wifi
  WiFi.mode(WIFI_STA);
  WiFi.scanNetworks();
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif
  static unsigned long lastCheck = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastCheck > 5000) {
      lastCheck = millis();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
    }
    Serial.println("Koneksi WiFi terputus!");
  }
  Serial.println("Wifi Terhubung.....");
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Koneksi WiFi terputussssss!");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
    }
  }
}