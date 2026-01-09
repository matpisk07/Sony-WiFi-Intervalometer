#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>
#include <DNSServer.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const char* ssid_ap = "SonyTimelapseAP";
const char* password_ap = "ESP32@timelapse#Mattia";

const char* RX100M2_SSID = "DIRECT-HqC1:DSC-RX100M2";
const char* RX100M2_PSWD = "v9aTJRE7";

WebServer server(80);
DNSServer dnsServer;
WiFiClient client;

touch_value_t threshold = 100000;
const uint8_t TOUCH_PIN = A0;

const String host = "10.0.0.1";
const String http_port = "10000";
const String url = "/camera";

float timelapse_span;
int shot_period;
int shot_period_ms;
int picture_number = 0;
int op_mode = 0;

int time_perso = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

char setShootMode[] = "{\"method\":\"setShootMode\",\"params\":[\"still\"],\"id\":1,\"version\":\"1.0\"}";
char actTakePicture[] = "{\"method\":\"actTakePicture\",\"params\":[],\"id\":1,\"version\":\"1.0\"}";
char setFocusMode[] = "{\"method\":\"setFocusMode\",\"params\":[\"AF-S\"],\"id\":1,\"version\":\"1.0\"}";
char setAutoPowerOff[] = "{\"method\":\"setAutoPowerOff\",\"params\":[{\"autoPowerOff\":60}],\"id\":1,\"version\":\"1.0\"}";
char beepOn[] = "{\"method\": \"setBeepMode\",\"params\": [\"On\"],\"id\": 1,\"version\": \"1.0\"}";
char beepOff[] = "{\"method\": \"setBeepMode\",\"params\": [\"Off\"],\"id\": 1,\"version\": \"1.0\"}";

void httpPost(char* j_request);
void handleRoot();
void handleSave();

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>📷</text></svg>">
    <meta charset="UTF-8">
    <meta content="width=device-width, initial-scale=1" name="viewport">
    <title>Sony Camera Remote - Timelapse</title>
    <style>
    body {background: #36393e; color: #ffffff; font-family: sans-serif; margin-left: 10px; margin-right: 10px; text-align: center;}
    h2 { font-size: 2.0rem; text-align: center; }
    p { font-size: 1.0rem; line-height: 1.4; text-align: center;  width: 900px; margin: 0 auto;}
    a {color: #bfbfbf; text-decoration: none;}
    input { font-size: 1.0rem; text-align: center}
    fieldset { display: block; margin-left: 10px; margin-right: 10px; border:none}
    .labels {font-size: 1.0rem; vertical-align:middle; padding-bottom: 15px;}
    .button {background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
    </style>
</head>
<body>
<h2>Sony Camera Remote - Timelapse</h2>
<p>Saisissez la durée du timelapse en minutes et le temps entre chaque photo en secondes.
<br>Une fois les paramètres enregistrés, il faudra juste toucher le pin A0.</p>
<form action="/save" method="POST">
    <fieldset>
        <span class="labels">Durée du timelapse:</span><br>
        <input type="number" name="time" step="0.1" placeholder="en minutes"><br><br>
        <span class="labels">Temps entre chaque photo:</span><br>
        <input type="number" name="period" min="1" placeholder="en secondes"><br><br>
    </fieldset>
    <button class="button" data-translate="save" id="save" onclick="save">Enregistrer</button>
</form>
    <br>
    <p>2024 | Mattia PISCHEDDA | v1.1</p>
</body>
</html>)rawliteral";

const char save_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>📷</text></svg>">
    <meta charset="UTF-8">
    <meta content="width=device-width, initial-scale=1" name="viewport">
    <meta content="Sony Camera Remote - Timelapse" name="description">
    <title>Sony Camera Remote - Timelapse</title>
    <style>
    body {background: #36393e; color: #ffffff; font-family: sans-serif; margin-left: 10px; margin-right: 10px; text-align: center;}
    h2 { font-size: 2.0rem; text-align: center;}
    p { font-size: 1.0rem;}
    a {color: #bfbfbf; text-decoration: none;}
    .labels {font-size: 1.0rem; vertical-align:middle; padding-bottom: 15px;}
    }
    </style>
</head>
<body>
<h2>Sony Camera Remote - Timelapse</h2>
<p>Paramètres enregistrés avec succès !</p>
<p>Touchez le pin A0 pour commencer le timelapse</p>
<p>2024 | Mattia PISCHEDDA | v1.1<br>
</body>
</html>)rawliteral";

void setup() {
  pinMode(TOUCH_PIN, GPIO_MODE_INPUT);
  lcd.begin(16, 2);
  lcd.clear();
  Serial.begin(115200);
  Serial.println();

  Serial.print("Setting AP: ");
  Serial.print(ssid_ap);
  Serial.print("Password");
  Serial.println(password_ap);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid_ap, password_ap);
  WiFi.begin(RX100M2_SSID, RX100M2_PSWD);

  dnsServer.start(53, "*", WiFi.softAPIP());

  lcd.setCursor(0, 0);
  lcd.print("   En attente   ");
  lcd.setCursor(0, 1);
  lcd.print("   Du RX100M2   ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("AP MAC address: ");
  Serial.println(WiFi.softAPmacAddress());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  switch (op_mode) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("   Config Web   ");
      lcd.setCursor(0, 1);
      lcd.print("    En Cours    ");
      while (picture_number == 0) {
        dnsServer.processNextRequest();
        server.handleClient();
      }
      op_mode = 1;
      lcd.clear();
      Serial.println();
      break;
    case 1:
      httpPost(setShootMode);
      Serial.print("Number of pictures: ");
      Serial.println(picture_number);
      Serial.print("Shot period: ");
      Serial.print(shot_period_ms / 1000);
      Serial.println(" seconds");
      op_mode = 2;
      httpPost(beepOn);
      break;
    case 2:
      Serial.println("Press start button");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   Touche  A0   ");
      lcd.setCursor(0, 1);
      lcd.print(" Pour Commencer ");
      while (touchRead(TOUCH_PIN) < threshold) {
        delay(100);
      }
      op_mode = 3;
      Serial.println("Capture started");
    case 3:
      Serial.println("Single AF Mode");
      httpPost(setFocusMode);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("000");
      lcd.setCursor(3, 0);
      lcd.print("1");
      lcd.setCursor(4, 0);
      lcd.print("/");
      lcd.setCursor(5, 0);
      lcd.print(picture_number);

      lcd.setCursor(2, 1);
      lcd.print("h");
      lcd.setCursor(5, 1);
      lcd.print("min");
      lcd.setCursor(10, 1);
      lcd.print("s");

      time_perso = picture_number * shot_period;
      hours = time_perso / 3600;
      minutes = (time_perso % 3600) / 60;
      seconds = time_perso % 60;
      if (hours < 10) {
        lcd.setCursor(0, 1);
        lcd.print("0");
        lcd.setCursor(1, 1);
        lcd.print(hours);
      } else {
        lcd.setCursor(0, 1);
        lcd.print(hours);
      }
      if (minutes < 10) {
        lcd.setCursor(3, 1);
        lcd.print("0");
        lcd.setCursor(4, 1);
        lcd.print(minutes);
      } else {
        lcd.setCursor(3, 1);
        lcd.print(minutes);
      }
      if (seconds < 10) {
        lcd.setCursor(8, 1);
        lcd.print("0");
        lcd.setCursor(9, 1);
        lcd.print(seconds);
      } else {
        lcd.setCursor(8, 1);
        lcd.print(seconds);
      }

      for (int i = 0; i < picture_number; i++) {
        Serial.print("Picture ");
        Serial.println(i + 1);
        if ((i + 1) >= 1000) {
          lcd.setCursor(0, 0);
          lcd.print(i + 1);
        } else if ((i + 1) < 1000 && (i + 1) >= 100) {
          lcd.setCursor(0, 0);
          lcd.print("0");
          lcd.setCursor(1, 0);
          lcd.print(i + 1);
        } else if ((i + 1) < 100 && (i + 1) >= 10) {
          lcd.setCursor(0, 0);
          lcd.print("00");
          lcd.setCursor(2, 0);
          lcd.print(i + 1);
        } else if ((i + 1) < 10) {
          lcd.setCursor(0, 0);
          lcd.print("000");
          lcd.setCursor(3, 0);
          lcd.print(i + 1);
        }
        time_perso = (picture_number - (i + 1)) * shot_period;
        hours = time_perso / 3600;
        minutes = (time_perso % 3600) / 60;
        seconds = time_perso % 60;
        if (hours < 10) {
          lcd.setCursor(0, 1);
          lcd.print("0");
          lcd.setCursor(1, 1);
          lcd.print(hours);
        } else {
          lcd.setCursor(0, 1);
          lcd.print(hours);
        }
        if (minutes < 10) {
          lcd.setCursor(3, 1);
          lcd.print("0");
          lcd.setCursor(4, 1);
          lcd.print(minutes);
        } else {
          lcd.setCursor(3, 1);
          lcd.print(minutes);
        }
        if (seconds < 10) {
          lcd.setCursor(8, 1);
          lcd.print("0");
          lcd.setCursor(9, 1);
          lcd.print(seconds);
        } else {
          lcd.setCursor(8, 1);
          lcd.print(seconds);
        }
        httpPost(actTakePicture);
        httpPost(beepOn);
        if (i < picture_number - 1) {
          delay(shot_period_ms);
        }
      }
      op_mode = 4;
      Serial.println("Capture finished");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  Timelapse OK  ");
      lcd.setCursor(0, 1);
      lcd.print("  Timelapse OK  ");
      httpPost(setAutoPowerOff);
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      break;
    case 4:
      delay(100);
      while (touchRead(TOUCH_PIN) < threshold) {
        delay(100);
      }
      esp_restart();
      break;
  }
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleSave() {
  if (!server.hasArg("time") || !server.hasArg("period")
      || server.arg("time") == NULL || server.arg("period") == NULL) {
    server.send(400, "text/plain", "400: Invalid Request");
    return;
  }
  if (server.hasArg("time") && server.hasArg("period")
      && server.arg("time") != NULL && server.arg("period") != NULL) {
    server.send(200, "text/html", save_html);
    timelapse_span = server.arg("time").toFloat();
    shot_period = server.arg("period").toInt();
    shot_period_ms = shot_period * 1000;
    picture_number = round((timelapse_span * 60) / shot_period);
  } else {
    server.send(401, "text/plain", "401: Unauthorized");
  }
}

void httpPost(char* j_request) {
  String server_name = "http://" + host + ":" + http_port + url;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, server_name);
    http.addHeader("Content-Type", "application/json");
    int http_responde_code = http.POST(j_request);
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    lcd.setCursor(0, 0);
    lcd.print("  Sony RX100M2  ");
    lcd.setCursor(0, 1);
    lcd.print("   Deconnecte   ");
    while (touchRead(TOUCH_PIN) < threshold) {
      delay(100);
    }
    esp_restart();
  }
}