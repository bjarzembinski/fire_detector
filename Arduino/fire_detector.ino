//zmeinne dla stanow 
bool led_ready = true;
bool fire_check = false; 
bool explo_check = false;
bool fire = false;
bool explo = false;

void setup() {
//otwarcie komunikacji po porcie szeregowym
Serial.begin(9600);
pinMode(5, OUTPUT); //zielona dioda ,,ready''
pinMode(6, OUTPUT); //czerwona dioda ,,FIRE''
pinMode(7, OUTPUT); //zolta dioda ,,EXPLOSION''
pinMode(9, OUTPUT); //biala dioda ,,chceck explosion''
pinMode(10, OUTPUT); //biala dioda ,,chceck fire''
pinMode(A5, OUTPUT); //buzzer
}

void loop() {
  //odczyt z portu
int data = Serial.read();

  if(data == 'A')
      fire_check = true;
    
  if(data == 'B')
      explo_check = true;

  if(data == 'C')
      fire = true;

  if(data == 'D')
      explo = true;
      
    if(data == 'E'){
      led_ready = true;
      fire_check = false;
      explo_check = false;
      fire = false;
      explo = false;}
//kontrolka pozaru
if (fire_check == true)
digitalWrite(10, HIGH);
else
digitalWrite(10, LOW);
//kotrolka eksplozji
if (explo_check == true)
digitalWrite(9, HIGH);
else
digitalWrite(9, LOW);
//pozar
if (fire == true){
digitalWrite(A5, HIGH);
digitalWrite(6, HIGH);}
else{
digitalWrite(A5, LOW);
digitalWrite(6, LOW);}
//eksplozja
if (explo == true)
digitalWrite(7, HIGH);
else
digitalWrite(7, LOW);
//stan gotowosci
if (led_ready == true)
digitalWrite(5, HIGH);
else{
digitalWrite(6, LOW);
digitalWrite(7, LOW);
digitalWrite(A5, LOW);}
}
