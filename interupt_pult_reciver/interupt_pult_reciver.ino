const byte ledPin = 13;
const byte interruptPin = 2;

#define BUF_SIZE 200
const uint32_t END_CODE = 100000;
uint32_t a[BUF_SIZE];

int16_t i=0;
uint32_t timer=0;
bool end_code=false;



enum{ ON_OFF, TEST, TIMER, SET, MIN, ECO, MAX, SOUND };
const uint32_t boiler_commands[] = { 3977408386, 3776868226, 3843714946, 4144525186, 4111101826, 3827003266, 3760156546, 4060966786 };
const String s_boiler_commands[] = { "ON_OFF", "TEST", "TIMER", "SET", "MIN", "ECO", "MAX", "SOUND" };

/*
ED126F82
E11E6F82
E51A6F82
F7086F82
F50A6F82
E41B6F82
E01F6F82
F20D6F82

*/





void setup() {
  Serial.begin(115200);
  Serial.println("begin");
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}
uint32_t code;
void loop() {
	delay(100);
	uint32_t dt = micros() - timer;
  end_code= (i>50 && dt>END_CODE);
  
  if (end_code){
	  Serial.println();
	  int bit = 0;
	  code = 0;
    for(int ii=0; ii<i; ii++){
     // Serial.print(a[ii]);Serial.print(",");
	  if (ii >= 3 && ii&1){
		  if (a[ii]>816)
			  code += (uint32_t)1 << bit;
		  bit++;
	  }
		  
    }
	Serial.println();
	for (i = 0; i < 8; i++){
		if (code == boiler_commands[i]){
			Serial.println(s_boiler_commands[i]);
			break;
		}
	}
	if (i==8)
		Serial.println(code);
    i=0;
    end_code=false;

  }
  
}

void blink() {
  uint32_t t=micros();
  uint32_t delta=t-timer;
  timer=t;
  if (end_code==false)
    if (delta>END_CODE){
       
     }else
        if (i<BUF_SIZE){

          a[i]=delta;
		  
		  i++;
        }
}
