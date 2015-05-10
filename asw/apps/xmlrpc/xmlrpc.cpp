#include "err.h"
#include "api.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include "tcp.h"
#define TCP_PORT 11311


/*
 * Notes:
 * registerPublisher(caller_id, topic, topic_type, caller_api) for registering publisher
 * -> all subscribers to this topic are then notified and each of them calls requestTopic(caller_id, topic, protocols)
 * As a response, each receives the protocol type (e.g. TCPROS), the ip address, and the port.
 * Then they connect to the 'TCP server' of the publisher.
 *
 * Problem: requestTopic requires STM32 to parse something like this:
 * <?xml version="1.0"?> <methodCall> <methodName>requestTopic</methodName> <params> <param>
 * <value>/listener</value> </param> <param> <value>chatter</value> <param> <value>
 * <array><data><value><array><data><value><string>TCPROS/</string></value></data>
 * </array></value></data></array></value> </param> </params> </methodCall>
 * Solution: get the first string after <methodName> until < is found. ->requestTopic (then same for the first two values)
 */



char req[] = "POST / HTTP/1.1\nUser-Agent: curl/7.35.0\nHost: SI-Z0M81:11311\Accept: */*\nContent-Length: 316\nContent-Type: application/x-www-form-urlencoded\n\n<?xml version=\"1.0\"?> <methodCall> <methodName>registerPublisher</methodName> <params> <param> <value>/rostopic_4767_1316912741557</value> </param> <param> <value>chatter</value> <param> <value>std_msgs/String</value> </param> <param> <value>http://10.3.84.99:47855/</value> </param> </param> </params> </methodCall>";

#include <lwip/sockets.h>

#define SENDER_PORT_NUM 47855
#define SENDER_IP_ADDR "10.3.84.99"

#define SERVER_PORT_NUM 11311
#define SERVER_IP_ADDRESS "10.3.84.100"

#include "xmlrpc.h"

extern "C" void os_printf(char* fmt, ...);


#include "XMLRPCHandler.h"
#include "ConnectionHandler.h"
#include "msg.h"
#include "std_msgs/String.h"
#include "std_msgs/Int32.h"
#include "std_msgs/ColorRGBA.h"
using namespace std_msgs;

void serializeMsg(const ros::Msg& msg, unsigned char* outbuffer)
{
	unsigned char stream1[100];
	uint32_t offset = msg.serialize(stream1);
	memcpy(outbuffer, &offset, sizeof(uint32_t));
	memcpy(outbuffer+sizeof(uint32_t), stream1, offset);
}

void xmlrpc_task(void* p)
{
	XMLRPCHandler::registerPublisher("rostopic_4767_1316912741557", "chatter", "std_msgs/String", "http://10.3.84.99:47855/");
	//XMLRPCHandler::registerSubscriber("rostopic_4767", "chatter", "std_msgs/String", "http://10.3.84.99:47857/");
	//XMLRPCHandler::registerPublisher("rostopic_4767_13169", "chatter", "std_msgs/String", "http://10.3.84.99:47856/");


	vTaskDelay(2000);
	ConnectionHandler* connection = new ConnectionHandler;
	connection->initPublisherEndpoint(50000);

	XMLRPCHandler::waitForRequest(47855);

	/*unsigned char stream[] =
	{0x0e, 0x00, 0x00, 0x00,
	   0x0a, 0x00, 0x00, 0x00,
	   	   'H', 'e', 'l', 'l', 'o', ' ', 'R', 'O', 'S', '!'
	};*/

	char string[] = "Hello ROS!";
	/*String str;
	str.data = string;*/

	/*Int32 str;
	str.data = 32;*/

	ColorRGBA str;
	str.r = 200;
	str.g = 100;
	str.b = 22;
	str.a = 234;
	unsigned char stream[100];
	serializeMsg(str, stream);

	/*unsigned char stream1[100];
	uint32_t offset = str.serialize(stream1);
	unsigned char stream[offset];
	memcpy(stream, &offset, sizeof(uint32_t));
	memcpy(stream+sizeof(uint32_t), stream1, offset);*/


	//TODO: Why does system crash with rostopic echo chatter?
	//TODO: Connection won't be initialized if PC side subscriber is created before STM32 side publisher.
	for (;;)
	{
		connection->sendMessage((const char*)stream);
		vTaskDelay(500);
	}

	vTaskDelete(NULL);
}

