case "$1" in
   halt)
      echo "H" > /dev/ttyACM0 2>> /var/log/serialComm.log
      ;;
   poweroff)
      echo "H" > /dev/ttyACM0 2>> /var/log/serialComm.log
      ;;
   reboot)
      echo "R" > /dev/ttyACM0 2>> /var/log/serialComm.log
      ;;
esac
