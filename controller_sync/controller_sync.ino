#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "rovCOM.h"

/*
 * Intializes the data handler
 */
void setup() {
  
//this needs to match the board's MAC for Ethernet firmware updates to work  
byte mac[] = {0x00, 0x1A, 0xB6, 0x02, 0xF4, 0xCC};
  setupSync(mac, IPAddress(192, 168, 2, 217), 20, 4545);
}

/*
 * Main loop that is called continuously. 
 */
void loop(){
  //reads a packet if available. 
  //This updates the two structs defined in "rovCOM.h"
  boolean newSyncData = updateSync();
}





