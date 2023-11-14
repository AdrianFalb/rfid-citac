# Zamestnanecký dochádzkový systém s využitím RFID kariet

## Popis projektu
### Stručný opis zamestnaneckého systému
Cieľom projektu je vytvoriť **zamestnanecký systém**, ktorý pomocou kariet alebo klúčeniek s integrovanými čipmi bude zaznamenávať dochádzku zamestnancov, príchody, odchody a obedové prestávky.
Pri príchode do práce priloží zamestnanec kartičku či klúčenku k RFID čítačke čím sa na displeji vypíše čas príchodu. Pri úspešnom načítaní karty sa bude podávať zamestnancovi spätná väzba prostredníctvom displeja a LED diód. Pri odchode zamestnanec priloží kartu k RFID čítačke čim sa zaznamená čas odchodu. Po príchode bude nasledujúce načítanie karty (odchod) reprezentovať začiatok obedovej prestávky. Nasledujúci príchod bude zaznamenaný ako koniec obedovej prestávky.

Pri správnom načítaní karty sa bude zamestnancovi podávať spätná väzba pomocou displeja, zelenej a červenej LED diódy. Pri úspešnom načítaní karty sa rozsvieti zelená LED dióda na dobu 3 sekúnd a vypíše sa na displej čas príchodu respektíve odchodu. Pri neúspešnom čítaní karty zasvieti červená LED dióda po rovnakú dobu ako pri úspešnom čítaní a vypíše sa na displej čas odchodu.

## Popis systému
### Popis komponentov
V systéme sa na čítanie kariet a klúčeniek bude využívať RFID čítačka MF RC522. Každá karta, kľúčenka alebo náramok má priradený unikátny kód a podľa toho sa dá zistiť ktorý modul je priložený k čítačke. Modul typu MF RC522 pracuje s frekvenciou 13,56 MHz.

Čas príchodu a odchodu sa bude vypisovať na displej.

Na riadenie systému sa bude využívať mikrokontrolér STM32F303k8T6. RFID čítačka bude s mikrokontrolérom komunikovať pomocou SPI rozhrania. Displej bude komunikovať prostredníctvom I^2^C zbernice alebo respektíve cez SPI rozhranie podľa vhodnosti.
Mikrokontrolér bude **MASTER** zariadením a RFID čítačka a displej budú **SLAVE** zariadeniami.

## Problémy

1. Ako vyriešiť problém so zaznamenávaním času nakoľko mikrokontrolér nemá nejaký vnutorný časovač reálneho času. Časová synchronizácia môže byť spravená aj pomocou USART pripojením MCU k počítaču.
2. 
