# Chat
Name: Hongru Qi
Partner: Jiahui Wang

Accessories:
* Arduino Mega Board

Wiring instructions:
Joy Stick (Board Pins <--> Arduino Pins):
*GND <--> GND (board bus)
*5V	<-->	5V (board bus)
*VRX	<-->	Analog Pin 1
*VRY	<-->	Analog Pin 0
*SW	<-->	Digital Pin 2
Screen (Board Pins <--> Arduino Pins):
*GND <--> GND
*Vin <--> 5V
*3Vo <--> NOT CONNECTED
*CLK <--> 52
*MISO <--> 50
*MOSI <--> 51
*CS <--> 10
*D/C <--> 9
*RST <--> NOT CONNECTED
*Lite <--> NOT CONNECTED
*Y+ <--> A2 (analog pin)
*X+ <--> 4  (digital pin)
*Y- <--> 5  (digital pin)
*X- <--> A3 (analog pin)
*IM0 - IM3 <--> NOT CONNECTED (they expect a 3.3v signal, DON'T CONNECT!)
*CCS <--> 6
*CD <--> NOT CONNECTED



Running Instructions:
1. Connect the wires, joy stick and screen as described
2. Connect the Arduino Mega Board to a computer
3. Upload the code

Assumption:
1. User may fail to connect the Arduino Mega Board to computer for the first time and need to try again.
2. User needs to be careful with the wiring as one mistake will cause the screen or joy stick not working properly.
3. User will find that the cursor will return to the center of map when move to the next page.

Function:
There is no addtional functionality.
