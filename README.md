# Zamestnanecký dochádzkový systém s využitím RFID čítačky

## Popis projektu
### Stručný opis zamestnaneckého systému
Cieľom projektu je vytvoriť **zamestnanecký systém**, ktorý pomocou kariet alebo klúčeniek s integrovanými čipmi bude zaznamenávať dochádzku zamestnancov (príchody, odchody a obedové prestávky).
Pri príchode do práce priloží zamestnanec kartičku či klúčenku k RFID čítačke. Pri úspešnom načítaní karty dostane zamestnanec spätnú väzbu prostredníctvom displeja a LED diód. Po príchode bude nasledujúce načítanie karty (odchod) reprezentovať začiatok obedovej prestávky. Nasledujúci príchod bude zaznamenaný ako koniec obedovej prestávky. Pri odchode z práce zamestnanec opäť priloží kartu k RFID čítačke čím sa zaznamená čas odchodu. Do externej pamäte sa zakaždým uloží záznam pozostávajúci z unikátneho čísla čipu, časovej značky a doplňujúcej informácie.

Pri správnom načítaní karty sa bude zamestnancovi podávať spätná väzba pomocou displeja, zelenej a červenej LED diódy. Pri úspešnom načítaní karty pri príchode a odchode sa rozsvieti zelená LED dióda na dobu 3 sekúnd a vypíše sa na displej čas príchodu respektíve odchodu. Pri neúspešnom čítaní karty zasvieti červená LED dióda po rovnakú dobu ako pri úspešnom čítaní a vypíše sa na displej, že čítanie bolo neúspešné.

Každá karta alebo kľúčenka má priradený unikátny kód na základe, ktorého sa dá zistiť, ktorý zamestnanec priložil k čítačke kartu respektíve klúčenku.

## Popis systému
### Popis komponentov
Na riadenie systému sa bude využívať mikrokontrolér STM32F303k8T6.

V systéme sa na čítanie kariet a klúčeniek bude využívať RFID čítačka MF RC522. RFID čítačka bude s mikrokontrolérom komunikovať pomocou SPI rozhrania. Displej bude komunikovať prostredníctvom I2C zbernice respektíve cez SPI rozhranie podľa dostupnosti.
Mikrokontrolér bude **MASTER** zariadením a RFID čítačka a displej budú **SLAVE** zariadeniami.

## Problémy

1. Ako vyriešiť problém so zaznamenávaním času nakoľko mikrokontrolér nemá nejaký vnutorný časovač reálneho času. Časová synchronizácia môže byť spravená aj pomocou USART pripojením MCU k počítaču.
2. 
