#include "gateway/modem.h"
#include "gateway/wireless.h"
#include "common/device.h"

# define PING_GATEWAY					0x01
# define PING_SENSOR					0x02
# define PINGBYTE					0x03
# define RESTART_SENSOR					0x04
# define RESTART_GATEWAY				0x05
# define ADD_NEWKI_TO_SENSOR				0x08
# define DELETE_KI					0x0A
# define OPEN_DOOR					0x0F
# define ACK_BACKEND					0x11
# define BREAK_COUNTER					100

/**
 * This function is polled by the main loop and should handle any packets coming
 * in over the modem or 868 MHz communication channel.
 */
void handle_communication(void)
{
  // YOUR CODE GOES HERE
	const uint8_t* puint8backenddata[128];//first byte should contain a command
	uint8_t uint8sensordata[128];//first byte should contain a command
	uint8_t uint8AckBackend  = ACK_BACKEND;
	uint8_t uint8PingGateway  = PING_GATEWAY;
	uint8_t uint8PingSensor  = PING_SENSOR;

	size_t length=128;
	
	// Incoming backend data in the gateway state machine 
	if(modem_dequeue_incoming(puint8backenddata, &length)){//check any incoming data sockets from the Backend
		//TODO: fing a  way to deal with wrong incomming data
			switch( *puint8backenddata[0] ){
			case(PING_GATEWAY):{
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway ack the backend
				modem_enqueue_outgoing(&uint8PingGateway,1);//gateway send pong to backend
				}
			break;
			case(PING_SENSOR):{
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway ack the backend
				wireless_enqueue_outgoing(get_device_id(),&uint8PingSensor);//gateway ping sensor
				device_id_t *device_id=NULL;
				wireless_dequeue_incoming(device_id,uint8sensordata);
				if( PING_SENSOR==uint8sensordata[0] ){//gateway receive pong sensor 
				modem_enqueue_outgoing(&uint8PingSensor,1);//gateway sends pong senor to backend
				}
				}
			break;
			case(RESTART_GATEWAY):{
			}
			break;
			case(RESTART_SENSOR):{
				}
			break;
			case(ADD_NEWKI_TO_SENSOR):{
				}
			break;
			case(DELETE_KI):{
				}
			break;
			case(OPEN_DOOR):{
				}
			break;
			}
	}
		device_id_t *device2_id=NULL;
		// Incoming sensor data in the gateway state machine 
		if(wireless_dequeue_incoming(device2_id, uint8sensordata)){ //check any incoming data sockets from the Sensor
				switch(uint8sensordata[0]){
					
		}
	}
}

