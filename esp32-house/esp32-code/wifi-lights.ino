/*
  ESP32 wifi-lights 
  activates LEDs based upon wifi instructions
*/
#include <WiFi.h>
 

const char* ssid = "SSID-name";
const char* password = "Password";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 53);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // Google DNS
IPAddress secondaryDNS(8, 8, 4, 4); // Google DNS

// Set web server port number to 80
WiFiServer server(80);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// flicker rate (ms)
const int flickerOn = 100;
const int flickerOff = 5;

 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// GPIO pin numbers - entry 0 is ignored 
int lightPins[] = {0, 15, 16, 17};
// status 0=off, 1=on, 2=flicker 3 and higher is flash rate in ms (50:50 cycle)
unsigned lightStatus[sizeof(lightPins)];
// tracks last time light status changed
// used in flicker
unsigned lightChangeTime[sizeof(lightPins)];

// Variable to store the HTTP request
String header;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
 
void setup()
{
  // Set all pins as output and status = 0
  for (int i=1; i<sizeof(lightPins); i++) {
    pinMode(lightPins[i], OUTPUT);
    digitalWrite(lightPins[i], LOW);
    lightStatus[i] = 0;
    lightChangeTime[i] = 0;
  }
  
  // Serial monitor setup
  Serial.begin(115200);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  initWiFi();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print ("Connecting");
  }
  server.begin();
}

String ledStatus (int light)
{
  if (lightStatus[light] == 0) return "off";
  else if (lightStatus[light] == 1) return "on";
  else if (lightStatus[light] == 2) return "flicker";
  else if (lightStatus[light] > 2) return "flash " + String(lightStatus[light]);
  else return "";
} 


void switchOn (int pinId)
{
  digitalWrite (lightPins[pinId], HIGH);
  lightStatus[pinId] = 1;
}

void switchOff (int pinId)
{
  digitalWrite (lightPins[pinId], LOW);
  lightStatus[pinId] = 0;
}

void flicker (int pinId)
{
  digitalWrite (lightPins[pinId], HIGH);
  lightStatus[pinId] = 2;
  lightChangeTime[pinId] = millis(); //set flicker to current time
}

void flash (int pinId, unsigned rate) 
{
  // if rate too small then set to minimum 
  if (rate < 3) rate = 3;
  digitalWrite (lightPins[pinId], HIGH);
  lightStatus[pinId] = rate;
  lightChangeTime[pinId] = millis(); //set flash to current time
  Serial.println ("flash set to "+String(pinId)+" rate "+String(rate));
}
 
void loop()
{

  unsigned long currentMillis = millis();

  // check for change in flicker / flash
  for (int i=1; i<=sizeof(lightPins); i++)
  {
    // if flicker
    if (lightStatus[i] == 2)
    {
      if (digitalRead(lightPins[i]) == HIGH)
      {
        if (currentMillis > lightChangeTime[i] + flickerOn) 
        {
          digitalWrite(lightPins[i], LOW);
          lightChangeTime[i] = currentMillis;
        }
      }
      else
      {
        if (currentMillis > lightChangeTime[i] + flickerOff) 
        {
          digitalWrite(lightPins[i], HIGH);
          lightChangeTime[i] = currentMillis;
        }
      }
    }
    // if flash
    else if (lightStatus[i] > 2)
    {
      if (digitalRead(lightPins[i]) == HIGH)
      {
        if (currentMillis > lightChangeTime[i] + lightStatus[i]) 
        {
          digitalWrite(lightPins[i], LOW);
          lightChangeTime[i] = currentMillis;
        }
      }
      else
      {
        if (currentMillis > lightChangeTime[i] + lightStatus[i]) 
        {
          digitalWrite(lightPins[i], HIGH);
          lightChangeTime[i] = currentMillis;
        }
      }
    }
  }
  


  // Regularly check to see if still connected by WiFi
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        // end of the client request, send a response
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          
          if (currentLine.length() == 0) {
            // return HTTP headers (currently just return 200 OK regardless)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            int light_num = 0;
            int rate_num = 0;
           
            if (header.indexOf("GET /switchon?light=") >= 0) {
              String light_string = header.substring(20);
              light_num = light_string.toInt();
              switchOn(light_num);
            }  
            else if (header.indexOf("GET /switchoff?light=") >= 0) {
              String light_string = header.substring(21);
              light_num = light_string.toInt();
              switchOff(light_num);
            }  
            else if (header.indexOf("GET /flicker?light=") >= 0) {
              String light_string = header.substring(19);
              light_num = light_string.toInt();
              flicker(light_num);
            }  
            else if (header.indexOf("GET /flash?") >= 0) {
              int pos_light = header.indexOf("light=");
              int pos_rate = header.indexOf("rate=");
              if (pos_light >= 0 || pos_rate >= 0)
              {
                // if light is before rate
                if (pos_light < pos_rate){
                  String light_string = header.substring(pos_light+6, pos_rate);
                  light_num = light_string.toInt();
                  String rate_string = header.substring(pos_rate+5);
                  rate_num = rate_string.toInt();
                }
                else {
                  String rate_string = header.substring(pos_rate+5, pos_light);
                  rate_num = rate_string.toInt();
                  String light_string = header.substring(pos_light+6);
                  light_num = light_string.toInt();
            
                }
                flash(light_num, rate_num);
              }
            }  
              

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".buttonoff { background-color: #F01F1F; border: none; color: white; padding: 16px 40px;}");  //red
            client.println(".buttonon { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;}");   //green
            client.println(".buttonflicker { background-color: #ED9D13; border: none; color: white; padding: 16px 40px;}");   //amber
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style>");
            
            // Web Page Heading
            client.println("<body><h1>Remote LEDs</h1>");
            
            // Display current state 
            client.println("<p>Light 1 - State " + ledStatus(1) + "</p>");
            // Display buttons     
            client.println("<p><a href=\"/switchoff?light=1\"><button class=\"buttonoff\">Off</button></a> &nbsp;");
            client.println("<a href=\"/switchon?light=1\"><button class=\"buttonon\">On</button></a> &nbsp;");
            client.println("<a href=\"/flicker?light=1\"><button class=\"buttonflicker\">Flicker</button></a>");
            client.println("</p>");

            
            // Display current state 
            client.println("<p>Light 2 - State " + ledStatus(2) + "</p>");
            // Display buttons     
            client.println("<p><a href=\"/switchoff?light=2\"><button class=\"buttonoff\">Off</button></a> &nbsp;");
            client.println("<a href=\"/switchon?light=2\"><button class=\"buttonon\">On</button></a> &nbsp;");
            client.println("<a href=\"/flicker?light=2\"><button class=\"buttonflicker\">Flicker</button></a>");
            client.println("</p>");


            // Display current state 
            client.println("<p>Light 3 - State " + ledStatus(3) + "</p>");
            // Display buttons     
            client.println("<p><a href=\"/switchoff?light=3\"><button class=\"buttonoff\">Off</button></a> &nbsp;");
            client.println("<a href=\"/switchon?light=3\"><button class=\"buttonon\">On</button></a> &nbsp;");
            client.println("<a href=\"/flicker?light=3\"><button class=\"buttonflicker\">Flicker</button></a>");
            client.println("</p>");


            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}