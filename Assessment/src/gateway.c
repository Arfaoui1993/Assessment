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
# define GATEWAY_TIMEOUT_COUNTER			100

/**
 * This function is polled by the main loop and should handle any packets coming
 * in over the modem or 868 MHz communication channel.
 */
void handle_communication(void)
{
	const uint8_t* puint8incomingbackenddata[128];//first byte should contain a command
	uint8_t uint8sensordata[128];//first byte should contain a command
	uint8_t uint8backenddatatosend[128];//senddata to backend buffer: first byte should contain a command data in here used to be sent to backend
	uint8_t uint8sensordatatosend[128];//senddata to sensor buffer: first byte should contain a command data in here used to be sent to sensors
	uint8_t uint8AckBackend = ACK_BACKEND;

	size_t length=128;
	// Handler should check incoming data from backend and from the sensor, data received from the backend can either go to the gateway or 		// go to the sensor. Data received from the sensor must go back to the backend through the gateway(it can be a pong(after ping), 	 // an acknowledgment,...)

	// Incoming backend data in the gateway state machine 
	if(modem_dequeue_incoming(puint8incomingbackenddata, &length)){//check any incoming data sockets from the Backend
		//TODO: fing a  way to deal with wrong incomming data
			switch( *puint8incomingbackenddata[0] ){
			case(PING_GATEWAY):{
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend
				for(uint8_t i=0;i<128;i++){
					uint8backenddatatosend[i]=*puint8incomingbackenddata[i];			
					}//TODO: find better solution to this
				modem_enqueue_outgoing(uint8backenddatatosend,128);//gateway send pong to backend
				}
			break;
			case(PING_SENSOR):{
				//modem_enqueue_outgoing(&uint8AckBackend,128);//gateway acknowledge the backend//not needed because backend 					//needs sensor ACK
				for(uint8_t i=0;i<128;i++){
					uint8sensordatatosend[i]=*puint8incomingbackenddata[i];			
					}//TODO: find better solution to this
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//gateway ping sensor
				}
			break;
			case(RESTART_GATEWAY):{
				//RESTART GATEWAY
			}
			break;
			case(RESTART_SENSOR):{
				uint8sensordatatosend[0] = RESTART_SENSOR;			
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send restart command to sensor
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
		device_id_t *device_id=NULL;
		// Incoming sensor data in the gateway state machine 
		// if the gateway don't receive ack from the sensor, it will keep sending data
		// TODO: Add timeout to receiving data here, if the time out is gone gateway should send the data again
		if(wireless_dequeue_incoming(device_id, uint8sensordata)){ //check any incoming data sockets from the Sensor
			if(NULL!=device_id){//check if sensor was detected
			switch(uint8sensordata[0]){
				case(PING_SENSOR):{//gateway receive pong sensor 
				modem_enqueue_outgoing(uint8sensordata,128);//gateway sends the sensor pong to backend
				}
				break;
				case(ACK_BACKEND):{//gateway should should send the ACK of the sensor 
				modem_enqueue_outgoing(uint8sensordata,1);//gateway sends sensor ACK to backend
				}
				break;
		}
	}
}

}
