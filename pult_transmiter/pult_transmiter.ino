
   

#define __ 600
#define OD 1200
#define NN 600

const uint16_t boiler_codes[] = { 9084, 4364 };

enum{ ON_OFF, TEST, TIMER, SET, MIN, ECO, MAX, SOUND };
const uint32_t boiler_commands[] = { 3977408386, 3776868226, 3843714946, 4144525186, 4111101826, 3827003266, 3760156546, 4060966786 };
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
//const uint32_t c_on_off = 3977408386;
//const uint32_t c_test = 3776868226;



#define IR_CLOCK_RATE    38000L
#define nightLedPin 10
#define pwmPin 11   // IR Carrier 
#define ledPin 13
#define relePin 12
#define intPin 2

volatile float ftime;



volatile int32_t oldTime = 0;
volatile int32_t  time = 0;   //tiime = 3600*24*7



uint32_t day;



//#define DEBUG_PRINT



const int16_t on_a[] = { 63, -1 ,15,  120, B00100010 , 10, 190, -1,25};


const int32_t DAYinSEC = 86400U;
const int32_t HOURinSEC = 3600U;






void on_off(){
  update_time();
	bool on = digitalRead(intPin);
	if (on == false){
		digitalWrite(relePin, 1);
		digitalWrite(nightLedPin, 0);
	}
	else{
		digitalWrite(nightLedPin,1);
		if(time>25200 && time < 79200)
		  digitalWrite(relePin, 0);

	}
}

void setup()  {



	// set the data rate for the Serial port
	day = 0;


	ftime =  (
		10.0 
		* 3600.0) + (
		6.0 
		* 60.0);


#ifdef DEBUG_PRINT
	Serial.begin(112500);
	Serial.println("start");
#endif

	// toggle on compare, clk/1
	TCCR2A =  _BV(WGM21) | _BV(COM2A0);
	TCCR2B = _BV(CS20);
	// 36kHz carrier/timer
	OCR2A =  (F_CPU / (IR_CLOCK_RATE * 2L) - 1);
	pinMode(pwmPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	TCCR2A = 0;


  pinMode(nightLedPin, OUTPUT);
  pinMode(relePin, OUTPUT);
  on_off();

	digitalWrite(relePin, digitalRead(intPin)); // turn LED OFF
	attachInterrupt(digitalPinToInterrupt(intPin), on_off, CHANGE);
}





String sdays[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };


void printTime(const bool on){
	//////////////////////////////////////////////////////////////
	uint32_t h = time / HOURinSEC;
	uint32_t m = (time - h * HOURinSEC) / 60U;
	uint32_t s = (time - h * HOURinSEC - m * 60U);

	Serial.println();
	Serial.println();
	Serial.print("boiler is "); Serial.println(on ? "ON" : "OFF");
	Serial.print("day: "); Serial.println(sdays[day % 7]);
	Serial.print(h);
	Serial.print(":");
	Serial.print(m);
	Serial.print(":");
	Serial.print(s);
	
	//////////////////////////////////////////////////////////////
}


/*
void send_old(){
	for (int i = 0; i <= 66; i++){
		TCCR2A = ((i & 1) == 0) ? _BV(WGM21) | _BV(COM2A0) : 0;
		delayMicroseconds(boiler_off[i]);
	}
	TCCR2A = 0;

}
*/

void send(const uint32_t code){
	TCCR2A =  _BV(WGM21) | _BV(COM2A0);
	delayMicroseconds(boiler_codes[0]);
	TCCR2A = 0;
	delayMicroseconds(boiler_codes[1]);

	for (uint8_t i = 0; i < 32; i++){
		TCCR2A = _BV(WGM21) | _BV(COM2A0);
		delayMicroseconds(__);
		TCCR2A = 0;
		delayMicroseconds(((code >> i) & 1) ? OD : NN);
	}

}




void boilerON(const bool on){
#ifdef DEBUG_PRINT
	Serial.print("boiler is "); Serial.println(on);
#endif
	digitalWrite(ledPin, on);
	send(boiler_commands[TEST]);
	delay(1000);
	send(boiler_commands[ON_OFF]);
	if (on == false){
		delay(1000);
		send(boiler_commands[ON_OFF]);
	}

}
void update_time(){
	uint32_t dt;
	uint32_t t = millis();
	if (t < oldTime)
		dt = 4294967295 - oldTime + t;
	else
		dt = t - oldTime;
	oldTime = t;


	float fdt = dt;

	fdt *= 0.000999137931;

	ftime += fdt;
	if (ftime >= 86400.0){
		ftime -= 86400.0;
		day += 1;
	}
	day &= 7;

	time = ftime;
}
void loop() {

	
	//	printTime(timer_on);

	update_time();
	for (uint8_t i = 0; i < (sizeof(on_a) / sizeof(uint16_t)); i += 3){
		const int32_t atime = (int32_t)on_a[i] * 360U;
		const uint8_t day_mask = on_a[i + 1];
		const int32_t _time4on = (int32_t)on_a[i + 2] * 360U;
		if ((day_mask&(1 << day)) != 0){

			if (time < (atime + _time4on)){
				const int32_t for_sleep = atime - time;
				const int32_t time4on = (atime>time) ? _time4on : _time4on - time + atime;

#ifdef DEBUG_PRINT
				Serial.println();
				Serial.print("time4sleep:"); Serial.println(for_sleep);
				Serial.print("time4on:"); Serial.println(time4on);
#endif

				if (for_sleep > 0)
					delay(for_sleep * 1000U);
				boilerON(true);
				delay(time4on * 1000U);
				boilerON(false);
				update_time();
			}
		}
		
	}
	delay((86401 - time) * 1000);

}
