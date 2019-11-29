
   

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


//чтобы включить бойлер надо дать комманду "TEST" а потом "ON-OFF" фактически тест это как выкл

#define IR_CLOCK_RATE    38000L

#define pwmPin 11   // IR Carrier 
#define ledPin 13

float ftime;

enum{SU,MO,TU,WE,TH,FR,SA};

int32_t oldTime = 0;
int32_t  time = 0;   //tiime = 3600*24*7



uint32_t day;



//#define DEBUG_PRINT

#define ALARMS 2
//время включения и отключения бойлера должні начатся и закончится в одних сутках.
const int16_t on_a[] = { 63, B1111111,15,  190, B1111111,25};//  часы умноженые на 10. и дни недели - биты,часи работи умн на 10
const int16_t pereodic[] {120, 4, 1,10}; //часы умноженные на 10 в 11 часов. каждый 4 день. offset,,часи работи умн на 10

const int32_t DAYinSEC = 86400U;
const int32_t HOURinSEC = 3600U;

uint8_t boilerMode = 0;

void setup()  {



	// set the data rate for the Serial port
	day = WE;


	ftime =  (
		19.0 
		* 3600.0) + (
		40.0 
		* 60.0);



	Serial.begin(112500);
	Serial.println("start");


	// toggle on compare, clk/1
	TCCR2A =  _BV(WGM21) | _BV(COM2A0);
	TCCR2B = _BV(CS20);
	// 36kHz carrier/timer
	OCR2A =  (F_CPU / (IR_CLOCK_RATE * 2L) - 1);
	pinMode(pwmPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	TCCR2A = 0;
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
enum {BOILER_OFF,BOILER_ON};


uint32_t time4on;


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

void loop() {

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

	time = ftime;
//	printTime(timer_on);


	


	int32_t for_sleep = DAYinSEC-time;
	boilerMode = BOILER_OFF;
	for (uint8_t i = 0; i<sizeof(on_a); i += 3){
		int32_t atime = (int32_t)on_a[i] * 360U;
		int32_t aday = on_a[i+1];
		int8_t rday = day % 7;
		if (((1 << rday)&aday)>0){
			const int32_t _time4on=(int32_t)on_a[i + 2] * 360U;
			if ((time - _time4on) < atime){
				const int32_t tfor_sleep = (atime - time + 10);
				if (tfor_sleep < for_sleep){
					for_sleep = tfor_sleep;
					boilerMode = BOILER_ON;
					time4on =  (int32_t)on_a[i + 2] * 360U + 10;
					if (time>atime)
						time4on -= time - atime;
#ifdef DEBUG_PRINT
					Serial.println();
					Serial.print("time4sleep:"); Serial.println(for_sleep);
					Serial.print("time4on:"); Serial.println(time4on);
#endif
				}
			}
		}
	}
	
	/*
	uint32_t atime = 360U*(uint32_t)pereodic[0];
	uint32_t period = pereodic[1];
	uint32_t offset = pereodic[2];
	uint32_t work4 = (uint32_t)pereodic[3] * 360U;

	if ( ((day + offset) % period) == 0){
		timer_on |= (
			(time >= atime &&
			(((time<atime) ? time + DAYinSEC : time < (atime + work4)))));
	}
	*/
	//Serial.println(for_sleep);
	if (for_sleep > 0)
		delay(for_sleep*1000U);
	//Serial.println(boilerMode);
	if (boilerMode == BOILER_ON){
		boilerON(true);
		delay(time4on*1000U);
		boilerON(false);
	}

}
