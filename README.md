# Edison_Nixie_Clock
Nixie clock but with Edison LED to mimic the Nixie Clock, 

## Introduction

I always want to make a nixie clock on my desk, but it's hard and expensive to buy a working nixie clock in my place, the only option is to buy from other countries.
While scrolling thourgh locak online marketplace I stumbeled upon a edison filament LED, and I found a seller that sell this LED in 18.5mm form factor. So I thought that I can make a small anough 7 segment display that resemble the Nixie display. Combining with microcontroller that connected with RTC Module, and a temperature and humidity sensor also.

## Components and Schematics

### Edison LED

The main part that I use offcourse is the LED, which is the edison led with the size of 18.5mm. 8 of this LEDs are then assembled and soldered with a cooper wire like the image bellow

<img src="/assets/image/LEDSchematic.jpg" width="250">

Then to make it easy to be connected to breadboard, the end of the cooper wire is soldered into a header pin. In the end will looked like this:

<img src="/assets/image/LEDPhoto.png" width="350">

The LEDs intensity can also be controlled, the cathode of the LED are connected into the 2n2222 into ground, and the base pin is controlled with PWM from microcontroller

<img src="/assets/image/LEDPhotoControll.png" width="350">

### LEDs Controll

To control the LEDs, I use the SN74HC595N Shift Register 8-bit ICs. I use four of this ICs to controll 4 LEDs, representing the Hour-Hour : Minute-Minute.
All of this ICs, are then connected in series, so it only need a total of 3 pins, which are the CLK, Latch, and Serial data, so the data from microcontroler are send first into the First Hour, then from the QH' pin of the ICs goes into the Ser pin on the second Ics, and so on. It minimized the use of pins from the microcontroller.

The Schematic of the LEDs Controll Will look like this:

![LED_Control](/assets/image/LEDControll.png)

### Second Light

As stated above, there is no LED to show the current second, so to visualize it I use a mini 5x25mm incandescent lamps, from the seller it state that it can be powered with 2.5-6V. To light this, the anode is connected to 3.3V and the cathode to 2n2222 Transistor to ground, the this transistor is controlled with the microcontroller, so the microcontroller only need to supply voltage to controll and to power the LEDs.

![Second LED](/assets/image/SecondLight.png) 

<img src="/assets/image/SecondLightSchematic.png" width="350">

### RTC

As for the RTC I use the DS3231

<img src="/assets/image/RTC.jpg" width="350">

### Temp and Humid Sensor

For the sensor I use the HTU21D, it can be connected to Microcontroller using the I2C

<img src="/assets/image/HTU21D.jpg" width="350">

### Microcontroller

Lastly for the microcontroller I use the ESP32-C3 Mini, I use this microcontroller because it compact size

<img src="/assets/image/ESP32C3.jpg" width="350">

---

### First Assembly

Firstly I try to assembly all in a project board to check if every thing is connected, working, and for code validation

<img src="/assets/image/breadboard_1.jpg" width="550">

After it show a sucesfull result, then I make the schematic and PCB for production.

## Full Schematic

### Top PCB

<img src="/assets/image/TOPPCB.png" width="550">

### Bottom PCB

<img src="/assets/image/BottomPCB.png" width="550">

But turns out, I make some mistakes on the button, so I fix it with some cables, and so the revised schematic should looks like this

<img src="/assets/image/BottomPCB_Rev.png" width="350">


### PCB Results

<img src="/assets/image/PCBResult.png" width="450">

### Assembly

The assembly using the top and bottom layer will result like this

<img src="/assets/image/onpcb_1.png" width="800">

what it need lastly is the case/enclosure, so I try to make it using a copper color PLA Fillament to mimic the vintage look

<img src="/assets/image/NixieClockResult.png" width="800">

### Result

Video: https://youtu.be/DrIr_Akatko?si=Y6-Fsod_8FQzEh6e
