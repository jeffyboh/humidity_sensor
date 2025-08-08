# Humidity Sensor

Sensor project that collects humidity levels and publishes them to HomeAssistant using MQTT

## Description

This is an ESP32 based sensor that uses inexpensive parts to make a humidity sensor that publishes to Home Assistant using MQTT. The results can then be scripted 
to do various things such as:
* Turn on a bathroom exhaust fan if the sensor is used in a bathroom
* Turn on a dehumidifier
* Send alerts
* Whatever action you can imagine

## Getting Started

### Dependencies
TODO - List library dependencies used

You will need to create a secrets.h that conains the following.
```cpp
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define MQTT_SERVER     "YOUR_MQTT_SERVER_IP"
#define MQTT_PORT       1883
#define MQTT_USERNAME   "YOUR_MQTT_USERNAME"
#define MQTT_PASSWORD   "YOUR_MQTT_PASSWORD"
```

### Installing


### Executing program

```
code blocks for commands
```

## Authors


## Version History


## License


## Acknowledgments

