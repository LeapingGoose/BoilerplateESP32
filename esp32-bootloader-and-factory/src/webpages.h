// LEFT OFF HERE> Change status color and pad
// LEFT OFF HERE> Fix firmware uploadß

#ifndef WEBPAGES_H
#define WEBPAGES_H

#include "admin-html.h"

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<body>
  <p><a href="/">Log Back In</a></p>
</body>
</html>
)rawliteral";

// reboot.html base upon https://gist.github.com/Joel-James/62d98e8cb3a1b6b05102
const char reboot_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
</head>
<body>
<h3>
  Rebooting, returning to main Administration page in <span id="countdown">20</span> seconds
</h3>
<script type="text/javascript">
  var seconds = 20;
  function countdown() {
    seconds = seconds - 1;
    if (seconds < 0) {
      window.location = "/";
    } else {
      document.getElementById("countdown").innerHTML = seconds;
      window.setTimeout("countdown()", 1000);
    }
  }
  countdown();
</script>
</body>
</html>
)rawliteral";

const char _ota_success_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
</head>
<body>
<h3>The firmware has been updated!</h3>
<h4>Reboot will be completed in...</h4>
<h2><span id="countdown">15</span> seconds</h2>
<script type="text/javascript">
  var seconds = 15;
  function countdown() {
    seconds = seconds - 1;
    if (seconds < 0) {
      window.location = "/";
    } else {
      document.getElementById("countdown").innerHTML = seconds;
      window.setTimeout("countdown()", 1000);
    }
  }
  countdown();
</script>
</body>
</html>
)rawliteral";


const char _ota_fail_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
</head>
<body>
<h3>
  Sorry, the firmware update has failed.
</h3>
<p><a href="/">Go back to Settings</a></p>
</body>
</html>
)rawliteral";


#endif