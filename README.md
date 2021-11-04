# esp32-firmware-update
Update ESP32 firmware over WiFi from Github

This includes a python script which generates the update json file based on the firmware version stated in the Arduino sketch.
The script also renames the generated binary file from the Arduino IDE.


## Usage
if you clone this repository
make the following changes

./.github/workflows/main.yml
```
                git config --local user.email "your@email.com"
                git config --local user.name "username"
```

./esp32-firmware-update.ino

```
String baseUrl = "https://raw.githubusercontent.com/{your github username}/esp32-firmware-update/main/";

/* your wifi credentials */
char * WIFI_SSID = "wifi name";
char * WIFI_PASS = "wifi password";
```