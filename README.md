# Simple Strip Cutter

## Description

[Українська версія](README_ua.md)

Instructions on assembling a simple strip cutter used in battery manufacturing. The device also has the capability to weld a second layer of strip for welding copper using "sandwich" technology. Included are 3D print files and code for Arduino Nano.

- [Video introduction](https://www.youtube.com/watch?v=2g4eOFBQs-M)
- [Jig mentioned in the previous video](https://youtu.be/SqSB7JrVHbk)

![Demo Gif](demo_gif.gif)


## Table of Contents

- [Arduino Nano Flashing](#arduino-nano-flashing)
- [List of Parts](#list-of-parts)
- [Electrical Schematic](#electrical-schematic)
- [TODO](#todo)

## Arduino Nano Flashing

- Install libraries using Arduino's IDE Library Manager:
  - Search for and install **AccelStepper**
  - Search for and install **VL53L0X_mod**
- If the welder functionality is not needed:
  - Change `bool needWelder = true;` to `bool needWelder = false;` in the code.
- To test modules separately, locate `// UNCOMMENT NEEDED TEST` in the code and follow the instructions provided.

## List of Parts

- **CNC Shield for Arduino Uno**  
  [https://vi.aliexpress.com/item/1005002807506440.html](https://vi.aliexpress.com/item/1005002807506440.html)
- **CNC Shield for Arduino Nano**  
  [https://vi.aliexpress.com/item/32834755847.html](https://vi.aliexpress.com/item/32834755847.html)
- **Step Motor NEMA17 JK42HS40**  
  [https://arduino.ua/prod2962-shagovii-dvigatel-nema17-jk42hs40-1704-13a-b](https://arduino.ua/prod2962-shagovii-dvigatel-nema17-jk42hs40-1704-13a-b)  
  [https://evse.com.ua/shagovyj-dvigatel-nema17-17a-17hs4401-3d-printer](https://evse.com.ua/shagovyj-dvigatel-nema17-17a-17hs4401-3d-printer)
- **Welding Handle**  
  [https://vi.aliexpress.com/item/1005006570618453.html](https://vi.aliexpress.com/item/1005006570618453.html)
- **Scissors JTC 3422A**  
  [https://jtc.com.ua/instrument-zagalnogo-priznachennya/instrument-slyusarnij/nozhici-bagatofunkcionalni/](https://jtc.com.ua/instrument-zagalnogo-priznachennya/instrument-slyusarnij/nozhici-bagatofunkcionalni/)
- **Relay 5V 10A Low Level**  
  [https://arduino.ua/prod1706-modyl-rele-5v-10a-nizkogo-yrovnya-low-level](https://arduino.ua/prod1706-modyl-rele-5v-10a-nizkogo-yrovnya-low-level)
- **Step Motor Driver StepStick A4988**  
  [https://arduino.ua/prod965-draiver-shagovogo-dvigatelya-stepstick-a4988](https://arduino.ua/prod965-draiver-shagovogo-dvigatelya-stepstick-a4988)
- **Laser Distance Sensor GY-530 on VL53L0X**  
  [https://arduino.ua/prod2144-lazernii-datchik-rasstoyaniya-gy-530-na-vl53l0x](https://arduino.ua/prod2144-lazernii-datchik-rasstoyaniya-gy-530-na-vl53l0x)
- **Strip**  
  [https://vi.aliexpress.com/item/1005008182375950.html](https://vi.aliexpress.com/item/1005008182375950.html)
- **Button**  
  [https://arduino.ua/prod3756-knopka-pbs-33b-bez-fiksacii-off-on-krasnaya](https://arduino.ua/prod3756-knopka-pbs-33b-bez-fiksacii-off-on-krasnaya)  
  [https://arduino.ua/prod3528-knopka-avariinoi-ostanovki](https://arduino.ua/prod3528-knopka-avariinoi-ostanovki)
- **Breadboard**  
  [https://arduino.ua/prod361-maketnaya-plata-mb-102-830-otverstii](https://arduino.ua/prod361-maketnaya-plata-mb-102-830-otverstii)
- **Breadboard for Soldering**  
  [https://arduino.ua/prod4130-mednaya-maketnaya-plata-diy-stripboard-iz-getinaksa-133x48-mm](https://arduino.ua/prod4130-mednaya-maketnaya-plata-diy-stripboard-iz-getinaksa-133x48-mm)
- **Arduino Nano**  
  [https://arduino.ua/prod6510-arduino-nano-v3-0-avr-atmega328p-type-c](https://arduino.ua/prod6510-arduino-nano-v3-0-avr-atmega328p-type-c)
- **Wires**  
  [https://arduino.ua/prod195-nabor-peremichek-dlya-arduino-65-sht](https://arduino.ua/prod195-nabor-peremichek-dlya-arduino-65-sht)
- **Voltage Regulator LM7805 TO-220**  
  [https://arduino.ua/prod1844-stabilizator-napryajeniya-lm7805-to-220](https://arduino.ua/prod1844-stabilizator-napryajeniya-lm7805-to-220)
- **Capacitor**  
  [https://arduino.ua/prod6477-kondensator-elektrolitichnii-jccon-47mkf-25v-lowesr](https://arduino.ua/prod6477-kondensator-elektrolitichnii-jccon-47mkf-25v-lowesr)
- **Resistors**  
  [https://arduino.ua/prod409-nabor-rezistorov-0-25w-600-sht](https://arduino.ua/prod409-nabor-rezistorov-0-25w-600-sht)
- **Servo SG90**  
  [https://arduino.ua/prod416-servoprivod-sg90-2kg](https://arduino.ua/prod416-servoprivod-sg90-2kg)
- **Plug 5.5x2.5mm**  
  [https://arduino.ua/prod1380-shteker-2-5-h-5-5-mm-papa-1-sht](https://arduino.ua/prod1380-shteker-2-5-h-5-5-mm-papa-1-sht)
- **Socket DC 5.5/2.5mm**  
  [https://arduino.ua/prod3909-gnezdo-pitaniya-dc-5-52-5mm-s-klemmnoi-kolodkoi-pod-vint](https://arduino.ua/prod3909-gnezdo-pitaniya-dc-5-52-5mm-s-klemmnoi-kolodkoi-pod-vint)
- **Power Supply 12V**  
  [https://evse.com.ua/blok-pitaniya-setevoj-adapter-12v-6a-55x21mm-55x25mm-kabel](https://evse.com.ua/blok-pitaniya-setevoj-adapter-12v-6a-55x21mm-55x25mm-kabel)

## Electrical Schematic

![Strip Cutter Schematic](strip_cutter_scheme.png)

## TODO

- Clean up the code.
- Improve the electrical schematic.
- Redesign scissors motor holder - it is ugly.
