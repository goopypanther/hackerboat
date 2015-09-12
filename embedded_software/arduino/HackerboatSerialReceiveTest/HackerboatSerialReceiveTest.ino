void setup(void) {
  Serial.begin(115200);
  Serial1.begin(115200);
  
  while(!Serial) {}
  while(!Serial1){}
}

void loop(void) {
    Serial1.println("test");
    
    if (Serial1.available() > 0) {
        Serial.write(Serial1.read());
    } else {}
}
