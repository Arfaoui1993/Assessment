#include "sensor/wireless.h"
#include "sensor/ki_store.h"
#include "sensor/door.h"
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
 * in over the 868 MHz communication channel.
 */
void handle_communication(void)
{
	uint8_t uint8receivedbackenddata[128];//send/receive data from backend buffer: first byte should contain a command data in here used to be sent to backend
	uint8_t uint8sensorpocketdata[32];//send/receivedata sensor buffer: first byte should contain a command data in here used to be sent to sensors
	uint8_t uint8AckBackend = ACK_BACKEND;
	uint8_t uin8index=0;
	
	if(wireless_dequeue_incoming(uint8sensorpocketdata)){
		switch( uint8sensorpocketdata[0] ){
			case(PING_SENSOR):{
			//sensor should send acknowledgment	
			wireless_enqueue_outgoing(&uint8AckBackend);//Send Ack to gateway
			
			//sensor should receive 4 data sockets now and store them in uint8receivedbackenddata
				uin8index=0;
				while(uin8index<128){
				if(wireless_dequeue_incoming(uint8sensorpocketdata)){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				uint8receivedbackenddata[i]=uint8sensorpocketdata[i-uin8index];			
						}
				uin8index = uin8index + 32;
					}
					//FIXME: program can bloc in here if there is no data received
				}
				
			//After receiving the 4 data pockets, the sensor will send them again pocket by pocket
				uin8index=0;
				while(uin8index<128){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				uint8sensorpocketdata[i-uin8index]=uint8receivedbackenddata[i];			
				}
				wireless_enqueue_outgoing(uint8sensorpocketdata);
				uin8index = uin8index + 32;
				}
	}
	break;
	case(RESTART_SENSOR):{
	//Resatrt sensor
	}
	break;
	case(ADD_NEWKI_TO_SENSOR):{
	//sensor should send acknowledgment	
			wireless_enqueue_outgoing(&uint8AckBackend);//Send Ack to gateway
	
				//sensor should receive 4 data sockets now and store them in uint8receivedbackenddata
				uin8index=0;
				while(uin8index<128){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				if(wireless_dequeue_incoming(uint8sensorpocketdata)){
				uint8receivedbackenddata[i]=uint8sensorpocketdata[i-uin8index];			
						}
				uin8index = uin8index + 32;
					}
					//FIXME: program can bloc in here if there is no data received
				}
				uint8_t uint8ki[16];
				for(uint8_t i=0;i<16;i++){
					uint8ki[i]=uint8receivedbackenddata[i+8] ;
				}
				if(KI_STORE_SUCCESS==ki_store_add(uint8ki)){
					wireless_enqueue_outgoing(&uint8AckBackend);//Send Ack to gateway
				}
	break;
		}
			case(DELETE_KI):{
			//sensor should send acknowledgment	
			wireless_enqueue_outgoing(&uint8AckBackend);//Send Ack to gateway
	
				//sensor should receive 4 data sockets now and store them in uint8receivedbackenddata
				uin8index=0;
				while(uin8index<128){
				for(uint8_t i=uin8index;i<32+uin8index;i++){
				if(wireless_dequeue_incoming(uint8sensorpocketdata)){
				uint8receivedbackenddata[i]=uint8sensorpocketdata[i-uin8index];			
						}
				uin8index = uin8index + 32;
					}
					//FIXME: program can bloc in here if there is no data received
				}
				uint8_t uint8ki[16];
				for(uint8_t i=0;i<16;i++){
					uint8ki[i]=uint8receivedbackenddata[i+8] ;
				}
				if(KI_STORE_SUCCESS==ki_store_remove(uint8ki)){
					wireless_enqueue_outgoing(&uint8AckBackend);//Send Ack to gateway
				}
	break;
			}
		}
	}
}
