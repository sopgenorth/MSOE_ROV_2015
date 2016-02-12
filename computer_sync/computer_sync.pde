import sync.DataHandler;

DataHandler handler;

String inVars[] = {"tivaData0", "tivaData1", "tivaData2"};
String outVars[] = {"PCdata0", "PCdata1" ,"PCdata2"};

void setup() {
 handler = new DataHandler(
    inVars, //received from device
    outVars, //sent to device
    20, //update rate, in milliseconds
    false, //debug mode
    "192.168.2.217", //device IP
    4545, //device UDP port
    4550); //host UDP port
}

void draw(){
  print("Time since last update: " + handler.getTimeSinceLastUpdate() + " | ");
  
  if(!handler.setValue("PCdata0", millis())){
    println("Could not set value");
  }
  
  try {
    int tivaData0 = handler.getValue("tivaData0");
    println("tivaData0: " + tivaData0);
  } 
  catch(Exception e) {
    e.printStackTrace();
  }
  
}