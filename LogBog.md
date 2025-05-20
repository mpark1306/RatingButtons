### Lånt udstyr:
---
- 1 ESP32 board
- 1 Breadboard
- 4 knapper
- 4 LEDs
- 4 Resistorer
- 8 Ledninger

# Logbog

### Dag 1 (d. 20-05-2025)
Her til morgen blev vi kastet ud i opgaven, fik udstyret i hænderne og gik straks i gang. Vi byggede fire separate kredsløb på breadboardet – ét for hver knap og én LED-farve – som alle er koblet til ESP32’ens digitale porte.

Undervejs ramte vi et klassisk bump: vores første kode fik pludselig alle lamper til at lyse på én gang, når vi trykkede på bare én knap. Efter en omgang detektiv-fejlsøgning – fra at dobbelttjekke ledningsføringen over på breadboardet til at finjustere debounce-logikken – lykkedes det os endelig at få hver knap til kun at tænde sin egen LED. Nu kører systemet stabilt, og vi er klar til næste skridt!
