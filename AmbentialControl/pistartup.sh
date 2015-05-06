#!/bin/bash
#exit 0          #per disabilitare lo script togliere il commento
ser=/dev/ttyS0
stty -F $ser 9600
sleep 5
echo "M" > $ser 
echo "Comando M" > /home/pi/start.log
  # ritardo per dare tempo ad Arduino di rispondere
sleep 1  
  # legge la risposta di Arduino                    
read -r -t 2 replay < $ser
if [ ${#replay} -lt 4 ]; then
 echo "Nessuna risposta!" >> /home/pi/start.log
 exit 1
fi  
replay=${replay:0:4}   
echo $replay >> /home/pi/start.log
if [ $replay = "TEST" ]; then
 echo "No motion" >> /home/pi/start.log
 exit 0                            #se in modalità test esce
fi
if [ $replay = "CTRL" ]; then
 echo "Start motion!" >> /home/pi/start.log
  # altrimenti chiude eventuale processo motion attivo
 sudo pkill motion          
  # e attiva motion con rilevatore di movimento ed invio foto 
 motion -c /home/pi/motion-detect.conf 
fi
