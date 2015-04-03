// Processing UDP example to send and receive string data from Arduino 
// press any key to send the "Hello Arduino" message


import hypermedia.net.*;

UDP udp;  // define the UDP object
//TODO: figure out how to find the IP of device dynamically
String deviceIP = "192.168.1.114";
int devicePort = 8888;

int[] inNums = new int[3];
int[] outNums = new int[3];

void setup() {
  udp = new UDP(this, devicePort);  // create a new datagram connection on port 88
  udp.log(true);            // <-- printout the connection activity
  udp.listen(true);         // and wait for incoming message  
}

void draw(){
}

void keyPressed() {
  send("Hello world");
}

void send(String message){
  udp.send(message, deviceIP, devicePort);
}

void receive( byte[] data ) {          // <-- default handler
  //void receive( byte[] data, String ip, int port ) {   // <-- extended handler
  print("Recieved: ");
  for(int i=0; i < data.length; i++) 
    print(char(data[i]));
  println();
}
