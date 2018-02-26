# Arduino Irrigation System
An irrigation system built with Arduino Uno R3

Link Bill of Material (BOM): https://docs.google.com/spreadsheets/d/1g82J1nOvGN7cDaFHDwJvao99QfbgMDixFHSb_e4Ozuw/edit?usp=sharing

Descriere:
  Sistemul de irigație cuprinde o modalitate de monitorizare a umidității solului și două moduri de a activa udarea zonei monitorizate, implicit a unei plante: una automată și una manuală.
  
Metode de setare:
  Informațiile privind umiditatea solului, modul de funcționare, nivelul critic de umiditate la care se emite avertizarea prin intermediul ledului roșu sau se udă automat planta, respectiv durata de funcționare a pompei în timpul unui cicle sunt indicate pe afișajul LCD. Aceste informații pot fi accesate prin utilizare tastelor 1 și 2 de pe telecomandă.
  Pentru a modifica modul de udare (de la manual la automat sau invers) se va apăsa pe tasta EQ a telecomenzii, atunci când putem vedea primul afișaj cu informații, acolo unde apare modul de funcționare.
  Nivelul critic se poate modifica prin apăsarea tastei 3 din panoul de informații, iar durata unui ciclu de udare prin apăsarea tastei 4.
  Ambii parametri variază între 1 și 99, reprezentând procentul de umiditate, respectiv timpul în secunde.
  Pentru modificări se folosesc tastele - și + de pe telecomandă, iar pentru a salva sau anula noile configurări se apasă CH+, respectiv CH-.
  Pentru a activa pompa se apasă tasta 0.
  
Sensul ledurilor:
  Ledul roșu indică faptul că ne aflăm în modul automat iar umiditatea solului se află sub pragul critic stabilit. Acesta se stinge atunci când activăm manual pompa sau schimbăm modul de funcționare al sistemului în cel automat.
  Ledul albastru indică funcționarea pompei.
  
Senzorul de umiditate:
  Oferă o citire la fiecare secundă și furnizează valori între 0 și 100. În funcție de valorile furnizate de acesta se va acționa sau nu pompa în modul automat, respectiv se va aprinde sau stinge ledul roșu.
  
Particularități ale construcției:
  Pentru a furniza 12V pompei s-a folosit o baterie de 9V conectată la un releu.
