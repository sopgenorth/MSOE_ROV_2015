#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

#define IN_NUM 3
#define OUT_NUM 3




/*union ins{
  struct inNames{
    int32_t in1, in2, in3;
  };
  int32_t inNums[3];
} inGroup;*/





struct outNames{
  int32_t o1, o2, o3;
} outGroup;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0x00, 0x1A, 0xB6, 0x02, 0xA4, 0x17};
IPAddress ip(192, 168, 1, 181);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
unsigned byte packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
  
  Serial.begin(9600);
  
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize){
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++){
      Serial.print(remote[i], DEC);
      if (i < 3){
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write("acknowledge");
    Udp.endPacket();
  }
  delay(10);
}

/*
 * Checks the network input buffer, processes all updates
 * Sends updates for all data needed
 *     For now, will send all data values regardless of whether they changed
 */
IPAddress staticRemote;
void handleDataSync(){
  int packetSize = Udp.parsePacket();
  if(packetSize){
    
    /*This code should check to see if the new remote IP is the same as the save staticRemote IP
    //But The reference for the IPAddress struct is shit and I don't know C++ well enough to figure it out
    IPAddress remote = Udp.remoteIP();
    void* IP1 = static_cast<void*>(&staticRemote);
    void* IP2 = static_cast<void*>(&remote);
    if(*IP1 == *IP2){
      
    }*/
    
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    proccessPacket(packetBuffer);
  }
  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(updateOutData());
  Udp.endPacket();
}

/*
 * Disasemble the buffer stream into individual updates
 * Stream format:
 * 0x00 <Data index> <Data>x4  Repeat
 */
void proccessPacket(byte buffer[]){
  int i = 0;
  while(i < sizeof(buffer)/sizeof(byte)){
    if(buffer[i] == 0){
      updateSegment(buffer[i + 1], buffer + i + 2);
      i += 6;
    }else{
      i++;
    }
  }
}

/*
 * Update the peice of data just received. Input format:
 * index: Bit 0-7 : index in the data array
 * data:  pointer to byte array containing the dissasembled
 *         float or int
 */
void updateSegment(byte index, byte * data){
  if(index >= IN_FLOAT_NUM){
    return;
  }
  memcpy(inFloats + index, data, sizeof(inFloats[index]));
}

/*
 * Returns array of all output data formatted to be sent directly over UDP
 */
byte[] updateOutData(){
  byte outStream[(OUT_FLOAT_NUM + OUT_INT_NUM) * (2 + sizeof(int))];
  for(unsigned char i = 0; i < OUT_FLOAT_NUM; i++){
    outStream[i] = 0;
    outStream[i + 1] = i | 0x80;
    memcpy(outStream + i + 2, outFloats + i, sizeof(float));
  }
  for(unsigned char i = 0; i < OUT_INT_NUM; i++){
    outStream[i] = 0;
    outStream[i + 1] = i;
    memcpy(outStream + i + 2, outInts + i, sizeof(int));
  }
  
  
}
