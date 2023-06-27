//libraries
  #include "WiFi.h" //wifi connection
  #include <Arduino.h> //
  #include "ThingSpeak.h" //to handle thingspeak
  #include <WiFiClient.h> // for HTTP requests
  #include <WiFiAP.h> // wifi 
  #include "OneWire.h" // temp sensor
  #include "DallasTemperature.h" // temp sensor
  #include <Adafruit_SH1106.h> // oled
  #include <Adafruit_GFX.h> //oled

  
  // ThingSpeak Channel ID and Api Keys
  #define CHANNEL_ID 1726478
  #define CHANNEL_API_KEY "U0TZ9AJ1ESPJKTBF"
  
  // tds And temp
  
  OneWire oneWire(27);
  DallasTemperature tempSensor(&oneWire);
  #define tdssensorPin 35
  
  //PH pin 

  const int potPin=34;

  int tdssensorValue = 0;
  float tdsValue = 0;
  float Voltage = 0;
  float Temp=0;
  float ph;
  float Value=0;

  // motor pins
  int pump1 = 16;  
  int pump2 = 17; 

  // oled pins

  #define OLED_SDA 21
  #define OLED_SCL 22
  
  Adafruit_SH1106 display(21, 22);
  
  #define NUMFLAKES 10
  #define XPOS 0
  #define YPOS 1
  #define DELTAY 2
  
  #if (SH1106_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SH1106.h!");
  #endif
  
  WiFiClient client;
  
  // Wifi Details
  
  const char *ssid = "abcd";
  const char *password = "abcd";

  // HTTP Client Object
    HTTPClient http;

  // Server URL
  String server = "http://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;
  String path = server + String() + OM2M_AE + "/" + String()+OM2M_Node_ID + String()+OM2M_DATA_CONT ;
  
  void setup()
  {
      pinMode(pump1, OUTPUT);
      pinMode(pump2, OUTPUT);
      pinMode(potPin,INPUT);
      pinMode(OLED_SDA, INPUT_PULLUP);           
      pinMode(OLED_SCL, INPUT_PULLUP);          
      Serial.begin(115200);
      
      WiFi.localIP();
      WiFi.begin(ssid, password);
      
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.println("Connecting to WiFi..");
      }
      
        Serial.println("Connected to the WiFi network");
        Serial.println(WiFi.localIP());

        display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
        display.display();
        delay(1000);
        
        tempSensor.begin();
        ThingSpeak.begin(client);
  }
  
  void loop()
  {

  //cleaning water before to get data 
   digitalWrite(pump2, LOW);      //Switch Solenoid ON
   delay(3000);
   Serial.println("motor 2 high");//Wait 1 Second
   delay(1000);                    //Wait 1 Second
   digitalWrite(pump2, HIGH);     //Switch Solenoid OFF Serial.println("hi");
   Serial.println("motor 2 low");//Wait 1 Second
   delay(3000); 
   
    //Getting water in to tank 
    
    digitalWrite(pump1,LOW);
    Serial.println("motor 1 high");//Switch motor ON
    delay(3000);                          //Wait 1 Second
    digitalWrite(pump1, HIGH);
    Serial.println("motor 1 low");//Switch motor OFF
    delay(3000);  
  

    //sensors data 

    //temp and tds value
      tdssensorValue = analogRead(tdssensorPin);
      Voltage = tdssensorValue*3.3/1024.0; //Convert analog reading to Voltage
      tdsValue=(133.42*Voltage*Voltage*Voltage - 255.86*Voltage*Voltage + 857.39*Voltage)*0.5; //Convert voltage value to TDS value
      Serial.print("TDS Value = ");  
      Serial.print(tdsValue);
      Serial.println(" ppm");
     tempSensor.requestTemperaturesByIndex(0);
     
    Temp=tempSensor.getTempCByIndex(0); 
    
    Serial.print("Temperature: ");
    Serial.print(Temp);
    Serial.println(" C");


   // cleaning tank 
    digitalWrite(pump2, LOW);      //Switch Solenoid ON
   delay(3000);
   Serial.println("motor 2 high");//Wait 1 Second
   delay(1000);                    //Wait 1 Second
   digitalWrite(pump2, HIGH);     //Switch Solenoid OFF Serial.println("hi");
   Serial.println("motor 2 low");//Wait 1 Second
   delay(3000); 
   //ph sensor
    Value= analogRead(potPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    Serial.println(ph);
    delay(500);


  //turbidity sensor
  int turbidity = analogRead(A0);
  float turbidityVoltage = turbidity * (3.3 / 4095.0);
 
  Serial.println ("turbidity (V):");
  Serial.println (turbidityVoltage);
  Serial.println();
  delay(1000);
  //thingspeak 

    ThingSpeak.setField(1,tdsValue);
    ThingSpeak.setField(2,Temp);


    ThingSpeak.writeFields(CHANNEL_ID,CHANNEL_API_KEY);

    delay (1000);
  //onem2m code
  // Required Headers
  http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  http.addHeader("Content-Type", "application/json;ty=4");
   String data =   receive_data;
  Serial.println(data);
  String req_data = String() + "{\"m2m:cin\": {"
    +
    "\"con\": \"" + data + "\","
    +
    "\"lbl\": \"" + "Node-1" + "\","
    +
    "\"cnf\": \"text\""
    +
    "}}";
  int code = http.POST(req_data);
  http.end();
  if (code == 201) {
    Serial.println("Data Posted Successfully");
  } else {
    Serial.println("Data posting failed with http code-" + String(code));
  }
  delay(1000);
  }
    
  //oled display
      
        // display node name in round rect
    display.setCursor(0,0);
    display.fillRoundRect(0, 2, 128, 64, 8, BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(3);
    display.setCursor(6,10);
    display.println("Water Quality");
    display.display();
    delay(1000);

  // Temperarture
  display.clearDisplay();

  display.setCursor(32,0);
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println("Temp:");

  display.setCursor(10,32);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(Temp);

  display.setCursor(75,32);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("°C");

  display.display();
  delay(1000);
  
// TDS

    display.clearDisplay();


    display.setCursor(32,0);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("TDS:");

    display.setCursor(10,22);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println(tdsValue);

    display.setCursor(50,45);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("PPM");

    display.display();
    delay(1000);
   
  //ph
    display.clearDisplay();


    display.setCursor(32,0);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("PH:");

    display.setCursor(10,22);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println(ph);

    display.setCursor(50,45);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println(".");

    display.display();
    delay(1000);

  //turbidity
  
    display.clearDisplay();


    display.setCursor(32,0);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("Turbidity");

    display.setCursor(10,22);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println(turbidityVoltage);

    display.setCursor(50,45);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("NTU");

    display.display();
    delay(1000);
  
  }
