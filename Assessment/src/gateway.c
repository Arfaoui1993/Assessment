#include "gateway/modem.h"
#include "gateway/wireless.h"
#include "common/device.h"

# define PING_GATEWAY					0x01
# define PING_SENSOR					0x02
# define PINGBYTE						0x03
# define RESTART_SENSOR					0x04
# define RESTART_GATEWAY				0x05
# define ADD_NEWKI_TO_SENSOR			0x08
# define DELETE_KI						0x0A
# define OPEN_DOOR						0x0E
# define CLOSE_DOOR						0x0F
# define ACK_BACKEND					0x11
# define GATEWAY_TIMEOUT_COUNTER		100

/**
 * This function is polled by the main loop and should handle any packets coming
 * in over the modem or 868 MHz communication channel.
 */
void handle_communication(void)
{
	const uint8_t* puint8incomingbackenddata[128];//first byte should contain a command
	uint8_t uint8sensordata[32];//first byte should contain a command

	uint8_t uint8backenddatatosend[128];//senddata to backend buffer: first byte should contain a command data in here used to be sent to backend
	uint8_t uint8sensordatatosend[32];//senddata to sensor buffer: first byte should contain a command data in here used to be sent to sensors
	uint8_t uint8AckBackend = ACK_BACKEND;
	
	device_id_t *device_id=NULL;

	size_t length=128;

	uint8_t timeout=GATEWAY_TIMEOUT_COUNTER;

	uint8_t uin8index=0;

	// Handler should check incoming data from backend and from the sensor, data received from the backend can either go to the gateway or 		// go to the sensor. Data received from the sensor must go back to the backend through the gateway(it can be a pong(after ping), 	 // an acknowledgment,...)

	// Incoming backend data in the gateway state machine 
	if(modem_dequeue_incoming(puint8incomingbackenddata, &length)){//check any incoming data sockets from the Backend
		//TODO: find a  way to deal with wrong incomming data
			switch( *puint8incomingbackenddata[0] ){
			case(PING_GATEWAY):{
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend
				for(uint8_t i=0;i<128;i++){
					uint8backenddatatosend[i]=*puint8incomingbackenddata[i];			
					}
				modem_enqueue_outgoing(uint8backenddatatosend,128);//gateway send pong to backend

				for(uint8_t i=0;i<128;i++){
					uint8backenddatatosend[i]=0;		 //clean send buffer	
					}
				}
			break;
			case(PING_SENSOR):{
				uint8sensordatatosend[0] = PING_SENSOR;		
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send Ping command to sensor

				//the sensor should be prepared to receive 4 consecutive ping data pockets.
				timeout=GATEWAY_TIMEOUT_COUNTER;
				while((false==wireless_dequeue_incoming(device_id, uint8sensordata))&&(timeout!=0)){
				//gateway should receive an Ack from the sensor that it will receive the ping 					//data				
						timeout--;
					}
				if((timeout==0)||(ACK_BACKEND!=uint8sensordata[0])){
				break;//if timeout is passed and ack from sensor is not received
				}else{
				modem_enqueue_outgoing(uint8sensordata,1);//gateway sends sensor ACK to backend				
				}

				//send 4 pockets of the ping in here, after being sure that the ping command has been received by the sensor
				uint8_t uin8index=0;
				while(uin8index<128){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				uint8sensordatatosend[i-uin8index]=*puint8incomingbackenddata[i];			
				}
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);
				uin8index = uin8index + 32;
				}
				
				//wait for ack that data being sent from gateway are received in the sensor
				timeout=GATEWAY_TIMEOUT_COUNTER;
				while((false==wireless_dequeue_incoming(device_id, uint8sensordata))&&(timeout!=0)){
				//gateway should receive an Ack from the sensor that it will receive the ping
										timeout--;
					}
				if((timeout==0)||(ACK_BACKEND!=uint8sensordata[0])){
				break;//if timeout is passed and ack from sensor is not received
				}else{
				modem_enqueue_outgoing(uint8sensordata,1);//gateway sends sensor ACK to backend				
				}
				
				//receive 4 pong 32 bytes pockets in here and put data in send backend buffer
				uin8index=0;
				while(uin8index<128){
				if(wireless_dequeue_incoming(device_id, uint8sensordata)){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				uint8backenddatatosend[i-uin8index]=uint8sensordata[i];			
				}
				uin8index = uin8index + 32;
				}
				}
				modem_enqueue_outgoing(uint8backenddatatosend,128);
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
				//modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend that the command is received
				uint8sensordatatosend[0] = ADD_NEWKI_TO_SENSOR;		
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send add newki command to sensor
				
				//the sensor should be prepared to receive 4 consecutive data pockets coontaining newki.
				timeout=GATEWAY_TIMEOUT_COUNTER;
				while((false==wireless_dequeue_incoming(device_id, uint8sensordata))&&(timeout!=0)){
				//gateway should receive an Ack from the sensor that it will receive the ping	
						timeout--;
					}
				if((0==timeout)||(ACK_BACKEND!=uint8sensordata[0])){
				break;//if timeout is passed and ack from sensor is not received
				}else{
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway sends sensor ACK to backend				
				}
				
				//backend should start sending another socket that contain newKI data
				timeout=GATEWAY_TIMEOUT_COUNTER;
				while((false==modem_dequeue_incoming(puint8incomingbackenddata, &length))&&(timeout!=0)){
				//gateway should receive an Ack from the sensor that it will receive the ping	
						timeout--;
					}
				if(0==timeout){
				break;//if timeout is passed and newKi data from backend is not received
				}else{ //second socket of data received
				modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend of receiving the new kiwi data			
				}
		
				//send 4 pockets in here
				uin8index=0;
				while(uin8index<128){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				uint8sensordatatosend[i-uin8index]=*puint8incomingbackenddata[i];			
				}
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);
				uin8index = uin8index + 32;
				}
				
				//wait for ack that data being sent from gateway are received in the sensor
				timeout=GATEWAY_TIMEOUT_COUNTER;
				while((false==wireless_dequeue_incoming(device_id, uint8sensordata))&&(timeout!=0)){
				//gateway should receive an Ack from the sensor that it will receive the ping
										timeout--;
					}
				if((timeout==0)||(ACK_BACKEND!=uint8sensordata[0])){
				break;//if timeout is passed and ack from sensor is not received
				}else{
				modem_enqueue_outgoing(uint8sensordata,1);//gateway sends sensor ACK to backend				
				}
				}
			break;
			case(DELETE_KI):{
				//modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend
				uint8sensordatatosend[0] = DELETE_KI;
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send delete ki command to sensor
				}
			break;
			case(OPEN_DOOR):{
				//modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend
				uint8sensordatatosend[0] = OPEN_DOOR;
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send open door command to sensor
				}
			break;
			case(CLOSE_DOOR):{
				//modem_enqueue_outgoing(&uint8AckBackend,1);//gateway acknowledge the backend
				uint8sensordatatosend[0] = CLOSE_DOOR;
				wireless_enqueue_outgoing(get_device_id(),uint8sensordatatosend);//Send close door command to sensor
				}
			break;
			}
	}
}
