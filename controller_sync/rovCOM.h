#ifndef rovCOM_H
#define rovCOM_H

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

//data received from PC
//modify this struct to fit your needs
struct inNames_t{
  int32_t PCdata0, PCdata1, PCdata2;
};

//data to send to PC
//modify this struct to fit your needs
struct outNames_t{
  int32_t TivaData0, TivaData1, TivaData2;
};

//These are the structs that are used for data
//Read/write to these as desired. 
extern inNames_t inGroup;
extern outNames_t outGroup;

// total number of variables in each of the structs
static const int32_t OUT_NUM = sizeof(outNames_t)/4;
static const int32_t IN_NUM = sizeof(inNames_t)/4;

#define DEBUG false

void setupSync(byte mac[], IPAddress ip, int updateRate, int localPort);
int updateSync();

#endif
