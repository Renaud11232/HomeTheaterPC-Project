(sleep 8
echo "U" > /dev/ttyACM0 2>> /var/log/serialComm.log
bash /storage/.config/syncclock.sh) &
