# Rotator
**Simple** antenna rotator for amateur radio use. This software can be used for controlling DC rotators with linear potentiometer as a feedback. 
Designed for Radioclub Omega ([OM3KFF](https://www.om3kff.sk))

> [!NOTE]
> This project is work in progress

### GUI
![Screenshot of GUI.](GUI.png)

# TO-DO
- [ ] Show number of connected users,
- [ ] Second axis for satellite work,
- [ ] Multiple rotator autofind support,
- [x] User lock controls,
- [x] End-stop calibration,
- [x] User-friendly initial setup
- [x] Settings page
- [x] Settings saving
- [x] HW implementation
- [x] PID controller
- [x] Lowpass filter on potentiometer

# Credits
- Frontend:
  - [ericek111](https://github.com/ericek111)
  - [Lopastudio](https://github.com/Lopastudio)
- Firmware:
  - [AdaSzi](https://github.com/AdaSzi)

> [!CAUTION]
> Before using with a rotator, make sure that `DEBUG` flag is commented out in [platformio.ini](RotatorFW/platformio.ini). This disables all Serial.* because they are blocking on ESP32 C3.

