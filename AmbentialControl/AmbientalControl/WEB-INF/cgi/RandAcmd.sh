#!/bin/bash
ser=/dev/ttyS0

 # Rilevazione dei parametri passati dalla chiamata http
 # Sono presenti nella variabile di ambiente QUERY_STRING
 # nel formato nome1=val1&nome2=val2...

qstring=$QUERY_STRING 
 # Sostituisce & con spazio per isolare le coppie par=val 
qstring=${qstring//&/ }            
 # Rileva le coppie par=val e le mette in un array chiamato par 
read -r -a par <<< "$qstring"  

# Ciclo per ogni elemento p del array par 
for p in ${par[@]}; do         
pn=${p%=*}                       # pn contiene il nome
pv=${p#*=}                        # pv contiene il valore

 # parsing del comando ed invio ad Arduino
case $pn in                       
     QM) 
        echo "M" > $ser
        ;;
     QA)
        echo "Q" > $ser
        ;;
     AN)
        echo "A"$pv > $ser
        ;;
     SC)
        echo "S"$pv > $ser
        ;;
     RA)
        echo "R" > $ser
        ;;
esac   
done # fine del ciclo

sleep 0.2   # Ritardo per dare tempo ad Arduino di rispondere
 # Legge la risposta da Arduino 
read -r -t 2 replay < $ser   

 # La ripete indietro come risposta alla chiamata http 
 # ma prima deve chiudere l'intestazione della risposta http 
 # con una linea vuota.
 # Lo standard output è automaticamente letto dal server Tomcat. 
echo "Content-type: text/html"            
echo ""
echo $replay   # risposta http (alla funzione Javascript)
