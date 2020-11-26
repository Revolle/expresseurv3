int led = 13;
void setup() {
  // put your setup code here, to run once:
  Serial1.begin(31250);
  delay(200);
pinMode(led, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(led, HIGH);
  Serial1.write(0x90);
  Serial1.write(0x40);
  Serial1.write(0x40);
   delay(1000);
digitalWrite(led, LOW); 
  Serial1.write(0x80);
  Serial1.write(0x40);
  Serial1.write(0x40);
  delay(1000);
digitalWrite(led, HIGH);
  Serial1.write(0x90);
  Serial1.write(0x48);
  Serial1.write(0x40);
   delay(2000);
digitalWrite(led, LOW); 
  Serial1.write(0x80);
  Serial1.write(0x48);
  Serial1.write(0x40);
   delay(1000);

}
