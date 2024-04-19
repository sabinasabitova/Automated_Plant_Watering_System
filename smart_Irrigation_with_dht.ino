#include <Ethernet.h>
#include <SPI.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 2
#define DHTTYPE DHT11
#define sensorPin A3
int timer=0;
float t = -1;
int h = -1;
int z = -1;
int redPin = 6;
int greenPin = 5;
int bluePin = 7;
int relay = 8;

DHT dht(DHTPIN, DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 137, 177);

EthernetServer server(80);

void setup() {
  Serial.begin(9600);
    pinMode(relay, OUTPUT);


  while (!Serial) {
    ; 
  }

  dht.begin();
  lcd.init();  
  lcd.backlight(); 
  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.");
    while (true) {
      delay(1); 
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Сable is not connected.");
  }

  server.begin();
  Serial.print(" ");
  Serial.println(Ethernet.localIP());

  pinMode(redPin,  OUTPUT);              
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  getData();
  listenForEthernetClients();
  lcd.clear();
  lcd.setCursor(0, 0); 

  lcd.print("Temp: "); 
  lcd.print(t);
  char a = 223;
  lcd.print(" ");  
  lcd.print(a);
  lcd.print("C");
  lcd.setCursor(0, 1); 
  int level = readSensor() / 10;
  lcd.print("Moisture: ");  
  lcd.print(level);
  lcd.print(" %");
  if (level < 30) {
    if(timer>3){
    digitalWrite(relay, HIGH); // Turn the relay on 
    setColor(0, 255, 255); //Red  }  
    delay(1000); // Turn the relay Off
    digitalWrite(relay, LOW);
    timer=0;}
    else {
      setColor(0, 255, 255);
      delay(3000);
      timer++;}
  }
  else if (level < 60) {
    setColor(0, 120, 255); //Yellow
  timer++;
  delay(3000);
  }
  else {
    setColor(255, 0, 255); 
    delay(3000);
  }
  

}

void getData() {
  Serial.println("Getting reading");
  int chk = dht.read(DHTPIN);

  // Temperature data
  t = dht.readTemperature();
  
  // Humidity data
  h = dht.readHumidity();

  // Humidity of air data
  z = readSensor() / 10;

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" C");

  Serial.print("Humidity of air: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Moisture of soil: ");
  Serial.print(z);
  Serial.println(" %");

}

int readSensor() {
 int val = analogRead(sensorPin);  // Read value form sensor
 return val;       //Current reading
}

void setColor(int redValue, int greenValue,  int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin,  greenValue);
  analogWrite(bluePin, blueValue);
}

void listenForEthernetClients() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("New client");

    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><meta http-equiv='refresh' content='5'></head>");
          client.println("<body>");

          client.print("Temperature: ");
          client.print(t);
          client.println(" °C<br>");

          client.print("Humidity of air: ");
          client.print(h);
          client.println(" %<br>");
          client.print("Moisture of soil: ");
          client.print(z);
          client.println(" %<br>");
          client.println("</body>");
          client.println("</html>");
          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

 