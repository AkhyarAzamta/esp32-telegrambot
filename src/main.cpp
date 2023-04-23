#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#define EEPROM_SIZE 512 // Ukuran EEPROM
#define SSID_LENGTH 32  // Maksimal panjang SSID WiFi
#define PASS_LENGTH 64  // Maksimal panjang password WiFi

String botToken;
String chatID;
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

String ssid;
String password;
int botRequestDelay = 100;
unsigned long lastTimeBotRan;

int ON = HIGH, OFF = LOW;

const int Pinled[] = {2, 4, 5, 18, 19};
bool Statusled[] = {OFF, OFF, OFF, OFF, OFF};
String controls[] = {"/relay1", "/relay2", "/relay3", "/relay4", "/relay5", "/status", "/reset"};
int count = sizeof(controls) / sizeof(controls[0]);

void S_resetSettings()
{
  // Membuat objek WiFiManager
  WiFiManager wifiManager;

  // Memutuskan koneksi Wi-Fi yang aktif
  WiFi.disconnect();

  // Menghapus data SSID dan password dari memori EEPROM
  for (int i = 0; i < SSID_LENGTH; i++) {
    EEPROM.write(i, 0);
    EEPROM.write(i, 1);
    EEPROM.write(i, 2);
  }
  for (int i = SSID_LENGTH; i < SSID_LENGTH + PASS_LENGTH; i++) {
    EEPROM.write(i, 0);
    EEPROM.write(i, 1);
    EEPROM.write(i, 2);
  }
  EEPROM.commit();
   // Buat parameter untuk bot token dan chat ID
  WiFiManagerParameter custom_botToken("botToken", "Bot Token", botToken.c_str(), 64);
  WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID.c_str(), 64);

  // Tambahkan parameter ke WiFiManager
  wifiManager.addParameter(&custom_botToken);
  wifiManager.addParameter(&custom_chatID);
  // // Menjalankan mode konfigurasi Wi-Fi
  wifiManager.startConfigPortal();
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
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot.updateToken(botToken);
  Serial.println("Terhubung ke WiFi. SSID: " + WiFi.SSID() + ", Password: " + WiFi.psk());
}


void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != chatID)
    {
      bot.sendMessage(chat_id, "Sorry gaes khusus @akhyar_azamta", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    for (int i = 0; i < (count - 1); i++)
    {
      if (text == controls[i])
      {
        String message = controls[i] + " " + String(Statusled[i] ? "OFF" : "ON");
        bot.sendMessage(chat_id, message.c_str(), "");
        Statusled[i] = !Statusled[i];
        digitalWrite(Pinled[i], Statusled[i]);
      }
    }
    if (text == "/start")
    {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Control Lednya.\n\n";

      for (int i = 0; i < count; i++)
      {
        if (controls[i] == "/status")
        {
          control += "/status Untuk Cek Status semua Relay Saat Ini \n";
        }
        else if (controls[i] == "/reset")
        {
          control += "/reset Untuk Konfigurasi Ulang\n";
        }
        else
        {
          control += controls[i] + " Untuk ON/OFF Relay " + String(i + 1) + " \n";
        }
      }
      control += "/button Untuk Menampilkan Tombol";
      bot.sendMessage(chat_id, control, "");
    }

    else if (text == F("/button"))
    {
      String control = "[";
      for (int i = 0; i < count; i++)
      {
        control += F("[{ \"text\" : \"");
        control += controls[i].substring(1); // mengambil callback data setelah karakter slash (/)
        control += F("\", \"callback_data\" : \"");
        control += controls[i];
        control += F("\" }");
        if (i < sizeof(controls) / sizeof(controls[0]) - 1)
        {
          control += F("],");
        }
      }
      control += F("]]");
      // control += F(",{ \"text\" : \"Cek Status Relay\", \"callback_data\" : \"/Status\" }]]");
      bot.sendMessageWithInlineKeyboard(chat_id, "\nKontrol Perangkat", "", control);
    }
    else if (text == "/status")
    {
      String message = "";
      for (int i = 0; i < (count - 1); i++)
      {
        message += controls[i] + " " + String(Statusled[i] ? "ON" : "OFF") + "\n";
      }
      bot.sendMessage(chat_id, message.c_str(), "");
    }
    else if (text == "/reset" )
    {
      bot.sendMessage(chat_id, "Sambungkan ke wifi 'ESP32-SmartHome'", "");
      S_resetSettings();
    }
    else
    {
      if (text != controls[i])
      {
        bot.sendMessage(chat_id, "Perintah tidak dikenali, Ketik /start untuk list perintah.", "");
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < count; i++)
  {
    pinMode(Pinled[i], OUTPUT);
    digitalWrite(Pinled[i], Statusled[i]);
  }
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif

  // Inisialisasi EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Baca SSID dan password dari EEPROM
  ssid = EEPROM.readString(0);
  password = EEPROM.readString(SSID_LENGTH + 1);
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
// wifiManager.autoConnect("ESP32-SmartHome", "password");
  // Jika SSID dan password tidak kosong, coba terhubung ke WiFi
  if (ssid.length() > 0 && password.length() > 0)
  {
    Serial.println("Mencoba terhubung ke WiFi: " + ssid);
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    // Coba terhubung ke jaringan WiFi yang pernah digunakan
    if (!wifiManager.autoConnect())
    {
      Serial.println("Gagal terhubung ke WiFi. Keluar dari program.");
      // while (true) {}
      Serial.println("ESP di Restart Ulang!!!");
      ESP.restart();
    }
    // Tunggu hingga terhubung atau timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
    {
      delay(100);
    }
  }

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
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot.updateToken(botToken);
  Serial.println("Terhubung ke WiFi. SSID: " + WiFi.SSID() + ", Password: " + WiFi.psk());
}

void loop()
{
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000)
  {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("Koneksi WiFi terputussssss!");
      WiFi.begin(ssid.c_str(), password.c_str());
    }
    else
    {
      Serial.println("Koneksi WiFi Tersambung!");
    }
  }
  // Serial.println("BotToken: " + botToken + ", chatID: " + chatID);
  // delay(3000);
}