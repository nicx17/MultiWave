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




