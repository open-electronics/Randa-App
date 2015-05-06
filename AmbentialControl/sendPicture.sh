#!/bin/bash
#Nome della foto da spedire in $1
#Usare l'indirizzo vero per la spedizione (mailto=...)!
#Bisogna, inoltre aver aggiornato 
/home/pi/bin/SendMail mailto=pippo@gmail.com subject="Alarm!" attach=$1
#fatto il lavoro chiude motion
sudo pkill motion 