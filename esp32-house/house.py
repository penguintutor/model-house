import urllib.request

screen_title = "Pygame Zero Christmas House"

# Set Pygame Zero screen size and title
WIDTH = 800
HEIGHT = 600
TITLE = screen_title

WEB_ADDR = "http://192.168.0.53/"

LAMP_LIGHT = 1
FIRE_LIGHT_1 = 2
FIRE_LIGHT_2 = 3
SMOKE_LIGHT = 4 # Smoke generator is defined same as an LED

# Buttons are actors - images that change when clicked
smoke_button = Actor("smoke-off", (255,30))
fire_button = Actor("fire", (155,510))
window_button = Actor ("window", (500, 360))


# delay_timer used to slow updates down
delay_timer = 0
delay_length = 15
timer = 0



# status is string "on, off, flicker"
def set_light (light_id, status) :
    if (status == "on"):
        url = WEB_ADDR + "switchon?light="+str(light_id)
    elif (status == "off"):
        url = WEB_ADDR + "switchoff?light="+str(light_id)
    elif (status == "flicker"):
        url = WEB_ADDR + "flicker?light="+str(light_id)
    # no checking just fire request and assume successful
    # if connection lost then it ignore failures
    # but will work again when available
    try:
        with urllib.request.urlopen(url) as response:
            returned_text = response.read()
            # ignoring the returned value
    except:
        pass



# Default turn everything on (except for fire as needs smoke fluid)
lamp_status = True
set_light(LAMP_LIGHT, "on")
fire_status = True
set_light(FIRE_LIGHT_1, "flicker")
set_light(FIRE_LIGHT_2, "flicker")
smoke_status = False
set_light(SMOKE_LIGHT, "off")


def draw():
    screen.blit("house", (0,0))
    smoke_button.draw()
    fire_button.draw()
    window_button.draw()

def on_mouse_down(pos, button):
    global fire_status, smoke_status, lamp_status
    if button == mouse.LEFT:
        if window_button.collidepoint(pos):
            if lamp_status == False:
                lamp_status = True
                set_light(LAMP_LIGHT, "on")
                window_button.image = "window"
            else:
                lamp_status = False
                set_light(LAMP_LIGHT, "off")
                window_button.image = "window-off"

        if fire_button.collidepoint(pos):
            if fire_status == False:
                fire_status = True
                set_light(FIRE_LIGHT_1, "flicker")
                set_light(FIRE_LIGHT_2, "flicker")
                fire_button.image = "fire"

            else :
                fire_status = False
                set_light(FIRE_LIGHT_1, "off")
                set_light(FIRE_LIGHT_2, "off")
                fire_button.image = "fire-off"

        if smoke_button.collidepoint(pos):
            if smoke_status == False:
                smoke_status = True
                set_light(SMOKE_LIGHT, "on")
                smoke_button.image = "smoke"

            else :
                smoke_status = False
                set_light(SMOKE_LIGHT, "off")
                smoke_button.image = "smoke-off"
