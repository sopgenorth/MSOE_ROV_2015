// Processing UDP example to send and receive string data from Arduino 
// press any key to send the "Hello Arduino" message


import hypermedia.net.*;

UDP udp;  // define the UDP object
String deviceIP = "192.168.1.181";
int devicePort = 8888;

double[] inDubs = new double[3];
double[] outDubs = new double[3];
int[] inInts = new int[3];
int[] outInts = new int[3];

void setup() {
  udp = new UDP(this, 88);  // create a new datagram connection on port 88
  udp.log(true);         // <-- printout the connection activity
  udp.listen(true);           // and wait for incoming message  
}

void draw(){
}

void keyPressed() {
  
}

void send(String message){
  udp.send(message, ip, port );
}

void receive( byte[] data ) {          // <-- default handler
  //void receive( byte[] data, String ip, int port ) {   // <-- extended handler
  print("Recieved: ");
  for(int i=0; i < data.length; i++) 
    print(char(data[i]));
  println();
}
