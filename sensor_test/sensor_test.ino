/*
 * Utility sketch for testing sensor board
 * Sets all pins up to LEN as INPUT, then prints the values to serial when they change
 */

#define LEN 46


boolean stat[LEN];
boolean preStat[LEN];

void setup(){
  for(int i = 0; i < LEN; i++){
    pinMode(i, INPUT);
    stat[i] = false;
    preStat[i] = false;
  }
  Serial.begin(9600);
}



void loop(){
  for(int i = 0; i < LEN; i++){
    stat[i] = (digitalRead(i) == HIGH);
  }
  Serial.print("[");
  for(int i = 0; i < LEN; i++){
    if(stat[i] != preStat[i]){
      Serial.print(i);
      Serial.print(":");
      Serial.print(stat[i] ? "1" : "0");
      Serial.print("; ");
      preStat[i] = stat[i];
    }
  }
  Serial.println("]");
  delay(200);
}
