# This is to keep sensative data offline and safe.

## You must do the following to use this project:

- Create a file in this folder called "secret.h"
- Copy the code below into the secret.h file.
- Update the values to whatever you like or need.

```c++
 #ifndef SECRET_H
 #define SECRET_H
 
 #define STATION_PASSWORD         "YourWiFiPwd" // Password of your WiFi router to go with  the SSID
 #define STATION_SSID             "YourWiFi"    // SSID of your WiFi router.
 // The local website/domain created by the ESP32.
 // The website can be access by an IP
 // address (192.168.0.4 for AP and dynamic for client) but more easily
 // using the domain provided below.
 // For example, if the value below is "esp32-site" then you would access the ESP32 provided
 // website using http://esp32-site.local in your browers address bar.
 #define DEFAULT_WEBAPP_DOMAIN    "esp32-site"
 #define DEFAULT_WEBAPP_USERNAME  "admin"       // The username to login to the website
 #define DEFAULT_WEBAPP_PASSWORD  "adminadmin"  // The password to login to the website.
 #define DEFAULT_BASE_AP_SSID     "WROOM"       // SSID of the AP (Access Point) server  created on the ESP32.
 #define DEFAULT_BASE_AP_PASSWORD "wroomwroom"  // Password for the AP SSID.
 
 #endif
```