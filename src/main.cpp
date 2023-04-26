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
int reading, cont;
String ssid;
String password;
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

int ON = HIGH, OFF = LOW;

// int buttonState = 0;
// int buttonState[8];
// const int PinButton[] = {13, 12, 14, 27, 33, 32, 15, 4};
const int PinButton[] = {13, 12, 14, 27, 33};
const int Pinled[] = {2, 4, 5, 18, 19};
int Statusled[5] = {OFF, OFF, OFF, OFF, OFF};
int buttonState[5] = {LOW, LOW, LOW, LOW, LOW};
int lastButtonState[5] = {LOW, LOW, LOW, LOW, LOW};
unsigned long lastDebounceTime[5] = {0, 0, 0, 0, 0};
const long debounceDelay = 50;

String controls[] = {"/relay1", "/relay2", "/relay3", "/relay4", "/relay5", "/status", "/reset"};
int count = sizeof(controls) / sizeof(controls[0]);

void S_resetSettings()
{
  // Membuat objek WiFiManager
  WiFiManager wifiManager;

  // Memutuskan koneksi Wi-Fi yang aktif
  WiFi.disconnect();

  // Menghapus data SSID dan password dari memori EEPROM
  for (int i = 0; i < SSID_LENGTH; i++)
  {
    EEPROM.write(i, 0);
    EEPROM.write(i, 1);
    EEPROM.write(i, 2);
  }
  for (int i = SSID_LENGTH; i < SSID_LENGTH + PASS_LENGTH; i++)
  {
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

    for (int i = 0; i < (count - 2); i++)
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
      for (int i = 0; i < (count - 2); i++)
      {
        message += controls[i] + " " + String(Statusled[i] ? "ON" : "OFF") + "\n";
      }
      bot.sendMessage(chat_id, message.c_str(), "");
    }
    else if (text == "/reset")
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

void tombol(){
     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    // while (numNewMessages)
    // {
    //   Serial.println("got response");
    if (numNewMessages>0)
    {
      handleNewMessages(numNewMessages);
    }
    
  //     numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  //   }
  //   lastTimeBotRan = millis();
  // static unsigned long lastCheck = 0;
  // if (millis() - lastCheck > 5000)
  // {
  //   lastCheck = millis();
  //   if (WiFi.status() != WL_CONNECTED)
  //   {
  //     Serial.println("Koneksi WiFi terputussssss!");
  //     WiFi.begin(ssid.c_str(), password.c_str());
  //   }
  //   else
  //   {
  //     Serial.println("Koneksi WiFi Tersambung!");
  //   }
  // }
}

void setup()
{
  Serial.begin(115200);
  // pinMode(BUTTON_PIN, INPUT_PULLUP);

  for (int i = 0; i < (count-2); i++)
  {
    pinMode(Pinled[i], OUTPUT);
    pinMode(PinButton[i], INPUT);
    // digitalWrite(Pinled[i], Statusled[i]);
    // digitalWrite(PinButton[i], Statusled[i]);
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
  for (int i = 0; i < 5; i++) {
    reading = digitalRead(PinButton[i]);
    if (reading == 1)
    {
      cont++;
      delay(300);
      if (cont == 1)
      {
          digitalWrite(Pinled[0], HIGH);
      }
      if (cont == 2)
      {
          digitalWrite(Pinled[0], LOW);
          cont=0;
      }
      
    }
    
    // if (reading != lastButtonState[i]) {
    //   Serial.println(reading);
    //   lastDebounceTime[i] = millis();
    // }
    // if ((millis() - lastDebounceTime[i]) > debounceDelay) {
    //   if (reading != buttonState[i]) {
    //     buttonState[i] = reading;
    //     if (buttonState[i] == HIGH) {
    //       Statusled[i] = !Statusled[i];
    //       digitalWrite(Pinled[i], Statusled[i]);
    //     }
    //   }
    // }
    // lastButtonState[i] = reading;
  }
        if (millis() > lastTimeBotRan + botRequestDelay)
  {
   tombol();
  }

  // int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
// memproses pesan yang masuk


// if (numNewMessages > 0)
// {
//   /* code */
//       handleNewMessages(numNewMessages);
// }



  //   Serial.println("numNewMessages : " + numNewMessages);
  // for (int i=0; i<numNewMessages; i++) {
  //   String chat_id = String(bot.messages[i].chat_id);
  //   String text = bot.messages[i].text;
  //   String from_name = bot.messages[i].from_name;

  //   Serial.println("Chat ID: " + chat_id);
  //   Serial.println("From: " + from_name);
  //   Serial.println("Message: " + text);

  //   // membalas pesan yang masuk
  //   if (text == "/hello") {
  //     bot.sendMessage(chat_id, "Hello " + from_name);
  //   } else if (text == "/time") {
  //     bot.sendMessage(chat_id, "The time is " + String(millis()/1000) + " seconds");
  //   } else {
  //     bot.sendMessage(chat_id, "I don't understand");
  //   }
  // }

  // memberikan jeda waktu sebelum membaca pesan berikutnya
  // delay(1000);
  
// for (int i = 0; i < 8; i++) {
//     buttonState[i] = digitalRead(PinButton[i]);
//   }
//   for (int i = 0; i < 8; i++) {
//     if ( buttonState[i] == LOW) {
//       // Jika tombol telah ditekan dan dilepaskan, ubah status lampu
//       Statusled[i] = !Statusled[i];
//       digitalWrite(Pinled[i], Statusled[i]);
//     }
//     Statusled[i] = buttonState[i];
//   }
  // Membaca nilai input dari tombol
//  buttonState = digitalRead(PinButton[7]);
  // Jika tombol ditekan, LED akan menyala
  // if (buttonState == LOW)
  // {
  //   tombol();
  //   // Statusled[0] = !Statusled[0];
  //   // digitalWrite(Pinled[0], Statusled[0]);
  //   // delay(100);
  // }


  // Serial.println("BotToken: " + botToken + ", chatID: " + chatID);
  // delay(3000);
}

