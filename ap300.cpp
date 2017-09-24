/******************************************************************************
	Dreambox
****************************************
******************************************************************************/
#include "sys_cfg.h"
#include "dreambox.h"
#include <Ethernet_STM32.h>
#include "ether_server.h"
#include "ether_client.h"
#include "file_client.h"
#include "vito.h"
//#include "serial1.h"

//#define DM800SE	0
//#define DM800		1
typedef enum { DM800SE, DM800 } ap_device_t;
static ap_device_t ap_device;
typedef enum { DM_OK, DM_STANDBY, CONNECTION_TIMEOUT } ap_state_t;
static ap_state_t ap_state[2];
typedef enum { CMD_TIMER_LIST, CMD_TIMER_CLEANUP, CMD_GET_STATUS} ap_cmd_t;
static ap_cmd_t ap_cmd;
// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet);
static EthernetClient ap_client;
static byte ap_ip[2];
static int ap_port = 80;
//
typedef enum {
	START = 1,
	DATA = 2,
	END = 4,
} list_status_t;	// add here further clients
static byte reply;
