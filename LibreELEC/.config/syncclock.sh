while true
do
   sleep 60
   ALL=en_EN.utf8 date +"T%b %e %Y|%H:%M:%S" > /dev/ttyACM0 2>> /var/log/clocksync.error
done
