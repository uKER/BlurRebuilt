#ifndef __SYSTEM_NETWORK_ADDRESS_H__
#define __SYSTEM_NETWORK_ADDRESS_H__

#pragma once

/// Classes for writing portable network code
namespace Network
{

/// IP address functionality
class Address
{
public:
					/// default constructor - initialize to 0.0.0.0
					Address();

					/// Initialize an address using a string (either x.x.x.x or hostname format)
					Address(const char *address);

					/// Copy constructor
					Address(const Address &rhs);

					// destructor
	virtual			~Address();

					/// return the hostname for the address held by this class
	const char		*GetHostname(void) const;

					/// return the address as a platform-specific in_addr struct
	struct in_addr	GetAddress() const;
protected:
					/// Sets the IP address from a string representation of the numeric address, ie "127.0.0.1"
	bool			SetIPAddress(const char *host);

					/// Specify a host name or numeric internet address
	void			SetAddress(const char *host);


	typedef Ut::Vector<struct in_addr> AddressVector;

	AddressVector	m_IPAddr;
	size_t			m_iAddrCount;
	Util::C_String	m_sHostname;
};	

/// contains the address of the local host
class LocalHost : public Address
{
public:
					LocalHost();
};

/// retrieve the hostname
Util::C_String GetHostName();

}


#endif
