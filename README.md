# Zamestnanecký dochádzkový systém s využitím RFID čítačky

## Popis projektu
### Stručný opis zamestnaneckého systému
Cieľom projektu je vytvoriť **zamestnanecký systém**, ktorý pomocou kariet alebo klúčeniek s integrovanými čipmi bude zaznamenávať dochádzku zamestnancov (príchody a odchody).
Pri príchode do práce stlačí zamestnanec tlačidlo "príchod" a priloží kartičku či klúčenku k RFID čítačke. Pri úspešnom načítaní karty dostane zamestnanec spätnú väzbu prostredníctvom LCD displeja, na ktorom sa zobrazí číslo jeho karty a čas príchodu. Pri odchode z práce zamestnanec stlačí tlačidlo "odchod" a priloží kartu či klúčenku k RFID čítačke. Na SD kartu sa pri každom odchode a príchode zapisujú informácie o unikátnom čísle čipu karty či klúčenky, časovej značky a informácia či ide o odchod alebo príchod.

Pri správnom načítaní karty sa bude zamestnancovi podávať spätná väzba pomocou displeja vypísaním unikátneho čísla čipu karty či klúčenky. Každá karta alebo kľúčenka má priradený unikátny kód na základe, ktorého sa dá zistiť, ktorý zamestnanec priložil k čítačke kartu respektíve klúčenku. Každý zamestnanec bude mať priradú kartu s unikátnym číslom čipu na základe, ktorého sa dá uchovávať jeho dochádzka.

## Popis systému
### Popis komponentov
Na riadenie systému sa bude využívať mikrokontrolér STM32F303k8T6, ktorý bude **MASTER** zariadením.

V systéme sa na čítanie kariet a klúčeniek bude využívať RFID čítačka MF RC522. RFID čítačka, LCD Displej a čítač/ zapisovač SD karty budú s mikrokontrolérom komunikovať prostredníctvom SPI zbernice.
Mikrokontrolér bude **MASTER** zariadením a RFID čítačka a displej budú **SLAVE** zariadeniami.

Na získanie časovej značky sa využije RTC periféria, ktorá je zabudovaná v STM32F303k8T6 mikrokontroléri.

### MF RC522

<img src="https://github.com/AdrianFalb/rfid_citac/assets/99915031/9edfc617-d5cf-4bc0-8967-ecdeb1d0b91b" width="250" height="250">

### LCD Displej

<img src="https://github.com/AdrianFalb/rfid_citac/assets/99915031/d7f08860-f607-4102-bd43-38fc161fd7b3" width="250" height="250">

### Čítač/ zapisovač SD karty

<img src="https://github.com/AdrianFalb/rfid_citac/assets/99915031/13999680-26db-45f4-9905-a42f376a57bd" width="250" height="250">

