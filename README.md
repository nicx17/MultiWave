# MultiWave
Multi-Cable Tester

## Description
This is a Wirelessly communicating multi-Cable tester Project.
It comprises of 2 main parts encased in 2 different cases. The Receiver and the Transmitter. The receiver being the main processor.
### Receiver
The receiver is the brain of the device. It is called the receiver because it mainly receives and processes the data from the transmitter.
It also sets pins/conductors of the cable either high or low.
To switch it on it has a rocker switch (pre voltage boosting).
Its power source are two lithium-ion batteries connected to 2 separate TP4056 BMS modules. those are then connected in parallel to a dc voltage booster which booster (3.7v → 8v). To charge the batteries the inputs of the BMS modules is connected in parallel to a 6 pin USB-C breakout board configured to supply 15W max. 

There are 3 ports connected to the Arduino. RJ45, RJ11 and BNC(FEMALE).  

It uses a 1.3-inch white OLED to display all the test status data and modes available.
To show the user which pin is currently being tested it has an 8 led array.
In order to allow the user to select modes and ports it uses a rotary encoder and a tactile button. An additional tactile switch to reset the Arduino.
It can test 3 types of cables RJ45, RJ11 and BNC. It uses nRF24l01+PA+LNA configured at medium power to communicate and Arduino mega as its main processor. 

It has 4 operational modes namely Continuity, Short and Wiring and Manual. As the names suggest each mode serves a purpose. Continuity mode tests continuity for each pin/conductor of the selected port.
Wiring mode tests whether each pin in the connector on one side of the cable is connected to the proper pin in the connector on the other end of the cable depending on the type of the cable and its standard of wiring (RJ45 T568A/T568B). Short Mode checks if any two pins/conductors in the cable are shorted to each other.

### Transmitter
This is the transmitter part of the device. It is powered directly through its micro-USB port.
There are 3 ports connected to the Arduino. RJ45, RJ11 and BNC(FEMALE) through which it detects the voltage levels set by the receiver. It Uses an Arduino Mega pro for processing, nRF24l01+PA+LNA configured in LOW power mode to communicate with the receiver box. 

It has the same 8 LED array as the receiver to indicate the pin/conductor being tested. For port selection it has a tactile button. to indicate the port selected for testing it has a Led above each port.

It has 8 pull-down resistors to pull-down the voltage on any floating pins (pins/conductor in the port that aren’t connected to any voltage). It has the same rocker switch to turn it on.

## Working

Once the transmitter is turned on the LED assigned to each port is initially turned off. The port can be selected by pressing the tactile button. This will cycle through each port and NoPortSelected state (RJ45, RJ11, BNC and NoPortSelected). The LED above each port indicating which port is selected. When no port is selected led assigned to each port will be off. 

When the receiver is turned on the display on the receiver box displays the selected the test mode and port. 

```
MODE: CONTINUITY TEST

PORT: RJ45 T568B

``` 

The mode can be changed by using the rotary encoder. It will cycle through the modes (Continuity, Short, wiring, Manual). The selected port can be changed by pressing the tactile button. This will cycle through the ports (RJ45 T568A, RJ45 T568B, Rj11 and BNC).
Now to activate the test for the selected port the switch on the rotary encoder has to be pressed.

```
CONTINUITY MODE SELECTED

``` 
Once this is done the ardunio in the receiver box will set the pins in the selected port to either high or low in a particular pattern which will be indicated by the 8 LED array on the top. 
If the continuity mode was selected for RJ45 port, pin1 would be set high and the remaining pins would be set low. It would look like this **HIGH LOW LOW LOW LOW LOW LOW LOW** . It will also store this state as a string: `10000000`.
The 8 LED array would represent this in the same way, the first led would be on and the remaining ones would be off. After approximately 600 ms the pin that was set high will change. Now it will be **LOW HIGH LOW LOW LOW LOW LOW LOW** The same will be represented by the LED array.
This will keep iterating and will and cycle the pin that is set high.

At the transmitter if the RJ45 port is selected then the arduino inside it is constantly checking the states of all the pins connected to the port. Once every 200ms.

The cable being tested has one end connected to the receiver and the other end to the transmitter. When the first pin is set high and all others low the transmitter will detect this and create a string representing the pin states. So, if the pin states are **HIGH LOW LOW LOW LOW LOW LOW LOW** 

The string would be: `10000000` and after 600ms when the receiver changes the pin states it will be `01000000` The pin states at the transmitter will be represented by the 8 LED array on it. Provided the cable is not broken and is functional.

After the Transmitter creates the string `10000000` representing the pin states it will transmit this to the receiver arduino through the nRF24l01+PA+LNA module. When the receiver receives this string, it will compare it with the string it had stored on its pin states. 
If the both the strings match then the value of a variable is incremented by one. The string stored and received by the arduino is also displayed on the screen one below the other along with the state of the current pin being tested and the colour of the conductor, as shown below
```

SENT: 10000000
RECV: 10000000
PIN 1 CONNECTED
WIRCLR: White Orange
``` 

After 600 ms it changes the order of the high pin and then compares and increments. When all the pins have been tested and if the value of the variable is <=8 then the arduino will display the result of the test on the screen. 

It would look like this 
```
CONTINUITY TEST PASSED
SENT: 00000001
RECV: 00000001
PIN 8 CONNECTED
WIRCLR: Brown
``` 
This test will keep running until the user stops it or by changing the mode. 

The mode can be changed by rotating the rotary encoder. This will automatically stop the test and change the mode which can then be activated by pressing the encoder switch.

The short and wiring mode also work similarly. The difference in the short mode is when the receiver arduino receives the string from the transmitter it adds all the numbers in the string `10000000` and checks if the sum is greater than one `01100000`. If the sum is greater than one that means that the cable being tested has a short.
Wiring mode works in the same way as the continuity mode.

Manual mode is a bit different. When it is selected the rotary encoder can be used to select which pin is set high. If the initial pin state is `10000000` and the user rotates the encoder clockwise by one indent then the resulting pin state will be `01000000`. All other data (pin number, wire colour, SENT, RECV) will be displayed the same way as other modes including the pin stat representation by the 8 LED array.

Other ports are tested in a similar way. The RJ11 port and BNC port consist of only 2 conductors so only the middle 2 characters of the string are used to represent that (`00010000` and `00001000`).

## Components

#### Here is a list of all the components used in this project.

1.	Arduino Mega 2560(Copy)
2.	Mega2560 Pro ATMEGA2560(Copy)
3.	Prototype Shield V3.0 For Arduino Mega
4.	Dotted PCB 24*10 (2) 
5.	Multi-core wire
6.	220Ω Resistors (16)
7.	5mm Green DIP LED (8)
8.	5mm RED DIP LED (8)
9.	5mm BLUE DIP LED (3)
10.	3 x 7 cm Universal PCB Prototype Board Double-Side (2)
11.	2 x 8 cm Universal PCB Prototype Board Single-Sided
12.	1.3 Inch I2C OLED Display Module 4 pin- WHITE
13.	2.4GHz NRF24L01+PA+LNA SMA Wireless Transceiver 
14.	Tactile Push Button Switch 6x6x5 (3)
15.	Rocker Switch-KDC-01
16.	M274 360 Degree Rotary Encoder Brick Sensor Module
17.	RJ11 Female Port (2)
18.	RJ45 Connector Breakout Module (2)
19.	BNC Female Jack (2) 
20.	USB-C Breakout – Horizontal Module
21.	TP4056 1A Li-Ion Battery Charging Board (2)
22.	XL6009 DC-DC Step-Up Converter LM2577 Booster Circuit Board
23.	18650 3.6V 2500mAh Li-ion Battery (2)
24. PLA Generic black filament

## Circuit Diagram
### Circuit Diagram for the receiver box

![Reciever_circuit_image](https://github.com/user-attachments/assets/87f6f35e-f5e3-4148-b5da-70c74523ed1f)
View in [Circkit Designer](https://app.cirkitdesigner.com/project/7d886924-d269-467c-b663-24d51e59681c)


### Circuit Diagram for the transmitter box

![Transmitter_circuit_image](https://github.com/user-attachments/assets/252e9cfd-8a7b-4bef-9b90-e070491c98b2)
View in [Circkit Designer](https://app.cirkitdesigner.com/project/dbc1c6d2-5d9e-4712-a71e-35fb16c95f0c)



