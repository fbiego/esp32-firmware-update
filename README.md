# esp32-firmware-update
Update ESP32 firmware over WiFi from Github

[![Versioner](https://github.com/fbiego/esp32-firmware-update/actions/workflows/main.yml/badge.svg)](https://github.com/fbiego/esp32-firmware-update/actions/workflows/main.yml)

This includes a python script which generates the update json file based on the firmware version stated in the Arduino sketch.
The script also renames the generated binary file from the Arduino IDE.

- update your code
- export compiled binary
- commit & push to github
- the rest is history

## Setup
If you clone this repository, make the following changes

[`workflow`](./.github/workflows/main.yml)
```
git config --local user.email "your@email.com"
git config --local user.name "username"
```

[`sketch`](./esp32-firmware-update.ino)

```
String baseUrl = "https://raw.githubusercontent.com/{your github username}/esp32-firmware-update/main/";

/* your wifi credentials */
char * WIFI_SSID = "wifi name";
char * WIFI_PASS = "wifi password";
```

## Releasing Updates
- Clone the repo and make changes to the code using Arduino IDE
- Upload the sketch to the ESP32 if this is the first time
- Increment the `currentVersion` in the sketch
- Export compiled binary (Ctrl+Alt+S) using Arduino IDE
- Commit and push the changes to Github
- The workflow will run the script to generate json file and rename the bin file
- Once the ESP32 finds higher `versionCode` in the json file it will download and update the firmware
