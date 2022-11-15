
#include <mcp_can.h>
#include <SPI.h>

#define CAN0_INT 2                          // Set INT to pin 2
MCP_CAN CAN0(10);                           // Set CS to pin 10

unsigned int curveCommand;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

const int steerSwitch = A1;   
int steerState = 0; 

const int pot = A0;   
int potValue = 0; 
unsigned int setCurve = 0;

byte data[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} ;          

unsigned int scanLoopInterval  = 80;
unsigned long scanLoopMs = millis();

//---------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  pinMode(steerSwitch,INPUT_PULLUP);

  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  }
  else {
    Serial.println("Error Initializing MCP2515...");
  }

  CAN0.init_Mask(0,1,0xFFFFFF);         // Init first mask...    
  CAN0.init_Filt(0,1,0x0CACAA08);       // Init first filter...  
  CAN0.init_Filt(1,1,0x0CACAA08);
  
  CAN0.init_Mask(1,1,0xFFFFFF);         // Init second mask... 
  CAN0.init_Filt(2,1,0x0CACAA08);       // Init third filter...
  CAN0.init_Filt(3,1,0x0CACAA08);       // Init fouth filter...
  CAN0.init_Filt(4,1,0x0CACAA08);       // Init fifth filter...
  CAN0.init_Filt(5,1,0x0CACAA08);       // Init sixth filter...

  CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);

  delay (1000);
 
      Serial.println("CNH CANBUS Steering Test");          
}

//---------------------------------------------------------------------------------------------------------------------------------

void loop()
{

potValue = analogRead(pot);
potValue = map(potValue,0,1023,0,4000);
setCurve = (potValue + 30128);

steerState = digitalRead(steerSwitch);
if (steerState == 0)steerState = 252;   //BIN 11111100
if (steerState == 1)steerState = 253;   //BIN 11111101


  if ((millis() - scanLoopMs) > scanLoopInterval) {   // Send data
    scanLoopMs = millis();
    data[0] = lowByte(setCurve);
    data[1] = highByte(setCurve);
    data[2] = steerState;
    data[3] = 255;
    data[4] = 255;
    data[5] = 255;
    data[6] = 255;
    data[7] = 255;
    sendCAN_Msg();
  }

if(!digitalRead(2)) {
  readCAN_Msg();
}
  
}

//---------------------------------------------------------------------------------------------------------------------------------

void sendCAN_Msg() {

byte sndStat = CAN0.sendMsgBuf(0x8CAD08AA, 1, 8, data);
 /* if (sndStat == CAN_OK) {
    Serial.print(potValue);
    Serial.print("'");
    Serial.print(setCurve);
    Serial.print("'");
    Serial.print(steerState);
    Serial.print("'");
    Serial.println(" Sent Successfully!");
  }
  else {
    Serial.print(potValue);
    Serial.print("'");
    Serial.print(setCurve);
    Serial.print("'");
    Serial.print(steerState);
    Serial.print("'");
    Serial.println("Error Sending Message...");
  }*/
}

//---------------------------------------------------------------------------------------------------------------------------------

void readCAN_Msg() {
  
      CAN0.readMsgBuf(&rxId, &len, rxBuf); 
      Serial.print("ID: ");
      Serial.print(rxId, HEX);
      Serial.print(" Data: ");
      for(int i = 0; i<len; i++)           
      {
        Serial.print(rxBuf[i], DEC);
        Serial.print(", ");
      }

      Serial.print(" Current Curve = ");
      curveCommand = ((rxBuf[1] << 8) + rxBuf[0]);
      Serial.print(curveCommand);
      Serial.print(" Current Valve Status = ");
      if ((rxBuf[2]) == 20)Serial.print("Auto Steering"); 
      if ((rxBuf[2]) == 16)Serial.print("Ready to Steer");
      if ((rxBuf[2]) == 0)Serial.print("Not Ready");
      if ((rxBuf[2]) == 80)Serial.print("Reset / Stop Auto Steering");
      Serial.println();

}
