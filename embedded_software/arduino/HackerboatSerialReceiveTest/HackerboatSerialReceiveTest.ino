void setup(void) {
  Serial.begin(115200);
  Serial1.begin(115200);
  
  while((!Serial) || (!Serial1)) {}
  Serial.println("test");
}

void loop(void) {
    
    if (Serial1.available() > 0) {
        Serial.write(Serial1.read());
    } else {}
}
