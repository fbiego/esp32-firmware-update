#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

WiFiMulti wifiMulti;

/* this info will be read by the python script */
int currentVersion = 2; // increment currentVersion in each release

String baseUrl = "https://raw.githubusercontent.com/fbiego/esp32-firmware-update/main/";
String checkFile = "update.json";
/* end of script data */

int fwVersion = 0;
bool fwCheck = false;
String fwUrl = "", fwName = "";

/* your wifi credentials */
char * WIFI_SSID = "WLAN1-4A6959";
char * WIFI_PASS = "5F310FE2122A5EF";

static void rebootEspWithReason(String reason) {
  Serial.println(reason);
  delay(1000);
  ESP.restart();
}

void performUpdate(Stream &updateSource, size_t updateSize) {
  String result = "";
  if (Update.begin(updateSize)) {
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      Serial.println("Written : " + String(written) + " successfully");
    }
    else {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    result += "Written : " + String(written) + "/" + String(updateSize) + " [" + String((written / updateSize) * 100) + "%] \n";
    if (Update.end()) {
      Serial.println("OTA done!");
      result += "OTA Done: ";
      if (Update.isFinished()) {
        Serial.println("Update successfully completed. Rebooting...");
        result += "Success!\n";
      }
      else {
        Serial.println("Update not finished? Something went wrong!");
        result += "Failed!\n";
      }

    }
    else {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      result += "Error #: " + String(Update.getError());
    }
  }
  else
  {
    Serial.println("Not enough space to begin OTA");
    result += "Not enough space for OTA";
  }
  //http send 'result'
}


void updateFromFS(fs::FS &fs) {
  File updateBin = fs.open("/update.bin");
  if (updateBin) {
    if (updateBin.isDirectory()) {
      Serial.println("Error, update.bin is not a file");
      updateBin.close();
      return;
    }

    size_t updateSize = updateBin.size();

    if (updateSize > 0) {
      Serial.println("Trying to start update");
      performUpdate(updateBin, updateSize);
    }
    else {
      Serial.println("Error, file is empty");
    }

    updateBin.close();

    // when finished remove the binary from spiffs to indicate end of the process
    Serial.println("Removing update file");
    fs.remove("/update.bin");

    rebootEspWithReason("Rebooting to complete OTA update");
  }
  else {
    Serial.println("Could not load update.bin from spiffs root");
  }
}

bool downloadFirmware() {
  HTTPClient http;
  bool stat = false;
  Serial.println(fwUrl);
  File f = SPIFFS.open("/update.bin", "w");
  if (f) {
    http.begin(fwUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Downloading...");
        http.writeToStream(&f);
        stat = true;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    f.close();
  } else {
    Serial.println("failed to open /update.bin");
  }
  http.end();

  return stat;
}

bool checkFirmware() {
  HTTPClient http;
  http.begin(baseUrl + checkFile);
  int httpCode = http.GET();
  bool stat = false;
  String payload = http.getString();
  Serial.println(payload);
  DynamicJsonDocument json(1024);
  deserializeJson(json, payload);
  if (httpCode == HTTP_CODE_OK) {
    fwVersion = json["versionCode"].as<int>();
    fwName = json["fileName"].as<String>();
    fwUrl = baseUrl + fwName;
    if (fwVersion > currentVersion) {
      Serial.println("Firmware update available");
      stat = true;
    } else {
      Serial.println("You have the latest version");
    }
    Serial.print("Version: ");
    Serial.print(fwName);
    Serial.print("\tCode: ");
    Serial.println(fwVersion);
  }
  http.end();

  return stat;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Firmware Updates from Github");
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    rebootEspWithReason("SPIFFS Mount Failed, rebooting...");
  }

  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  /* No need to wait for wifi to connect */
  //  while ((wifiMulti.run() != WL_CONNECTED)) {
  //    Serial.print(".");
  //  }

}

void loop() {

  /* check the firmware once connected */
  if ((wifiMulti.run() == WL_CONNECTED) && !fwCheck) {
    fwCheck = true;
    Serial.println("Wifi connected. Checking for updates");
    if (checkFirmware()) {
      if (SPIFFS.exists("/update.bin")) {
        SPIFFS.remove("/update.bin");
        Serial.println("Removed existing update file");
      }
      if (downloadFirmware()) {
        Serial.println("Download complete");
        updateFromFS(SPIFFS);
      } else {
        Serial.println("Download failed");
      }
    }
  }

}
