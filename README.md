# Arduino-Zabbix-Agent

Using an Arduino Uno v3 and a Zabbix Server to monitor a data center environment.

It's a Zabbix Agent tested and running successfully.

* [Materials](#materials)
* [Schematic](#schematic)
* [More info](#more-info)
  * [DHT22](#dht22)
  * [Observations](#observation)
* [How to use](#how-to-use)
* [Keys used on Zabbix](#keys-used-on-zabbix)
* [On the code](#on-the-code)
* [Example](#example)



## Materials:

- Arduino Uno v3
- Ethernet Shield
- Arduino Sensor Shield*
- DHT22 (or similar) and one 10k resistor

###### *A new shield may be built by yourself.

## Schematic:
Connect DHT22 VCC pin to Arduino 3.3V, DHT22 GND pin to Arduino GND
and DHT Data pin to Arduino pin 5.

### More info:

#### `DHT22`
This sensor can measure the air temperature and humidity. It doesn't need to be calibrated. It is recommended to take 1 sample per second or less. It returns one `int` for each data. The temperature is in Celsius. A 10k resistor must be connected to pins Vcc and Signal.

### Observations:
All temperature are in Celsius.
To convert to Fahrenheit:
```
Temp F = 1.8*(Temp C) + 32
```

## How to use:

Check pins used.
Upload the code to the Arduino.
Add an item on Zabbix Server.
 - Type: Zabbix agent.
 - Key: The key for each sensor (`q`, `w`, `e`, `r`, `t` etc).
 - It is recommended to check each item on not less than once a minute.

## Keys used on Zabbix:

* q - soil humidity
* w - air temperature on DHT11
* e - air humidity on DHT11

## On the code:

Set the IP address, the gateway and the MAC address to be used by the agent.

Check how many sensors will be used and add them to the code if necessary. To do this:
* Set a pin using `#define`.
* Write a function to read the sensor if necessary.
* Choose a letter to be used as a command for each sensor function. Make sure the letter is not used anywhere else!
* On `parseCommand`, add the function to be executed when receiving the command defined on the step before.

__*Pins 10, 11, 12 and 13 cannot be used. They are used by the ethernet shield*.__

Pin 5 is reserved for the DHT22.

