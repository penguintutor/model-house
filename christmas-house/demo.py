#!/usr/bin/python3
from gpiozero import LED
from rpi_ws281x import PixelStrip, Color
import time

# config details for neopixel
LEDCOUNT = 1
GPIOPIN = 18
FREQ = 800000
DMA = 5
INVERT = True
BRIGHTNESS = 255

FIRE_PIN = 24
LAMP_PIN = 25
GREEN_PIN = 17
RED_PIN = 27

fire_led = LED(FIRE_PIN)
lamp_led = LED(LAMP_PIN)
green_led = LED(GREEN_PIN)
red_led = LED(RED_PIN)

timer = 0

# red / green chaser mode - toggle between red and green
# if 1 = red led on, if 0 then green led on
red_green = 1

rgb_colours = [Color(248,12,18), Color (255,51,17), Color(255,102,68), \
               Color(254,174,45), Color(208,195,16), Color(105,208,37), \
               Color(18,189,185), Color(68,68,221), Color(59,12,189)]
seq_number = 0

# Create neopixel object
strip = PixelStrip(LEDCOUNT, GPIOPIN, FREQ, DMA, INVERT, BRIGHTNESS)
strip.begin()
strip.setPixelColor(0, rgb_colours[0])
strip.show()

def led_change():
    global timer, red_green, seq_number
    timer += 1
    # if number is divisble by 6 then flicker red led
    if (timer % 6 == 0):
        fire_led.off()
    else:
        fire_led.on()

    # toggle red green leds
    if (timer % 10 == 0):
        red_green = 1 - red_green
        if (red_green == 1):
            red_led.on()
            green_led.off()
        else:
            red_led.off()
            green_led.on()


    # Color change snowman
    if (timer % 12 == 0):
        seq_number += 1
        if (seq_number >= len(rgb_colours)):
            seq_number = 0
        strip.setPixelColor(0, rgb_colours[seq_number])
        strip.show()



lamp_led.on()

while True:
    led_change()
    time.sleep(0.25)


