/*
   Author: Jay Patel, Rohan Mir

   Target Device: Thermostat (HVAC Interface)
   Target Board: STM32
*/

#include <DHT.h> // For use with the DHT sensor

//On the STM32F123C8T6
//PA1 = DHT11 Sensor
//PB13 = Red LED (Compressor)
//PB14 = Yellow LED (Reverse Value)
//PB15 = Green LED (Cooling Fan)

#define GREEN_LED PB12
#define YELLOW_LED PB13
#define RED_LED PB14

#define READ_TIMEOUT 250

const boolean IS_COMMON_ANODE = false;
DHT dht(PA1, DHT11);

int messageResult = 0;
String message = "";

float temperature = 0.0;
float humidity = 0.0;
float tempRange = 0.0;
float targetTemp = 0.0;

bool compressor = false;  //Red LED
bool reverse = false;     //Yellow LED
bool fan = false;         //Green LED

unsigned long sensorNextLoop = 0;
#define SENSOR_LOOP_INTERVAL 5000


void setup() {
  // Set the LED pins to output
  pinMode(PB13, OUTPUT);
  pinMode(PB14, OUTPUT);
  pinMode(PB15, OUTPUT);

  dht.begin();

  Serial.begin(9600);
  Serial2.begin(115200);
}

void loop() 
{
  if (Serial2.available()) 
  {
    messageResult = readIncomingMessage();

    if (messageResult == -1) 
    {
      Serial.println("Error reading message: timed out waiting for new character");
    } 
    else if (messageResult == -2) 
    {
      Serial.println("Error reading message: the read message is empty");
    } 
    else if (messageResult < -2) 
    {
      Serial.println("Error reading message: an unknown error occurred with code ");
      Serial.println(messageResult);
    }

    Serial.print("ESP8266: ");
    Serial.println(message);

    if (messageResult == 1) 
    {
      Serial.println("HVAC Interfaces");
      setHVAC();
    }
  }
  
  if (millis() >= sensorNextLoop) 
  {
    sensorNextLoop = millis() + SENSOR_LOOP_INTERVAL;

    updateTempAndHumid();
  }
}


void updateTempAndHumid() 
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  Serial.println("Read temperature and humidity from sensor");
  
  Serial2.print(temperature);
  Serial2.print(",");
  Serial2.print(humidity);
  Serial2.print("\n");
  //  String sendTo = String(t) + "\r\n"
  //	Serial2.print("32.5/19.7\r\n");
}



void setHVAC() 
{
  Serial.print(compressor);
  Serial.print(", ");
  Serial.print(reverse);
  Serial.print(", ");
  Serial.println(fan);

  digitalWrite(RED_LED, compressor);
  digitalWrite(YELLOW_LED, reverse);
  digitalWrite(GREEN_LED, fan);
}



int readIncomingMessage(void) 
{
  unsigned long millisReadTimeout = millis() + READ_TIMEOUT;
  char currChar = 0;
  String intString = "";
  int numCommas = 0;
  boolean notData = false;

  message = "";

  while (millis() < millisReadTimeout) 
  {
    if (Serial2.available() > 0) 
    {
      currChar = Serial2.read();

      if (currChar == '\n') 
      {
        if (notData) 
        {
          return 0; // code 0 = finished reading log message
        } 
        
        else if (message.length() < 1) 
        {
          return -2; // code -2 = message read is empty
        }

        tempRange = intString.toFloat();
        Serial.print("Read temperature range variable as ");
        Serial.println(tempRange);
        return 1; // code 1 = finished reading and interpreting data message
      } 
      
      else if (!notData) 
      {
        if (isDigit(currChar)) 
        {
          intString += currChar;
        } 
        
        else if (currChar == ',') 
        {
          numCommas++;

          if (numCommas == 1) 
          {
            if (currChar == '0') 
            {
              compressor = false;
            } 
            else if (currChar == '1') 
            {
              compressor = true;
            } 
            else 
            {
              notData = true;
            }
            if (!notData) 
            {
              Serial.print("Read compressor variable as ");
              Serial.println(compressor);
            }
          } 
          
          else if (numCommas = 2) 
          {
            if (currChar == '0') 
            {
              reverse = false;
            } 
            else if (currChar == '1') 
            {
              reverse = true;
            } 
            else 
            {
              notData = true;
            }
            if (!notData) 
            {
              Serial.print("Read reverse variable as ");
              Serial.println(reverse);
            }
          } 
          
          else if (numCommas = 3) 
          {
            if (currChar == '0') 
            {
              fan = false;
            } 
            else if (currChar == '1') 
            {
              fan = true;
            } 
            else 
            {
              notData = true;
            }
            
            if (!notData) 
            {
              Serial.print("Read fan variable as ");
              Serial.println(fan);
            }
          } 
          
          else if (numCommas = 4) 
          {
            targetTemp = intString.toFloat();
            Serial.print("Read target temperature variable as ");
            Serial.println(targetTemp);
          } 
          
          else 
          {
            Serial.println("message is not data: too many commas");
            notData = true;
          }

          intString = "";
        } else {
          Serial.print("message is not data: char '");
          Serial.print(currChar);
          Serial.print("' is not used in data\n");
          notData = true;
        }
      }

      if (currChar != '\r') {
        message += currChar;
        millisReadTimeout = millis() + READ_TIMEOUT;
      }
    } else {
      yield();
    }
  }

  return -1; // code -1 = timed out waiting for new character
}
