# eddi-sensors
A lightweight multithreaded C program that monitors the sensor pins and outputs their data to a socket.

### Compile
run ```make``` from the parent directory

### Run
run ```bin/sensors &```

### Data
output data is pushed to a Unix Stream Socket at ```data/sensors.sock```
The program will not read from the sensors until a listener is attached to this socket.
