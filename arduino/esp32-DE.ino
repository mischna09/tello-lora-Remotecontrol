/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */ 
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiUdp.h>
//TELLO-D8687F
// WiFi network name and password:
HardwareSerial MySerial(1);
const char * networkName = "TELLO-D8687F";
const char * networkPswd = "";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
//(192.168.10.1 , 8889)
const char * udpAddress = "192.168.10.1";
const int udpPort = 8889;

//Are we currently connected?
boolean conn = false;

//The udp library class
WiFiUDP udp;

void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);
  MySerial.begin(115200, SERIAL_8N1, 9, 10);
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
  while(!conn)
  {
    delay(7000);
    if(!conn)
    connectToWiFi(networkName, networkPswd);
  }
  Serial.println("Setup End...");
}
#define STATE_IDLE  1
#define STATE_RECEIVING 2
#define STATE_COMPLETE 3
int incoming;
int state = STATE_IDLE;
int str_len = 0;
char buf[100], temp[3];
void loop(){
  // If there is incoming data from MySerail, Read a byte
  if(state == STATE_IDLE || state == STATE_RECEIVING) {
    if (MySerial.available()) {
      state = STATE_RECEIVING;
      incoming = MySerial.read();
      sprintf(temp, "%c", incoming);
      Serial.print(temp);
      buf[str_len++] = incoming;
      if (incoming == 0x0A) {
        state = STATE_COMPLETE;
        buf[str_len] = '\0';
      }
    }
  }
  if(conn)
  {
    char *pdata;
    if(state == STATE_COMPLETE) {
      Serial.print("Get string: ");
      Serial.print(buf);
      // Parse the incoming string (from LoRa module)
      if(strncmp("+R", buf, 2)==0) { // Case: "+RCV......"
        // retrieve the data part from the incoming string
        udp.beginPacket(udpAddress,udpPort);
        udp.printf("command");
        udp.endPacket();
        strtok(buf, ",");
        strtok(NULL, ",");
        pdata = strtok(NULL,","); // the pointer of the desired data
        int num = (int)*pdata-48;
        //Serial.print(num);
        switch(num)
        {
          case 1: //起
          {
            pdata="takeoff";
            break;
          }
          case 2: //降
          {
            pdata="land";
            break;
          }
          case 3: //setA 向前飛120cm,4秒後向後120cm
          {
            udp.beginPacket(udpAddress,udpPort);
            udp.printf("forward 120");
            udp.endPacket();
            delay(4000);
            pdata="back 120";
            break;
          }
          case 5: //setB 轉90度
          {
            pdata="ccw 90";
            break;
          }
          case 6: //battery?
          {
            pdata="battery?";
            break;
          }
          case 7: //forward
          {
            pdata="forward 500";
            break;
          }
          case 8: //左轉 30度
          {
            pdata="ccw 30";
            break;
          }
          case 9: //右轉 30度
          {
            pdata="cw 30";
            break;
          }
          case 10: //後退
          {
            pdata="back 200";
            break;
          }
          case 11: //上升
          {
            pdata="up 30";
            break;
          }
          case 12: //下降
          {
            pdata="down 30";
            break;
          }
        }
        /*if(strncmp("A1",pdata,2)==0)//command to drone:take off
          pdata="takeoff";
        else if(strncmp("A2",pdata,2)==0)//command to drone:land
          pdata="land";*/
          
        udp.beginPacket(udpAddress,udpPort);
        udp.printf(pdata);
        udp.endPacket();
      }
      str_len = 0;
      state = STATE_IDLE;
    }
    int len;
    len = 0;
    len = udp.parsePacket();
    if(len>0) {
      char lora_command_str[100];
      Serial.print("packet size:");
      Serial.println(len);
      char * buf = new char[146];
      udp.read(buf,len);
      buf[len] = '\0';
      Serial.print(buf);
      sprintf(lora_command_str, "AT+SEND=0,10,%s\r\n", buf);
      if(strncmp("ok", buf, 2)!=0){
        MySerial.print(lora_command_str);
      }
      delete[] buf;
     }
/*    while (Serial.available() > 0){
        udp.beginPacket(udpAddress,udpPort);
        udp.printf("command");
        udp.endPacket();
        udp.beginPacket(udpAddress,udpPort);
        udp.printf("land");
        udp.endPacket();
    }
*/
  }
  else { 
    connectToWiFi(networkName, networkPswd);
    //Wait for 10 second
    delay(10000);
  }
}
void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}
//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          //udp.begin(WiFi.localIP(),udpPort);
          conn = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          conn = false;
          break;
    }
}
