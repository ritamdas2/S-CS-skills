# Code Readme

Example code uses the H-bridge to modulate UART with a 38 kHz carrier with the ESP RMT module

Modulates ‘traffic light’ signals in the example code

Code was modified so that a button click will cause one fob to send a code (containing the fob state – off, red, green, yellow) to the receiving fob. If received by the second fob, it will set its state to the new state (off, red, green, yellow) and then light up the appropriate LED color.

State/LED color on the first fob can be changed by button click, and then, using a second button, a click causes the state of the fob to be sent via IR to the second fob. The second fob matches the LED color from the sender.
