# MultiWave
Multi-Cable Testter


## Description
This is a Wiressly communicating Multi-Cable tester Project.
It comprises of 2 main parts encased in 2 different cases. The Reciever and the Transmitter. The receiver being the main processor.
### Reciever
It uses a 1.3-inch white OLED to display all the test status data and modes available.
To show the user which pin is currently being tested it has an 8 led array.
In order to allow the user to select modes and ports it uses a rotary encoder and a tactile switch.
It can test 3 types of cables RJ45, RJ11 and BNC. It uses nRF24l01+PA+LNA to communicate and arduino mega as its main processor. 
It has 4 operational modes namely Continuity Short and Wiring and Manual. As the names suggest each mode serves a purpose. Continuity mode tests continuity for each pin/conductor of the selected port.
Wiring mode tests whether each pin in the connector on one side of the cable is connected to the proper pin in the connector on the other end of the cable depending on the type of the cable and its standard of wiring (RJ45 T568A/T568B). Short Mode checks if any two pins/conductors in the cable are shorted to each other.

### Transmitter
