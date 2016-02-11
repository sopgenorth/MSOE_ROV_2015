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
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "arch/lwiplib.h"
#include "rovCOM.h"

#define ROM_APITABLE    ((uint32_t *)0x01000010)
#define ROM_EMACTABLE   ((uint32_t *)(ROM_APITABLE[42]))
#define ROM_UpdateEMAC  ((void (*)(uint32_t ui32Clock))ROM_EMACTABLE[71])

#define FIRMWARE_UPDATE_UDP_PORT 9
#define FIRMWARE_PACKET_LENGTH 30


#define UDP_TX_PACKET_MAX_SIZE 256

inNames_t inGroup;
outNames_t outGroup;

int UPDATE_RATE;
int LOCAL_PORT;

// buffers for receiving and sending data
byte packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
EthernetUDP Udp_firmwareUpdate;

//function prototypes
void updateSegment(byte index, byte * data);
void proccessPacket(byte buffer[], int size);
void handleDataOut();
void updateOutStream();
void SoftwareUpdateBegin(uint32_t ui32SysClock);

void setupSync(byte mac[], IPAddress ip, int updateRate_ms, int localPort){
  //set global variables
  UPDATE_RATE = updateRate_ms;
  LOCAL_PORT = localPort;
  
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(LOCAL_PORT);
  
  //initialize UDP port for listening for firmware updates
  Udp_firmwareUpdate.begin(FIRMWARE_UPDATE_UDP_PORT);
  
  Ethernet.enableActivityLed();
  Ethernet.enableLinkLed();

# if(DEBUG)
    Serial.begin(115200);
    Serial.println(Ethernet.localIP());
# endif

}

//Function to check if new UDP data is available and to send data cyclically
//Returns 1 if data was received
//Returns 0 if there was no new data
// TODO: Move the data output to a timer interrupt
int updateSync(){
  int dataReceived = 0;
  int packetSize = Udp.parsePacket();
  static int time = 0; //ms between updates
  
  if(packetSize){
    // read the packet into packetBufffer
    proccessPacket(packetBuffer, Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE));
    dataReceived = 1;
  }

  //TODO: Move this to use a timer interrupt instead of polling millis()
  if(time + UPDATE_RATE < millis()){
    time = millis();
    handleDataOut();
  }
  
  //check if a firmware update packet has been sent
  if(Udp_firmwareUpdate.parsePacket() == FIRMWARE_PACKET_LENGTH){ 
    SoftwareUpdateBegin(SysCtlClockGet());
  }

  return dataReceived;
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
    }
    else{
      i++;
    }
  }
#if(DEBUG)
    int * ins = (int*)(&inGroup);
    for(byte i = 0; i < IN_NUM; i++){
      Serial.print(*(ins + i));
      Serial.print(", ");
    }
    Serial.println();
#endif
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


//copied from TI's swupdate.c file provided in examples and TivaWare
//*****************************************************************************
//
//! Passes control to the bootloader and initiates a remote software update
//! over Ethernet.
//!
//! This function passes control to the bootloader and initiates an update of
//! the main application firmware image via BOOTP across Ethernet.  This
//! function may only be used on parts supporting Ethernet and in cases where
//! the Ethernet boot loader is in use alongside the main application image.
//! It must not be called in interrupt context.
//!
//! Applications wishing to make use of this function must be built to
//! operate with the bootloader.  If this function is called on a system
//! which does not include the bootloader, the results are unpredictable.
//!
//! \note It is not safe to call this function from within the callback
//! provided on the initial call to SoftwareUpdateInit().  The application
//! must use the callback to signal a pending update (assuming the update is to
//! be permitted) to some other code running in a non-interrupt context.
//!
//! \return Never returns.
//
//*****************************************************************************
void SoftwareUpdateBegin(uint32_t ui32SysClock)
{
    //
    // Disable all processor interrupts.  Instead of disabling them
    // one at a time (and possibly missing an interrupt if new sources
    // are added), a direct write to NVIC is done to disable all
    // peripheral interrupts.
    //
    HWREG(NVIC_DIS0) = 0xffffffff;
    HWREG(NVIC_DIS1) = 0xffffffff;
    HWREG(NVIC_DIS2) = 0xffffffff;
    HWREG(NVIC_DIS3) = 0xffffffff;
    HWREG(NVIC_DIS4) = 0xffffffff;

    //
    // Also disable the SysTick interrupt.
    //
    SysTickIntDisable();
    SysTickDisable();

    //
    // Return control to the boot loader.  This is a call to the SVC
    // handler in the flashed-based boot loader, or to the ROM if configured.
    //
    ROM_UpdateEMAC(ui32SysClock);
}
