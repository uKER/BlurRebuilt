#ifndef _ACK_SYSTEM
#define _ACK_SYSTEM

// client ackowledgement
class C_ClientAcknowledgement
{
public:
	C_ClientAcknowledgement();
	bool Active()										{ return m_bActive; }
	void Active(const bool bActive)						{ m_bActive = bActive; }
	int	 OutboundAck()	const							{ return m_uOutboundAck; }
	void OutboundAck(const int uState)					{ m_uOutboundAck = uState; }
	int	 InboundAck()									{ return m_uInboundAck; }
	void InboundAck(int uInboundAck, bool bAcknowledge)	{ m_uInboundAck = uInboundAck; m_bAckRequired = bAcknowledge; }
	bool AckRequired()									{ return m_bAckRequired; }
	void AckSent()										{ m_bAckRequired = false; }
	int	State()	const									{ return m_uState; }
	void State(int uState)								{ m_uState = uState; }
private:
	bool	m_bActive;
	int		m_uState;
	bool	m_bAckRequired; // do we need to acknowledge receipt of last state?
	int		m_uOutboundAck;	// this is the last of my states that the client has acknowledge
	int		m_uInboundAck;	// this is the last state that I received from this client
};


// acknowledgement system
class C_AckSystem
{
public:
					C_AckSystem(int uNumClients);
					~C_AckSystem();
	void			OutboundAck(const int uAcknowledgedHostId, const int uMyHostId, const int uAcknowledgedState);
	int				OutboundAck(const int uHostId) const;
	void			InboundAck(const int uAcknowledgedHostId, const int uMyHostId, const int uAcknowledgedState, bool bAcknowledge);
	int				InboundAck(const int uAcknowledgedHostId);
	bool			AckRequired(int uClient);
	void			AckSent(int uClient);
	virtual void	SetClientActive(int uClient, bool bActive = true);
	bool			IsClientActive(int uClient);
	void			SetClientState(int uClient, int uState);
	int				GetClientState(int uClient) const;
	bool			AllClientsState(int uClient);
	int				ActiveClients() const;
	int				OldestOutboundState() const;
	int				NumClients() const { return m_uNumClients; }
	bool			DebugOutput(int i, char* str, int uBufferSize, const char* strHostType);
	char*			StateString(int uState);
private:
	const int					m_uNumClients;
	C_ClientAcknowledgement*	m_pxClient;
};

#endif