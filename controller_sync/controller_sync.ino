//////////////////////////////////////////////////////////////////////////////////////////////
//*******Dan Miller's super-excellent computer to microcontroller data transfer code!*******//
//////////////////////////////////////////////////////////////////////////////////////////////
/*
Usage guide:
Output:
  Put variable names that are to be sent to the computer in the outNames struct, and update the
  OUT_NUM def to accurately represent how many variables are placed into the struct
Input:
  Put variable names that are to be sent to the computer in the inNames struct, and update the
  IN_NUM def to accurately represent how many variables are placed into the struct

Value Access:
  Use the pre-defined instances of either inNames or outNames(inGroup and outGroup, respecively)
  to access the variables named previously. The inGroup values can be modified, but will be
  overwritten on the next update received from the computer. The outGroup values should be
  modified: again, these are the values which will be sent to the computer side.

Other Constants:
  UPDATE_RATE - the minimum time in ms between updates sent to the server
  DEBUG - true for prints to serial, false otherwise
  LOCAL_PORT - the port to be used by the device
  mac[] - the MAC address for this device. Should be set manually
  ip - the IP to be used for this device

Necessary Method Calls:
  setupSync() - must be called sometime during the setup() function. if DEBUG, initializes Serial
  updateSync() - should be called very often in loop(). Checks for updates in the network buffer,
      and if UPDATE_RATE ms have passed since last update will send an update back to the computer
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define IN_NUM 3
//Used to give names to stored data the has been received
struct inNames{
  int32_t in1, in2, in3;
} inGroup;

#define OUT_NUM 3
//used to give names to stored data to send back
struct outNames{
  int32_t o1, o2, o3;
} outGroup;

#define UPDATE_RATE 100
#define DEBUG true
#define LOCAL_PORT 4545

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
const byte mac[] = {0x00, 0x1A, 0xB6, 0x02, 0xAA, 0xD2};

IPAddress ip(192, 168, 1, 117);


// buffers for receiving and sending data
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
void setupSync(){
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(LOCAL_PORT);
  
  if(DEBUG){
    Serial.begin(115200);
    Serial.println(Ethernet.localIP());
  }
}

//ms between update pings
int time;
void updateSync(){
  int packetSize = Udp.parsePacket();
  if(packetSize){
    // read the packet into packetBufffer
    proccessPacket(packetBuffer, Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE));
  }
  if(time + UPDATE_RATE < millis()){
    time = millis();
    handleDataOut();
  }
}

/*
 * Returns pointer to array of all output data formatted to be sent directly over UDP
 */
const int outSegLen = (2 + sizeof(int32_t));
const int outStreamLen = (OUT_NUM) * outSegLen;
byte outStream[outStreamLen];
void updateOutStream(){
  int * outs = (int*)(&outGroup);
  for(byte i = 0; i < OUT_NUM; i++){
    outStream[i * outSegLen] = 0;
    outStream[(i * outSegLen) + 1] = (byte)(i + 1);
    memcpy(outStream + (i*outSegLen) + 2, outs + i, sizeof(int32_t));
  }
}

/*
 * Checks the network input buffer, processes all updates
 * Sends updates for all data needed
 *     For now, will send all data values regardless of whether they changed
 */
void handleDataOut(){
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  updateOutStream();
  
  Udp.write(outStream, outStreamLen);
  Udp.endPacket();
}

/*
 * Disasemble the buffer stream into individual updates
 * Stream format:
 * 0x00 <Data index + 1> <Data>x4  Repeat
 * buffer: contains raw byte stream from UDP
 */
void proccessPacket(byte buffer[], int size){
  int i = 0;
  while(i < size){
    if(buffer[i] == 0 && (size - i) >= 6 && buffer[i+1] <= IN_NUM){
      updateSegment(buffer[i + 1] - 1, buffer + i + 2);
      i += 6;
    }else{
      i++;
    }
  }
  if(DEBUG){
    int * ins = (int*)(&inGroup);
    for(byte i = 0; i < IN_NUM; i++){
      Serial.print(*(ins + i));
      Serial.print(", ");
    }
    Serial.println();
  }
}

/*
 * Update the peice of data just received. Input format:
 * index: Bit 0-7 : index in the data array
 * data:  pointer to byte array containing the dissasembled
 *         float or int
 */
void updateSegment(byte index, byte * data){
  if(index >= IN_NUM){
    return;
  }
  memcpy((int*)(&inGroup) + index, data, sizeof(int32_t));
}
//////////////////////////////////////////////////////////////////////////////////////
//**********************HARDWARE CONTROL CODE GOES BELOW HERE***********************//
//////////////////////////////////////////////////////////////////////////////////////



void setup() {
  setupSync();
}

void loop(){
  // if there's data available, read a packet
  outGroup.o1 = millis();
  outGroup.o2 = 0xFFFFFF - millis();
  updateSync();
}
