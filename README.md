# Weather-station

## The device and files

Requires a device, which consists of Arduino Nano, BME280 sensor, SSD1306 128x64 monochrome OLED screen and a button. All the components should be connected in the following scheme:

![scheme](https://user-images.githubusercontent.com/35459417/143701665-9917ab3d-5fb5-4612-8dd2-22efb2575f12.png)

All the necessary Arduino libraries are located in the External libraries folder.

The source file is located in the Weather folder.

The sketch should be loaded into the microcontroller via Arduino IDE.

## Usage

To display correct weather prediction and graphs, the weather station should be always powered on.

Weather can be predicted with the current pressure change. If the value is lower than -50 Pa/hr, then the lower the value, the greater the probability of overcast/rain/storm with higher humidity in the next couple of hours. If the value is higher than 50 Pa/hr, then the higher the value, the greater the probability of clear sunny weather with lower humidity.

The weather station has 4 modes, which can be switched with the button press:
- 1 mode. Displays current pressure change, weather prediction, temperature, humidity and pressure; updates once every minute.
- 2 mode. Displays current pressure change, weather prediction, temperature, humidity, pressure and height relatively to the sea level; updates twice a second.
- 3 mode. Displays pressure graph during the last 10 hours; updates once every 4 minutes.
- 4 mode. Displays temperature graph during the last 10 hours; updates once every 4 minutes.

## Implementation

In case the power is lost, the sketch saves all the data into EEPROM (non-volatile memory) and loads it from the EEPROM on start, but doesn't update it when the power is off.

To calculate accurate pressure change, the sketch measures pressure every 4 minutes, stores pressure values during the last 2 hours and uses [the method of least squares](https://en.m.wikipedia.org/wiki/Least_squares) to get an accurate result.

## Examples

1 mode:
![1 mode](https://user-images.githubusercontent.com/35459417/143704219-b18a0289-6cdf-4bad-a31d-fa2a554b75d0.jpg)

2 mode:
![2 mode](https://user-images.githubusercontent.com/35459417/143704203-00790f73-3546-4138-afbf-9b2fb2e56509.jpg)

3 mode:
![3 mode](https://user-images.githubusercontent.com/35459417/143704191-253bfc1c-741e-4f6b-a628-c08166ccccdf.jpg)

4 mode:
![4 mode](https://user-images.githubusercontent.com/35459417/143704166-49fbae28-1a7d-4dd8-9ff6-d025b4486fbd.jpg)

[Video example](https://drive.google.com/file/d/1AsLBO8nUu0zL_WNKnmUEWsLBj5RbZkd4/view?usp=sharing)

In the pictures and the video the sketch is not the latest version, in the newer version it also displays storm/rain/same/clear as the weather prediction.
