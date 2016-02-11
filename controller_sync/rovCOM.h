#ifndef rovCOM_H
#define rovCOM_H

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

//data received from PC
struct inNames_t{
  int32_t m0,m1,m2,m3,m4,m5,m6,m7; //8 motors/thrusters, speed range: -3200 to 3200
  int32_t PID_Kp, PID_Ki, PID_Kd, PID_dir, PID_setPoint;
  int32_t PID_Kp_low, PID_Ki_low, PID_Kd_low;
  int32_t PIDgainSwitchPoint_uBar;
  int32_t depthFilter;
  int32_t PIDenable;
};

//data to send to PC
struct outNames_t{
  int32_t depth_microBar, depth_microBarRaw, rovTempMilliCelsius, PID_output, rovCycleTime_us;
  int32_t rov48V, rov48I, rov12V;
};

// total number of variables in each of the structs
static const int32_t OUT_NUM = sizeof(outNames_t)/4;
static const int32_t IN_NUM = sizeof(inNames_t)/4;

extern inNames_t inGroup;
extern outNames_t outGroup;


#define DEBUG false

void setupSync(byte mac[], IPAddress ip, int updateRate, int localPort);
int updateSync();

#endif
