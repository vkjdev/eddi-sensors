SENSOR_PID="$(cat ./sensors.pid)"
kill -9 $SENSOR_PID

CONTROL_PID="$(cat ./control.pid)"
kill -9 $SENSOR_PID
