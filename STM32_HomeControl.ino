
#include "sys_cfg.h"  // configure debug if necessary

#include <Time_lib.h>
#include <SPI.h>
#include <Ethernet_STM.h>
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

	Ethernet_Init();
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
void Ethernet_Init(void)
{
byte mymac[] = { 0x10,0x69,0x69,0x2D,0x30,0x40 };
IPAddress myip(192,168,100, 58);
IPAddress gateway(192,168,100, 1);
IPAddress subnet(255, 255, 255, 0);
//byte ETHERNET_RESET_PIN = 0;
	// start the Ethernet connection:
#if _DEBUG_>0
	Serial.print(("Getting IP address using DHCP ... "));
#endif
	// reset Ethernet interface
	ETHERNET_RESET_PIN_MODE_SET_TO_OUTPUT;  //      pinMode(ETHERNET_RESET_PIN, OUTPUT);
	ETHERNET_RESET_PIN_SET_TO_LOW;  //    digitalWrite(ETHERNET_RESET_PIN,LOW);
	delay(1);
	ETHERNET_RESET_PIN_SET_TO_HIGH;  //    digitalWrite(ETHERNET_RESET_PIN,HIGH);
	delay(100);
	
	Ethernet.init(ETHERNET_SPI_CS_PIN, SPISettings(18000000));

	//  if ( Ethernet.begin(mymac, myip, gateway, subnet) == 0 ) {
	if ( Ethernet.begin(mymac)==0 ) {
#if _DEBUG_>0
		Serial.print(F("failed! Setting static IP address ... "));
#endif
		// initialize the ethernet device not using DHCP:
		Ethernet.begin(mymac, myip, gateway, subnet);
	}
#if _DEBUG_>0
	Serial.print(F("done."));
	// print your local IP address:
	Serial.print(F(" My IP address: "));
	//Serial.println(Ethernet.localIP());
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print("."); 
	}
	Serial.println();
	showSocketStatus();
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
	WDG_RST;
	// intervall check
	//time_t t = now();
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


