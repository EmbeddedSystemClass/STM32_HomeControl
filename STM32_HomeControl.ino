
#include "sys_cfg.h"  // configure debug if necessary

//#include <TimeLib.h>
#include <SPI.h>
#include <Ethernet_STM32.h>
#include <SdFat.h>
#include "ether_server.h"
#include "time_client.h"
#include "file_client.h"
#include "vito_client.h"
#include "energy_cam.h"

// Pin mapping:
// ---------------------------
// Ethernet SPI CS: PA3
// Ethernet reset:  PA2
// SD-Card SPI CS:  PA4
// RS485 direction: PA0


//byte wdg_counter = 0;
static byte minute_old;
/***********************************************************************************/
void setup()
{
	// Open serial communications and wait for port to open:
#if _DEBUG_>0
	Serial.begin(115200);
	delay(5000);
	pinMode(DBG_PIN, OUTPUT);
	digitalWrite(DBG_PIN, HIGH);
	Serial.println(F("\n\n***** Home Automation application started *****\n"));
#endif

	EtherServer_Init();
	TimeClient_Init();
	FileClient_Init(CS_SD_CARD);
	VitoClient_Init();
	EC_Init();
	minute_old = 60;
#if _DEBUG_>0
	Serial.println(F("setup end."));
#endif
}
/***********************************************************************************/
int freeRam (void)
{
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
/***********************************************************************************/
void loop()
{
	// intervall check
	byte minute_now = minute();
	// check incoming serial character
#if _DEBUG_>0
	if ( Serial.available() )
	{
		// get the new byte:
		char inChar = Serial.read();
		//if (inChar == 'f')    File_PrintFile();  // display file content
		if (inChar == 't')	TimeClient_UpdateFileString();  // update date and time strings
		//if (inChar == 'n')    Vito_ClientNewDay();
		//if (inChar == 'v')    Vito_ClientSetVitoTime();
	}
#endif
	// execute next functions only once in each minute
	if ( minute_now!=minute_old )
	{
		//Serial.printF("minute: ")); Serial.println(minute_now);
		TimeClient_UpdateFileString();  // update date and time
		// new day events
		if ( Time_NewDay() ) {
			File_NewDay();
			VitoClient_NewDay();
			EC_NewDay();
		}
		// read Vito parameters each even minute
		if ( (minute_now%2)==0 ) {
			VitoClient_ReadParameters();
			EC_ReadValue();
			File_WriteDataToFile();
			// control hot water
			VitoClient_CheckDHW();
		}
		// do time update each 5 minutes
		if ( (minute_now%5)==0 )  TimeClient_Ping();
	}
	// do here server client tasks, listen to requests and send reply
	EtherServer_CheckForClient();
	//Ethernet.maintain();

	minute_old = minute_now;
	delay(20);
}


