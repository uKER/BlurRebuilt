// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Store all the common information for the UPnP module.

#ifndef BD_UPNP_COMMON_H
#define BD_UPNP_COMMON_H

#include <bdCore/bdCore.h>

// special reserved address for UPnP discovery
static const bdByte8 s_discoveryAddress []  = "239.255.255.250:1900";

#define BD_MAX_UPNP_DISCOVERY_REPLY (1024)
#define BD_DEFAULT_UPNP_CONNECTION_PORT (80u)

#define BD_UPNP_MAX_DEVICE_REPLY_SIZE (1024 * 6)
#define BD_UPNP_MAX_DEVICE_REQUEST_SIZE (2048)
#define BD_UPNP_FETCH_LOCATION_SIZE (64) 
#define BD_UPNP_CONTROL_URL_SIZE (128)

// two different possible UPnP routing methods
#define IP_SERVICE "WANIPConnection:1"
#define PPP_SERVICE "WANPPPConnection:1"
#define MAX_SERVICE_LEN 18
#define HTTP "http://"
#define CONTROL_URL "<controlURL"
#define URL_BASE "<URLBase"
#define CLOSE_TAG ">"
#define OPEN_END_TAG "</"
#define HTTP_OK "200 OK"
#define INTERNAL_CLIENT_STRING "<NewInternalClient"
#define EXTERNAL_IP_ADDRESS_STRING "<NewExternalIPAddress"
#define SOAP_ENVELOPE_END_TAG "</s:Envelope>"

// UPnP discovery request string
static const bdByte8 s_discoveryString[] =
"M-SEARCH * HTTP/1.1\r\n"
"MX: 1\r\n" // ask the device to reply as quickly as possible
"HOST: 239.255.255.250:1900\r\n"
"MAN: \"ssdp:discover\"\r\n"
"ST: urn:schemas-upnp-org:service:%s\r\n"
"\r\n";

static const bdByte8 s_getRequest[] = 
"GET %s HTTP/1.1\r\n"
"Connection: close\r\n"
"Host: %s\r\n\r\n";

static const bdByte8 s_httpPost[] =
"POST %s HTTP/1.1\r\n"
"HOST: %s\r\n"
"SOAPACTION: \"urn:schemas-upnp-org:service:%s#%s\"\r\n"
"CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
"CONTENT-LENGTH: %d\r\n\r\n"
"%s";

static const bdByte8 s_soapMessage[] =
"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
"<s:Body>\r\n"
"<u:%s xmlns:u=\"urn:schemas-upnp-org:service:%s\">\r\n"
"%s"
"</u:%s>\r\n"
"</s:Body>\r\n"
"</s:Envelope>\r\n";

static const bdByte8 s_removePortMappingCommand[] = "DeletePortMapping";
static const bdByte8 s_removePortMappingArguments[] = 
"<NewRemoteHost></NewRemoteHost>\r\n"
"<NewExternalPort>%d</NewExternalPort>\r\n"
"<NewProtocol>UDP</NewProtocol>\r\n";

static const bdByte8 s_addPortMappingCommand[] = "AddPortMapping";
static const bdByte8 s_addPortMappingArguments[] =
"      <NewRemoteHost></NewRemoteHost>\r\n"
"      <NewExternalPort>%d</NewExternalPort>\r\n"
"      <NewProtocol>UDP</NewProtocol>\r\n"
"      <NewInternalPort>%d</NewInternalPort>\r\n"
"      <NewInternalClient>%s</NewInternalClient>\r\n"
"      <NewEnabled>1</NewEnabled>\r\n"
"      <NewPortMappingDescription>"
"DemonwarePortMapping"
"</NewPortMappingDescription>\r\n"
"      <NewLeaseDuration>0</NewLeaseDuration>\r\n";

static const bdByte8 s_getPortMappingCommand[] = "GetSpecificPortMappingEntry";
static const bdByte8 s_getPortMappingArguments[] =
"<NewRemoteHost></NewRemoteHost>\r\n"
"<NewExternalPort>%d</NewExternalPort>\r\n"
"<NewProtocol>UDP</NewProtocol>\r\n";

static const bdByte8 s_getExternalAddressCommand[] = "GetExternalIPAddress";
static const bdByte8 s_getExternalAddressArguments[] = "\r\n";

static const bdUInt s_maxCommandLen =	bdUInt( sizeof(s_getPortMappingCommand));
static const bdUInt s_maxArgLen		=	bdUInt( sizeof(s_addPortMappingArguments)										
												+ BD_ADDR_STRING_SIZE 
												+ 10 /* For the two port strings*/);
static const bdUInt s_soapSize		=	bdUInt( sizeof( s_soapMessage ));
static const bdUInt s_maxSoapSize	=	s_soapSize + s_maxCommandLen + s_maxArgLen + MAX_SERVICE_LEN;

#endif // BD_UPNP_COMMON_H

