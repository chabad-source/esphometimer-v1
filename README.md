[![Banner](https://github.com/chabad-source/esphometimer/blob/main/images/banner.png)](https://github.com/chabad-source/esphometimer)

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f17caa6e3d2946378de9beae9fc0ffe8)](https://www.codacy.com/gh/chabad-source/melachaplug/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=chabad-source/melachaplug&amp;utm_campaign=Badge_Grade)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate/?hosted_button_id=Q9A7HG8NQEJRU)


# ESPHome Timer
Adds dynamic timer functionality to your esphome project. Set and adjust timers on the fly!

This project is using [ESPHome](https://esphome.io/).

## Features
-   [x] Features 20 updatable timers.
-   [x] Set repeatable timers based on the day of the week.
-   [x] Auto detect missed timers and restore the latest timers value.
-   [x] Includes quick override option.
-   [x] All running locally, no reliance on another server.

![Web Server Screenshot](https://github.com/chabad-source/esphometimer/blob/main/images/Screenshot%202023-11-27%20211702.png)

## Instructions

-   Copy the code and edit to your ESPs specifications.
-   Copy the ids of the relays you have and add them into the top of the timer.h file. Be sure to change the number of relays if necessary.
-   Be sure to include your location and the correct time zone for your area.

*Usage Instructions*
-   Mode 0 uses the time as the base. Mode 1 uses sunrise, Mode 2 uses sunset.
-   Action sets the action to do when timer runs. 0 = off 1 = on and 2 = toggle.
-   Output is the position of your relay, if you only have 1 relay use 0 (with two relays the first would be 0 and the second 1).
-   When using an offset, set "Timer Negative Offset" to on to make the offset be applied to before sunrise or sunset.
-   If "Timer Repeat" is disabled the timer will only run once no matter if todays weekday is toggled.
-   After running once the timer will automatically disable it self.
  
## Contribute 

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=Q9A7HG8NQEJRU) - or - [!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/rebbepod)
