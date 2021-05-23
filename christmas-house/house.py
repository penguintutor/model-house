from gpiozero import LED
from rpi_ws281x import PixelStrip, Color

screen_title = "Pygame Zero Christmas House"

# Set Pygame Zero screen size and title
WIDTH = 800
HEIGHT = 600
TITLE = screen_title

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
SMOKE_PIN = 23

fire_led = LED(FIRE_PIN)
lamp_led = LED(LAMP_PIN)
green_led = LED(GREEN_PIN)
red_led = LED(RED_PIN)
# Smoke generator is defined same as an LED
smoke = LED(SMOKE_PIN)

# Smoke is actor so it can be shown on vs off
smoke_button = Actor("smoke-off", (140,20))
# Rest are just click zones - rectangles
lamp_box = Rect(320, 285, 155, 100)
snowman_box = Rect(110,410, 130, 155)
xmas_box = Rect(120,220, 430, 65)

# delay_timer used to slow updates down
delay_timer = 0
delay_length = 15
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

# Default turn everything on (except for fire as needs smoke fluid)
fire = False
lamp_led.on()
snowman_on = True
xmaslights_on = True

def draw():
    screen.blit("house", (0,0))
    smoke_button.draw()



def update():
    global delay_timer, delay_length
    if delay_timer < delay_length:
        delay_timer += 1
    else:
        delay_timer = 0
        led_change()


def led_change():
    global timer, red_green, seq_number, fire, snowman_on, xmaslights_on
    timer += 1
    # if number is divisble by 6 then flicker red led
    if (timer % 8 != 0) and fire == True:
        fire_led.on()
    else:
        fire_led.off()

    # toggle red green leds
    if (timer % 10 == 0):
        if xmaslights_on == False:
            red_led.off()
            green_led.off()
        else:
            red_green = 1 - red_green
            if (red_green == 1):
                red_led.on()
                green_led.off()
            else:
                red_led.off()
                green_led.on()


    # Color change snowman
    if (timer % 12 == 0):
        if (snowman_on == False):
            strip.setPixelColor(0, Color(0,0,0))
        else:
            seq_number += 1
            if (seq_number >= len(rgb_colours)):
                seq_number = 0
            strip.setPixelColor(0, rgb_colours[seq_number])
        strip.show()


def on_mouse_down(pos, button):
    global fire, snowman_on, xmaslights_on
    if button == mouse.LEFT:
        if lamp_box.collidepoint(pos):
            lamp_led.toggle()
            #print ("Changing Lamp status")
        if smoke_button.collidepoint(pos):
            if fire == False:
                fire = True
                smoke.on()
                smoke_button.image = "smoke"
                #print ("Smoke on")
            else :
                fire = False
                smoke.off()
                smoke_button.image = "smoke-off"
                #print ("Smoke off")
        if snowman_box.collidepoint(pos):
            if (snowman_on == True):
                snowman_on = False
                #print ("Snowman off")
            else:
                snowman_on = True
                #print ("Snowman on")
        if xmas_box.collidepoint(pos):
            if (xmaslights_on == True):
                xmaslights_on = False
                #print ("Xmas lights off")
            else:
                xmaslights_on = True
                #print ("Xmas lights on")
