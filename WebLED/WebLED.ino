/*
Arduino Due - ESP 8266 WiFi Module
Serial (Tx/Rx) communicate to PC via USB
Serial3 (Tx3/Rx3) connect to ESP8266
Tx3 - ESP8266 Rx
Rx3 - ESP8266 Tx
ESP8266 CH_PD Connect to ESP8266 VCC
*/

#define ESP8266 Serial3
String SSID = "PTCL-805";
String PASSWORD = "11223344";

int LED = 13;
boolean LEDst = false;

//always high
int CH_PD_8266 = 53;
boolean FAIL_8266 = false;
int i;
void setup() {

    Serial.begin(115200);
    ESP8266.begin(115200);
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LEDst);
    
    pinMode(CH_PD_8266, OUTPUT);
    digitalWrite(CH_PD_8266, LOW);
    delay(1000);
    digitalWrite(CH_PD_8266, HIGH);
    delay(1000);
    ESP8266.println("AT+RST");
    delay(1000);
    
    do{
    
    //Wait Serial Monitor to start
    while(!Serial);
    Serial.println("--- Start ---");
    //ESP8266.println("AT+RST");
    delay(1000);
    ESP8266.println("AT");
    delay(1000);
    if(ESP8266.find("OK"))
    {
      Serial.println("Module is ready");
      ESP8266.println("AT+CWMODE=1");
      delay(2000);
      
      //Quit existing AP, for demo
      Serial.println("Quit AP");
      ESP8266.println("AT+CWQAP");
      delay(1000);
      
      clearESP8266SerialBuffer();
      if(cwJoinAP())
      {
        Serial.println("CWJAP Success");
        FAIL_8266 = false;
        
        delay(3000);
        clearESP8266SerialBuffer();
        //Get and display my IP
        sendESP8266Cmdln("AT+CIFSR", 1000);  
        //Set multi connections
        sendESP8266Cmdln("AT+CIPMUX=1", 1000);
        //Setup web server on port 80
        sendESP8266Cmdln("AT+CIPSERVER=1,80",1000);
        
        Serial.println("Server setup finish");
      }else{
        Serial.println("CWJAP Fail");
        delay(500);
        FAIL_8266 = true;
      }
    }else{
      Serial.println("Module have no response.");
      delay(500);
      FAIL_8266 = true;
    }
  }while(FAIL_8266);
  
  digitalWrite(LED, HIGH);
}

void loop(){
  if(ESP8266.available()) 
  {
    Serial.println("Something received");
    delay(1000);
    if(ESP8266.find("+IPD,"))
    {
      String action;
      
      Serial.println("+IPD, found");
      int connectionId = ESP8266.read()-48;
      Serial.println("connectionId: " + String(connectionId));
      
      ESP8266.find("led=");
      char s = ESP8266.read();
      if(s=='0'){
        action = "led=0";
        digitalWrite(LED, LOW);
      }else if(s=='1'){
        action = "led=1";
        digitalWrite(LED, HIGH);
      }else{
        action = "led=?";
      }
      
      Serial.println(action);
      sendHTTPResponse(connectionId, action);
      
      //Close TCP/UDP
      String cmdCIPCLOSE = "AT+CIPCLOSE="; 
      cmdCIPCLOSE += connectionId;
      sendESP8266Cmdln(cmdCIPCLOSE, 1000);
    }
  }
}

void sendHTTPResponse(int id, String content)
{
  String response;
  response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/html; charset=UTF-8\r\n"; 
  response += "Content-Length: ";
  response += content.length();
  response += "\r\n";
  response +="Connection: close\r\n\r\n";
  response += content;

  String cmd = "AT+CIPSEND=";
  cmd += id;
  cmd += ",";
  cmd += response.length();
  
  Serial.println("--- AT+CIPSEND ---");
  sendESP8266Cmdln(cmd, 1000);
  
  Serial.println("--- data ---");
  sendESP8266Data(response, 1000);
}

boolean waitOKfromESP8266(int timeout)
{
  do{
    Serial.println("wait OK...");
    delay(1000);
    if(ESP8266.find("OK"))
    {
      return true;
    }

  }while((timeout--)>0);
  return false;
}

boolean cwJoinAP()
{
  String cmd="AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  ESP8266.println(cmd);
  return waitOKfromESP8266(10);
}

//Send command to ESP8266, assume OK, no error check
//wait some time and display respond
void sendESP8266Cmdln(String cmd, int waitTime)
{
  ESP8266.println(cmd);
  delay(waitTime);
  clearESP8266SerialBuffer();
}

//Basically same as sendESP8266Cmdln()
//But call ESP8266.print() instead of call ESP8266.println()
void sendESP8266Data(String data, int waitTime)
{
  ESP8266.print(data);
  delay(waitTime);
  clearESP8266SerialBuffer();
}

//Clear and display Serial Buffer for ESP8266
void clearESP8266SerialBuffer()
{
  Serial.println("= clearESP8266SerialBuffer() =");
  while (ESP8266.available() > 0) {
    char a = ESP8266.read();
    Serial.write(a);
  }
  Serial.println("==============================");
}
