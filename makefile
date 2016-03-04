sensors: persistence.c sensors.c 
	gcc -pthread -o bin/sensors persistence.c sensors.c -I.

