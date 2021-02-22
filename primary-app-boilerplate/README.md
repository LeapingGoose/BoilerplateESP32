# Boilerplate For ESP32


### You *must* create a secret.h file to provide desired usernames, passwords, etc...
See "/lib/secret/README.md" for more details.


### This boiler plate provides the following:
- Web interface via Access Point WiFi server and Local WiFi server
- OTA (Over The Air) updating via the web interface.
- Structured 'Task Controller' running on the same timer for ease of use.
- File system partition via SPIFFs.


### To Do
- Add Factory app for fail safe firmware updating.
- Improve visual appeal of the web interface.
- Remove unused code and libraries.
- Refactor (always)