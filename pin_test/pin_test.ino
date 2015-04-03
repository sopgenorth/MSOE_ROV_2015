/*
 * Utility sketch for testing sensor board pins
 * Sets all pins between numbers in ranges[] to Output, then blinks all of them
 */


int ranges[] = {0, 46, 20, 46};
int rangeNum = 2;

void setup(){
  for(int b = 0; b < rangeNum; b++)
    for(int i = ranges[b * 2]; i < ranges[b * 2 + 1]; i++)
      pinMode(i, OUTPUT);
  Serial.begin(4800);
}

void loop(){
  for(int b = 0; b < rangeNum; b++)
    for(int i = ranges[b * 2]; i < ranges[b * 2 + 1]; i++)
      digitalWrite(i, HIGH);
  delay(200);
  for(int b = 0; b < rangeNum; b++)
    for(int i = ranges[b * 2]; i < ranges[b * 2 + 1]; i++)
      digitalWrite(i, LOW);
  delay(200);
  Serial.write("b");
}
