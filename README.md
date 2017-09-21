# Arduino-Zabbix-Agent

Using an Arduino Uno v3 and a Zabbix Server to monitor a data center environment.

It's a Zabbix Agent tested and running successfully.

* [Materials](#materials)
* [Schematic](#schematic)
* [More info](#more-info)
  * [DS18B20](#ds18b20)
  * [Observations](#observation)
* [How to use](#how-to-use)
* [Keys used on Zabbix](#keys-used-on-zabbix)
* [On the code](#on-the-code)
* [Example](#example)



## Materials:

- Arduino Uno v3
- Ethernet Shield
- Arduino Sensor Shield*
- DS18B20 sensor and a 4.7 k ohms resistor.

###### *A new shield may be built by yourself.

## Schematic:
Connect DS18B20 VCC pin to Arduino 5V, DS18B20 GND pin to Arduino GND
and DS18B20 Data pin to Arduino pin 2.

### More info:

#### `DS18B20`
The DS18B20 is a so called 1-wire digital temperature sensor. The words “digital” and “1-wire” make this sensor really cool and allows you, with a super simple setup, to read the temperature of one or more sensors. You can even connect multiple devices together, utilizing only one pin on your Arduino.

https://www.tweaking4all.com/hardware/arduino/arduino-ds18b20-temperature-sensor/

## How to use:

Check pins used.
Upload the code to the Arduino.
Add an item on Zabbix Server.
 - Type: Zabbix agent.
 - Key: The key (commands) for each sensor (`agent.temp`, `agent.ping`, etc).
 - It is recommended to check each item on not less than once a minute.

## Keys used on Zabbix:

* agent.temp - Temperature in celsius
* agent.ping - Check to get a pong response

## On the code:

Set the IP address, the gateway and the MAC address to be used by the agent.

Check how many sensors will be used and add them to the code if necessary. To do this:
* Set a pin using `#define`.
* Write a function to read the sensor if necessary.
* Choose a letter to be used as a command for each sensor function. Make sure the letter is not used anywhere else!
* On `parseCommand`, add the function to be executed when receiving the command defined on the step before.

__*Pins 10, 11, 12 and 13 cannot be used. They are used by the ethernet shield*.__

Pin 2 is reserved for the DS18B20.

