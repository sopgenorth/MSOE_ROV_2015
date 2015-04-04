#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

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
byte mac[] = {0x00, 0x1A, 0xB6, 0x02, 0xAA, 0xD2};

//TODO: figure out how to find the IP of this device dynamically
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
