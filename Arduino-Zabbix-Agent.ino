/******************************************************************************
   Purpose: Provide a way Zabbix access Temperature and Humidity.
   Git: https://github.com/ramiroluz/Arduino-Zabbix-Agent
   Author: Ramiro Batista da Luz <ramiroluz@gmail.com>
   Adapted from Diego W. Antunes, Gabriel Ferreira, Marco Rougeth and Marco Fischer

   Circuit:
    - Arduino Uno + Ethernet shield uses pins 4, 10, 11, 12, 13
    - DS18B20 DHT22 sensor use pin 2, VCC and GND

   Required external libraries:
     - DallasTemperature Arduino Library https://www.milesburton.com/Dallas_Temperature_Control_Library
 *****************************************************************************/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ethernet.h>
#include <SPI.h>
#include <string.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#ifndef SERIAL_DEBUG
//#define SERIAL_DEBUG
#endif

#ifndef DHCP_ON
//#define DHCP_ON
#endif

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x5D, 0x05 };
IPAddress ip_host(192, 168, 2, 2);
IPAddress ip_gw(192, 168, 2, 0);
IPAddress subnet(255, 255, 255, 0);
EthernetServer g_server(10050);
EthernetClient g_client;

// Data wire is plugged into port 3 on the Arduino
#define ONE_WIRE_BUS 3

boolean cli_connected = false;  // Controll if client is connected
double dht_temp = 0;            // Temperature (Celsius)
String cmd;                     // Zabbix command
int8_t eth_maintain = -1;       // Ethernet dhcp variable
const uint8_t MAX_CMD_LENGTH = 25; // Max command message
const uint16_t READ_TIME = 10000; // Time between readings (milliseconds)
uint16_t wdt_state = 0x0000;    // Watchdog control
unsigned long last_check = 0; // Controls last read time

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// Store last sensor temperature.
float tempC;


/************************************************************************************
   Description: Setup function, config MAC, IP, initiate server and watchdog.
   Notes:
 ************************************************************************************/
void setup()
{
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.getResolution(insideThermometer);
  
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  Serial.println("Setup");
#endif

  // Set watchdog to 8 seconds.
  watchdog_setup();

#ifdef DHCP_ON
  int dhcp_return = 0;
  const uint8_t max_num_dhcp_tries = 5;
  uint8_t num_dhcp_tries = 0;
  bool f_dhcp_done = false;

  do {
    dhcp_return = Ethernet.begin(mac);

    if (dhcp_return == 1) {
      f_dhcp_done = true;
#ifdef SERIAL_DEBUG
      Serial.println(Ethernet.localIP());
#endif
    } else {
      // Too bad, DHCP server did not provided a valid IP address.
      delay(5000);
#ifdef SERIAL_DEBUG
      Serial.println(num_dhcp_tries);
#endif
    }

    num_dhcp_tries++;
    wdt_reset();
  } while ((num_dhcp_tries < 5) && (f_dhcp_done == false));

  // If could not get a IP from dhcp server reset.
  if ((num_dhcp_tries >= 5) && (f_dhcp_done == false)) {
    halt();
  }

#else
  Ethernet.begin(mac, ip_host, ip_gw, subnet);
#endif

  g_server.begin();
}

/************************************************************************************
   Description: Read data, check WDT and manage client connection.
   Notes:
 ************************************************************************************/
void loop()
{
  wdt_state = 0x5555;
  wdt_a();

  get_data(insideThermometer);
  g_client = g_server.available();

#ifdef DHCP_ON
  eth_maintain = Ethernet.maintain();
#endif

  if (g_client) {
    if (!cli_connected) {
      cli_connected = true;
    }

    int to_read = g_client.available();
    if (to_read > 0) {
      read_stream(g_client.read());
    }
  }


  delay(10);
  wdt_state += 0x2222;
  wdt_b();
}

/************************************************************************************
   Description: Read received command.
   Notes:
 ************************************************************************************/
static void read_stream(const char c)
{
  if (cmd.length() == MAX_CMD_LENGTH) {
    cmd = "";
  }

  cmd += c;

  if (c == 10 || c == 13) {
    if (cmd.length() >= 2) {
#ifdef SERIAL_DEBUG
      for (int i = 0; i < cmd.length(); ++i) {
        g_server.println(cmd.charAt(i), DEC);
      }
#endif
      cmd = cmd.substring(0, cmd.length() - 1);
      parse_command();
    }
    cmd = "";
  }
}

/************************************************************************************
   Description: Parse received command.
   Notes:
 ************************************************************************************/
void parse_command(void)
{
#ifdef SERIAL_DEBUG
  g_server.println(cmd);
  g_server.println(cmd.length());
#endif

  if (cmd.equals("agent.ping")) {
    g_server.println("pong");
#ifdef SERIAL_DEBUG
    Serial.println("pong");
#endif
  } else if (cmd.equals("agent.version")) {
    g_server.println("Arduino Zabbix Agent 0.1");
#ifdef SERIAL_DEBUG
    Serial.println("Arduino Zabbix Agent 0.1");
#endif
  } else if (cmd.equals("agent.temp")) {
    g_server.println(tempC);
#ifdef SERIAL_DEBUG
    Serial.println(tempC);
#endif
  } else if (cmd.equals("agent.ethernet")) {
    g_server.println(eth_maintain);
#ifdef SERIAL_DEBUG
    Serial.println(eth_maintain);
#endif
  } else if (cmd.equals("agent.close")) {
    g_server.println("closed");
#ifdef SERIAL_DEBUG
    Serial.println("closed");
#endif
    g_client.stop();
    cli_connected = false;
  } else { // Agent error
    g_server.println("Invalid command");
  }

    g_client.stop();
  cli_connected = false;
}

/************************************************************************************
   Description: Read sensor data.
   Notes:
 ************************************************************************************/
float get_data(DeviceAddress deviceAddress)
{
  if (millis() - last_check > READ_TIME) {
#ifdef SERIAL_DEBUG
    Serial.println("read new sensor data");
#endif
    sensors.requestTemperatures();
    tempC = sensors.getTempC(deviceAddress);

    if (isnan(tempC)) {
#ifdef SERIAL_DEBUG
      Serial.println("Error reading temperature!");
#endif
    } else {
#ifdef SERIAL_DEBUG
      Serial.print("Temperature: ");
      Serial.print(tempC);
      Serial.println(" *C");
#endif
    }

    last_check = millis();
  }
}

/************************************************************************************
   Description: Configure watchdog, enable interrupts and system reset.
   Notes: Watchdog is set to 8 seconds.
 ************************************************************************************/
void watchdog_setup(void)
{
  // Disable all interrupts.
  cli();

  // Reset watchdog timer.
  wdt_reset();
  /*
    WDTCSR configuration:
    WDIE = 1 :Interrupt Enable
    WDE  = 1 :Reset Enable
    See table for time-out variations:
    WDP3 = 1 :For 8000ms Time-out
    WDP2 = 0 :For 8000ms Time-out
    WDP1 = 0 :For 8000ms Time-out
    WDP0 = 1 :For 8000ms Time-out
  */
  // Configuro os bits para poder modificar a configuracao de tempo do watchdog
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // Setup watchdog bits
  WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0);

  // Reenable system interrupts.
  sei();
}

/************************************************************************************
   Description: Check that system state is still consistent.
   Notes: If state condition is not ok, halts system.
 ************************************************************************************/
void wdt_a(void)
{
  if (wdt_state != 0x5555)
    halt();

  // Pat the dog otherwise it can bite.
  wdt_reset();
  wdt_state += 0x1111;
}

/************************************************************************************
   Description: Check that system state is still consistent.
   Notes: If state condition is not ok, halts system.
 ************************************************************************************/
void wdt_b(void)
{
  if (wdt_state != 0x8888)
    halt();

  // Pat the dog otherwise it can bite.
  wdt_reset();

  if (wdt_state != 0x8888)
    halt();

  wdt_state = 0;
}

/************************************************************************************
   Description: Function called when something goes wrong with watchdog.
   Notes:
 ************************************************************************************/
void halt(void)
{
  // Stays here waiting system to reset to avoid more trouble.
  while (1);
}
