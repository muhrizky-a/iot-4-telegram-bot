#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>

// Wifi network station credentials
#define WIFI_SSID "SSID_WIFI"
#define WIFI_PASSWORD "PASSWORD_WIFI"

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "GET_IT_FROM_BOTFATHER"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

// Sensor Pins
#define DHTPIN D5
#define LEDPIN D6
#define DHTTYPE DHT11 //Mengatur TYPE DHT (Karena ada 2 jenis [DHT11 & DHT22])
int ledStatus = 0;

DHT dht(DHTPIN, DHTTYPE);

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (text == "/ledon") {
      digitalWrite(LEDPIN, HIGH); // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      bot.sendMessage(chat_id, "Led is ON", "");
    }

    if (text == "/ledoff") {
      ledStatus = 0;
      digitalWrite(LEDPIN, LOW); // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Led is OFF", "");
    }

    if (text == "/ledstatus") {
      String message = "LED Status: ";
      if(ledStatus == 1) {
        message += "ON";
      } else {
        message += "OFF";
      }
      
      bot.sendMessage(chat_id, message, "");
    }
    
    if (text == "/dhtstatus") {
      String message = "";
      message += "Suhu : "+ String(t) + "°C\n";
      message += "Kelembapan : "+ String(h) + " %";
      bot.sendMessage(chat_id, message, "");
    }

    if (text == "/start") {
      String welcome = "Welcome to IoT Telegram Bot, " + from_name + ".\n";
      welcome += "List of Commands:\n\n";
      welcome += "/ledon : to switch the Led ON\n";
      welcome += "/ledoff : to switch the Led OFF\n";
      welcome += "/ledstatus : Returns current status of LED\n";
      welcome += "/dhtstatus : Returns temperature and humidity status of DHT11\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  // Sensor Pin Mode
  pinMode(LEDPIN, OUTPUT);
  dht.begin();

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      Serial.println("Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
