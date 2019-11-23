#include "sys_cfg.h"
#include <SPI.h>
#include <Ethernet_STM32.h>
#include <SdFat.h>
#include "ether_server.h"
#include "file_client.h"
#include "time_client.h"
#include "vito.h"
#include "vito_client.h"
#include "energy_cam.h"

// data buffer to store data received by the server
char s_buf[SERVER_BUFFER_MAX_SIZE];
byte s_ind;
// Initialize the Ethernet server on port 80
EthernetServer my_server(80);
uint8_t remoteip[4];
uint32_t oldip;
byte allow;
long accessTimeout;

void EtherServer_ProcessData(EthernetClient cl);

/***********************************************************************************/
void Ethernet_Init(void)
{
byte mymac[] = { 0x10,0x69,0x69,0x2D,0x30,0x40 };
IPAddress myip(192,168,100, 41);
IPAddress gateway(192,168,100, 1);
IPAddress subnet(255, 255, 255, 0);

	// reset Ethernet interface
	pinMode(ETHERNET_RESET_PIN, OUTPUT);
	digitalWrite(ETHERNET_RESET_PIN, HIGH);
	delay(10);
	digitalWrite(ETHERNET_RESET_PIN, LOW);
	delay(10);
	digitalWrite(ETHERNET_RESET_PIN, HIGH);
	delay(10);

	Ethernet.init(ETHERNET_SPI_CS_PIN);

	// start the Ethernet connection:
#if _DEBUG_>0
	Serial.print("Getting IP address using DHCP ... ");
#endif
	if ( Ethernet.begin(mymac)==0 ) {
#if _DEBUG_>0
		Serial.print("failed! Setting static IP address ... ");
#endif
		// initialize the ethernet device not using DHCP:
		Ethernet.begin(mymac, myip, gateway, subnet);
	}
#if _DEBUG_>0
	Serial.print(F("done."));
	// print your local IP address:
	Serial.print(F(" My IP address: "));
	Serial.println(Ethernet.localIP());
	//for (byte thisByte = 0; thisByte < 4; thisByte++) {
	//	// print the value of each byte of the IP address:
	//	Serial.print(Ethernet.localIP()[thisByte], DEC);
	//	Serial.print("."); 
	//}
	Serial.println();
	showSocketStatus();
#endif
}

/*****************************************************************************/
/*****************************************************************************/
void EtherServer_Init(void)
{
#if _DEBUG_>0
	Serial.println("Initializing the Ethernet interface...");
#endif
    Ethernet_Init();

#if _DEBUG_>0
	Serial.print(F("Initializing Ethernet server..."));
#endif
	my_server.begin();
	s_ind = 0;
	allow = 0;
	*(uint32_t*)remoteip = 0;
	oldip = 1;
	accessTimeout = millis();

#if _DEBUG_>0
  Serial.println(F("done."));
#endif
}
/*****************************************************************************/
/*****************************************************************************/
void EtherServer_ReceiveData(EthernetClient cli)
{
	byte eol = 0;
	s_ind = 0;
   // read and store the first header line only
  if ( cli.connected() ) {
	while ( cli.available() ) {
		char c = cli.read();
#if _DEBUG_>1
//    if ( s_line>0 )  Serial.write(c);
#endif
		// don't store CR, store only first header line, and avoid buffer overflow      
		if ( eol==1 || c=='\r' || s_ind>=(SERVER_BUFFER_MAX_SIZE-1) )	continue;

		if ( c=='\n' ) {
			c = 0;  // mark end of the received string
			eol = 1;
		}
		s_buf[s_ind++] = c;	// store here the data
	}
  }

#if _DEBUG_>0
	Serial.print(("ether server received: "));
	Serial.println(s_buf);
#endif
}
/*****************************************************************************/
/*****************************************************************************/
void EtherServer_CheckForClient(void)
{
	// listen for incoming clients
	EthernetClient s_client = my_server.available();
	if ( !s_client)  return;

	TimeClient_UpdateFileString();  // update date and time

	s_client.getRemoteIP(remoteip);
#if _DEBUG_>0
	Serial.print(F("---> request from: "));
	Serial.println((IPAddress)remoteip);
    showSocketStatus();
#endif
	// security check remote IP. search for the same ip in the local record (clients.txt)
	if ( oldip == *(uint32_t*)remoteip ) {
		// client is same as previous one
		if ( allow )  accessTimeout = millis() + 5*60*1000;  // renew time-out if it is registered IP
	} else {
		// new client. check if it is already recorded
		byte found=0;
		for (byte i=0; i<100; i++) {
			strcpy_P(s_buf, ("clients.txt"));
			File_GetFileLine(i);  // the input file name is in f_buf, the read line is returned also in f_buf
			if ( s_buf[0]==0 ) break;
			// the line is in s_buf. parse the IP address
			char * chrPtr = strtok(s_buf, " ");
			chrPtr = strtok(NULL, " .");
			if ( remoteip[0]==atoi(strtok(NULL, " .")) && remoteip[1]==atoi(strtok(NULL, ".")) && remoteip[2]==atoi(strtok(NULL, ".")) && remoteip[3]==atoi(strtok(NULL, ".")) ) {
#if _DEBUG_>0
				Serial.print(("client has index ")); Serial.println(i);
#endif
				found = 1;
				break;
			}
		}
		if ( found==0 ) {
#if _DEBUG_>0
			Serial.print(F("recording new client: "));
			sprintf(s_buf, ("%u.%u.%u.%u"), remoteip[0], remoteip[1], remoteip[2], remoteip[3]);
			Serial.println(s_buf);
#endif
			File_LogClient(s_buf);
		}
		// update old ip
		oldip = *(uint32_t*)remoteip;
	}

	if ( s_client.available() ) {
		// read client request, store it into the s_buf and process it
		EtherServer_ReceiveData(s_client);
		EtherServer_ProcessData(s_client);
	}
#if _DEBUG_>0
	showSocketStatus();
	Serial.println(("...stopping client..."));
#endif
	s_client.stop();
#if _DEBUG_>0
	showSocketStatus();
	Serial.println(F("<--- client end."));
#endif
}
/***********************************************************************************/
void putdecimal(word w, char * s, byte digs)
{
  s += digs;
//  *s = 0;  // write string ending mark
  do {
      *(--s) = ( w % 10 ) + '0';
  } while ( (w/=10)!=0 && (--digs)>0 );
}
/***********************************************************************************/
void Ether_BufAddInt(word w, byte digits)
{
  putdecimal(w, s_buf+s_ind, digits);
  s_ind += digits;
}
/***********************************************************************************/
void Ether_BufAdd_P(const char * str)
{
  strcpy_P(s_buf+s_ind, str);
  s_ind += strlen_P(str);
}
/***********************************************************************************/
/***********************************************************************************/
void Ether_PrintSimpleHeader(EthernetClient cl)
{
	cl.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\n"));
}
/***********************************************************************************/
/***********************************************************************************/
void Ether_PrintStandardHeader(EthernetClient cl)
{
	Ether_PrintSimpleHeader(cl);
	cl.println(F("<!DOCTYPE HTML>\n<html>\n<head><meta charset='UTF-8'>\n<title>My homepage</title></head>\n"));
	cl.println(F("<body>\n<h3>&emsp;<a href='/web/index.htm'>Home</a>&emsp;<a href='/files/'>SD-card</a>&emsp;<a href='/web/vitomon.htm'>Data GUI</a></h3>"));
	cl.print(F("<p>It is ")); cl.print(dayStr(weekday())); cl.print(F(", 20")); cl.print(date_str); cl.print(F(", ")); cl.print(time_str); cl.println(F("</p>"));
}
/***********************************************************************************/
/***********************************************************************************/
void EtherServer_ProcessData(EthernetClient cl)
{
	uint32_t time0 = millis();
	if ( strncmp(s_buf, ("GET /"), 5)==0 )
	{
		if ( strncmp(s_buf+5, ("?regmyip"), 8)==0 ) {  // requested a file or execute a command
			// register IP -> allow access for 5 minutes
			allow = 1;
			accessTimeout = millis() + 5*60*1000;  // allow access for 5 minutes
			goto bad_request_1;  // tricky 'bad request' response for unknown users
		} else {	// parse the requested dir/file
			//dir_file_check:
			char * str = strchr(s_buf+5, ' ');	// find next space pointing before 'HTTP1/x'
			if ( str==0 ) goto bad_request;
			*str = 0;  // tricky marking end of dir/file to 0
			if ( *(str-1)=='/' ) *(str-1) = 0;	// remove any directory delimiter '/' from the end
			// set string to lower case
			strlwr(s_buf+5);
			// list dir or send file content of SD-card
			File_CheckDirFile(cl);
		}
	} else if ( strncmp(s_buf, ("POST /"), 6)==0 ) {  // requested a file or execute a command
		// POST /readfile=15-06-07.TXT HTTP/1.1
		*(strchr(s_buf+5, ' ')) = 0;
		// feasibility check
		if ( strchr(s_buf+5, '=')!=NULL ) {
			// check POST request
			File_CheckPostRequest(cl);
		}
	} else {  //    unknown request
bad_request:
#if _DEBUG_>0
      Serial.print(F("Ether server: unknown request: ")); Serial.println(s_buf+5);
#endif
bad_request_1:
      cl.println(F("HTTP/1.0 400 Bad Request\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nBad Request:"));
      cl.println(s_buf+5);
  }

  // send here required processing time info
  if ( s_buf[0]!=0 ) { cl.print(F("\nPage sent in ")); cl.print(millis()-time0); cl.println(F(" ms.")); }
}
