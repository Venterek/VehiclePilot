// Kod pilota drona do wersji 1.0
//
// Author: Jan Wielgus
// Date: 26.10.2017r.
//

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Communication.h"
#include "ControlPanelApp.h"
#include "CustomDiodeLib.h"
#include "config.h"

//SoftwareSerial software_serial(tx_pin, rx_pin); // zapasowy dla mudulu BT
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
CustomDiodeLibClass red(redDiodePin, true);
CustomDiodeLibClass green(greenDiodePin, true);


void setup()
{
	#ifdef _INO_DEBUG
		Serial.begin(9600);
		Serial.println("INO DEBUG has sterted");
#endif

	lcd.init(); // I2C
	com.init();
	
	// init diodes
	red.init();
	green.init();
	
	cpa.init();
	//Wire.setClock(400000L); // 400kHz  DO PRZETESTOWANIA !!! jak zadziala to uzyc z tym
	
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("Pilot drona :)");
}

void loop()
{
	// <<<<< ====== ---  GLOWNE  --- ====== >>>>>
	
	//kom.odbierz(); // nie dziala przy starych radiach
	
	#ifdef USE_PC_APP
		static int32_t lastCpaRTime = 0; // czas oststniego odebrania danych od posrednika I2C pc app
		if (millis()-lastCpaRTime > 330) // odbieranie duzej paczki
		{
			cpa.odbierz(); // w config ustawia sie czy dziala przez UART0 czy I2C
			lastCpaRTime = millis();
		}
	#endif
	
// Obliczanie drazkow
	static float lastThrottle=0;
	static float lastRotate=0;
	static float lastTiltTB=0;
	static float lastTIltLR=0;
	// EWA filters
	lastThrottle = STEERING_FILTER_BETA*lastThrottle + (1-STEERING_FILTER_BETA)*analogRead(pinThrottle);
	lastRotate = STEERING_FILTER_BETA*lastRotate + (1-STEERING_FILTER_BETA)*analogRead(pinRotate);
	lastTiltTB = STEERING_FILTER_BETA*lastTiltTB + (1-STEERING_FILTER_BETA)*analogRead(pinTiltTB);
	lastTIltLR = STEERING_FILTER_BETA*lastTIltLR + (1-STEERING_FILTER_BETA)*analogRead(pinTiltLR);
	// maps ans constrains (final step)
	com.pilot.throttle = constrain(map(long(lastThrottle), 960, 65, 0, 1000), 0, 1000);
	com.pilot.rotate = constrain(map(long(lastRotate), 968, 50, -450, 450), -450, 450);
	com.pilot.tilt_TB = constrain(map(long(lastTiltTB), 900, 20, -450, 450), -450, 450);
	com.pilot.tilt_LR = constrain(map(long(lastTIltLR), 982, 67, -450, 450), -450, 450);
	
	#ifdef _INO_DEBUG
		Serial.print("DRAZKI: THR: ");
		Serial.print(com.pilot.throttle);
		Serial.print("\tROT: ");
		Serial.print(com.pilot.rotate);
		Serial.print("\tTB: ");
		Serial.print(com.pilot.tilt_TB);
		Serial.print("\tLR: ");
		Serial.println(com.pilot.tilt_LR);
#endif
	
	
	//com.wyslij(PILOT_RAMKA_TEST_TYPE);   // DO PRZEBUDOWY
	
	#ifdef USE_PC_APP
		// Calc steering data for pc app
		cpa.sterVar.throttle = com.pilot.throttle/4; //    0:250 dla apki na pc
		cpa.sterVar.rotate = com.pilot.rotate/3;     // -150:150
		cpa.sterVar.tiltTB = com.pilot.tilt_TB/3;    // -150:150
		cpa.sterVar.tiltLR = com.pilot.tilt_LR/3;    // -150:150
		
		cpa.wyslij(); // wyslij do apki pc
	#endif
	
	
	
	// <<<<< ====== ---  RZECZY POBOCZNE  --- ====== >>>>>

	// Naraznie testowe
	if (com.connectionState()) green.setPattern(DIODE_ON);
	else green.setPattern(DIODE_OFF);
	
	red.runDiode();
	green.runDiode();
	
	//delay(48);  // ========asdfasdfajsdkj     DO PRZEMYSLENIA  !!! 
}