import sync.DataHandler;

DataHandler handler;
void setup() {
  handler = new DataHandler(
    new String[] {"millis", "num-millis", "null"},
    new String[] {"myMillis", "null", "null"},
    true);
}

void draw(){
  if(!handler.setValue("myMillis", millis())){
    println("Could not set value");
  }
}
