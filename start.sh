./sensors.out &
echo $! > ./sensors.pid

./control.out
echo $! > ./control.pid
