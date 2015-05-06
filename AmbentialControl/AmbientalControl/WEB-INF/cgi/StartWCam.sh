#!/bin/bash

 # Rilevazione dei parametri passati dalla chiamata http
 # Sono presenti nella variabile di ambiente QUERY_STRING
 # nel formato nome1=val1&nome2=val2...

qstring=$QUERY_STRING 
 # Sostituisce & con spazio per isolare le coppie par=val 
qstring=${qstring//&/ }   
 # Rileva le coppie par=val e le mette in un array chiamato par 
read -r -a par <<< "$qstring"  
ang=-1
 
 # Ciclo per ogni elemento p del array par 
for p in ${par[@]}; do         
pn=${p%=*}                      # pn contiene il nome
pv=${p#*=}                       # pv contiene il valore

 # parsing del comando  
if [ $pn = "wcam" ]; then
 fstart=$pv
fi

done # fine ciclo

 # azione

mconfig="/home/apache-tomcat-7.0.47/webapps/ControlloAmbientale/WEB-INF/cgi/motion.conf"
if [ $fstart = "OK" ]; then
 motion -c $mconfig           # lancia motion e risponde alla chiamata http
 echo “Content-type: text/html"
 echo “”
 echo "OK"
else
 pid=$(< /tmp/motion.pid)   # altrimenti chiude motion e risponde
 sudo kill $pid 
 echo “Content-type: text/html”
 echo “”
 echo "NOK"
fi
