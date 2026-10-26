#include "stdafx.h"
#include "TestSectionMigration.h"
#include "netPlatform/netPlatform.h"
#include "netPlatform/netPlatform_Core.h"
#include <bdCore/bdContainers/bdHashMap.h>
#include <system/debug/I_DebugRender.h>
#include <Framework/Application/UserControls.h>

namespace TestMigration
{

	static Debug::Logger& g_log = Debug::GetLogger( "migrationtest.debug" );

	// peer data, as sent across network
	BD_REFERENCE(PeerState);
	class PeerState: public bdReferencable
	{
	public:
		PeerState(int self)
		: m_self(self)
		, m_host(-1)
		, m_sqn(-1)
		, m_electionCandidate(-1)
		, m_electionWeight(0)
		, m_electionResults(false)
		, m_electionUnanimous(false)
		{
			m_peercount = 0;
			m_name[0] = 0;
			m_lastReceivedStamp = Time::GetPerformanceCounter();
		}

		bool operator > (const PeerState& other) const
		{
			if(m_electionWeight > other.m_electionWeight)
				return true;

			if(m_electionWeight < other.m_electionWeight)
				return false;

			if(m_self > other.m_self)
				return true;

			if(m_self < other.m_self)
				return false;

			return false;
		}

		int* findPeer(int id) 
		{ 
			for(int i = 0; i < m_peercount; i ++)
			{
				if(m_peers[i] == id)
					return &(m_peers[i]);
			}
			return NULL;
		}

		bool valid() const
		{
			return ((Time::GetPerformanceCounter() - m_lastReceivedStamp) < 6 * 1000 * 1000);
		}

		int			m_peers[100];
		int			m_peercount;

		char		m_name[128];
		int			m_self;
		int			m_host;
		int			m_sqn;
		
		int			m_electionCandidate;
		int			m_electionWeight;
		bool		m_electionResults;
		bool		m_electionUnanimous;
		U64			m_lastReceivedStamp;
	};

	// single peer
	BD_REFERENCE(Peer);
	class Peer: public bdReferencable
	{
	public:
		Peer(int self, int host, const char* name)
		: m_self(self)
		, m_host(host)
		, m_sqn(0)
		{
			m_sqnTimestamp = 0;
			m_sqn = netPlatform::Core::instance()->randomInt(1000);
			sprintf_s(m_name, sizeof(m_name), name);
		}

		float render(DbUtil::I_DebugRenderPtr debugRenderPtr, float y)
		{
			char buff[512];
			
			sprintf_s(buff, sizeof(buff), "name(%s) self(%d) host(%d) sqn(%d) states(%d)", m_name, m_self, m_host, m_sqn, m_peerStates.getSize());
			debugRenderPtr->Draw2dText(Maths::HVec2(40, y), buff, Maths::RGBA(0xffff6666), 24.0f);
			y += 24.0f;

			// remove all peers that are not on host anymore.
			bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef state = m_peerStates.getValue(it);
				m_peerStates.next(it);

				PeerStateRef host = getState(state->m_host);
				PeerStateRef migr = getState(state->m_electionCandidate);
				
				Maths::RGBA colour = (state->m_self == m_self)? Maths::RGBA(255, 255, 128, 255) : state->valid()? Maths::RGBA(255, 255, 255, 255) : Maths::RGBA(128, 128, 128, 255);
			
				char buffer[1024];
				int len = sizeof(buffer);
				sprintf_s(buffer, len, "self(%d, %s) sqn(%d) host(%d, %s) mig(%d, %s) weight(%d) results(%s) unanimous(%s) peers(%d).", 
									state->m_self, 
									state->m_name,
									state->m_sqn,
									state->m_host, 
									host.isNull()? "unknown" :	host->m_name,
									state->m_electionCandidate, 
									migr.isNull()? "unknown" :	migr->m_name,
									state->m_electionWeight,				
									state->m_electionResults? "yes" : "no",
									state->m_electionUnanimous? "yes" : "no",
									state->m_peercount);

				debugRenderPtr->Draw2dText(Maths::HVec2(80, y), buffer, colour, 24.0f);
				y += 24.0f;
			}
			m_peerStates.releaseIterator(it);
			return y;
		}

		void removeState(int id)
		{
			m_peerStates.remove(id);
		}
		
		PeerStateRef createState(int id)
		{
			PeerStateRef state = getState(id);
			if(state.notNull()) 
				removeState(id);

			state = new PeerState(id);
			m_peerStates.put(id, state);
			return state;
		}
		
		PeerStateRef getState(int id)
		{
			PeerStateRef s;
			m_peerStates.get(id, s);
			return s;
		}

		void invalidateState(int id)
		{
			PeerStateRef state = getState(id);

			if(state.notNull())
				state->m_lastReceivedStamp = 0;
		}

		PeerStateRef selfState()
		{
			return getState(m_self);
		}

		PeerStateRef hostState()
		{
			return getState(m_host);
		}

		void promotePeer()
		{
			// no self yet
			PeerStateRef self = selfState();
			if(self.isNull())
				return;

			if(self->m_self != self->m_host)
				return;

			int peers[100];
			int peercount=0;
			int weight = -1;

			bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef state = m_peerStates.getValue(it);
				int key = m_peerStates.getKey(it);
				m_peerStates.next(it);

				if(!state->valid()) continue;
				if(state == self) continue;
				
				if(weight < 0 || state->m_electionWeight >= weight)
				{
					if(state->m_electionWeight > weight)
						peercount = 0;

					peers[peercount++] = key;
					
					weight = state->m_electionWeight;
				}
			}
			m_peerStates.releaseIterator(it);

			if(peercount <= 0)
				return;

			int index = netPlatform::Core::instance()->randomInt(peercount);

			self->m_electionCandidate = peers[index];
		}

		void receiveState(PeerStateRef other)
		{
			// copy peer state.
			PeerStateRef proxy = getState(other->m_self);

			if(proxy.isNull() && other->m_self == m_host)
			{
				proxy = createState(other->m_self);
				m_sync = proxy;
			}

			if(proxy.notNull())
			{
				*proxy = *other;
				proxy->m_lastReceivedStamp = Time::GetPerformanceCounter();
			}
		}

		void updateConnections()
		{
			//-------------------------------------
			// sync peers
			//-------------------------------------
			if(m_sync.isNull())
				return;

			if(m_sync->m_self == m_self)
				return;

			// remove all peers that are not on host anymore.
			bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef state = m_peerStates.getValue(it);
				int key = m_peerStates.getKey(it);
				
				if(m_sync->findPeer(key) == NULL)
				{
					m_peerStates.remove(it);
				}
				else
				{
					m_peerStates.next(it);
				}
			}
			m_peerStates.releaseIterator(it);

			// add new peers that appeared on host.
			for(int i = 0; i < m_sync->m_peercount; i ++)
			{
				int key = m_sync->m_peers[i];
				
				if(getState(key).isNull())
				{
					PeerStateRef newState = new PeerState(key);
					m_peerStates.put(key, newState);
				}
			}
		}
		
		void updateSelf()
		{
			// no self yet
			PeerStateRef self = selfState();
			if(self.isNull())
				return;

			//-----------------------------------------------
			// update generic data
			//-----------------------------------------------
			self->m_self = m_self;
			self->m_host = m_host;
			self->m_sqn = m_sqn;
			self->m_lastReceivedStamp = Time::GetPerformanceCounter();
			sprintf_s(self->m_name, sizeof(self->m_name), m_name);
			
			//-----------------------------------------------
			// list all known peers
			//-----------------------------------------------
			self->m_peercount = 0;
			bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef mystate = m_peerStates.getValue(it);
				int key = m_peerStates.getKey(it);
				m_peerStates.next(it);
				
				if(mystate.notNull() && mystate->valid())
					self->m_peers[self->m_peercount++] = key;

				if(self->m_peercount >= 100) break;
			}
			m_peerStates.releaseIterator(it);

			//-----------------------------------------------
			// update migration data.
			//-----------------------------------------------

			// we have found our host
			PeerStateRef host = hostState();
			if(host.notNull() && host->valid())
			{
				// elect same candidate as host
				self->m_electionCandidate = host->m_electionCandidate;

				// setup weight, based on last received host state.
				self->m_electionWeight = host->m_sqn;

				m_sync = host;
			}
			// host is gone. Elect a candidate.
			else
			{
				// elect us first
				self->m_electionCandidate = m_self;				
				m_sync = self;
				
				bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
				while(it)
				{
					PeerStateRef peer = m_peerStates.getValue(it);
					int key = m_peerStates.getKey(it);
					m_peerStates.next(it);

					// peer disconnected. ignore.						
					if(!peer->valid()) continue;

					// peer has more authority. 
					if(*peer > *m_sync)
					{
						m_sync = peer;

						// use his election candidate.
						self->m_electionCandidate = m_sync->m_electionCandidate;
					}

				}
				m_peerStates.releaseIterator(it);
			}

			// now check if our peers agreed with our candidate.
			self->m_electionResults = true;
			
			it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef peer = m_peerStates.getValue(it);
				int key = m_peerStates.getKey(it);
				m_peerStates.next(it);

				// peer disconnected. ignore.						
				if(!peer->valid()) continue;

				// peer does not agree with us. 
				if(peer->m_electionCandidate != self->m_electionCandidate)
				{
					self->m_electionResults = false;
					break;
				}
			}
			m_peerStates.releaseIterator(it);

			// finally, check if everyone agrees with everyone.
			self->m_electionUnanimous = self->m_electionResults;

			it = m_peerStates.getIterator();
			while(it)
			{
				PeerStateRef peer = m_peerStates.getValue(it);
				int key = m_peerStates.getKey(it);
				m_peerStates.next(it);

				// peer disconnected. ignore.						
				if(!peer->valid()) continue;

				// peer does not agree with us. 
				if(peer->m_electionCandidate != self->m_electionCandidate || !peer->m_electionResults)
				{
					self->m_electionUnanimous = false;
					break;
				}
			}
			m_peerStates.releaseIterator(it);
		}

		void updateMigration()
		{
			PeerStateRef self = getState(m_self);
			if(self.isNull())
				return;
			
			// detect if we have been elected
			if(	self->m_self == self->m_electionCandidate &&
				self->m_self != self->m_host &&
				self->m_electionCandidate != self->m_host &&
				self->m_electionUnanimous)
			{
				LOGINFO(g_log, "<peer(%s)> we have been promoted to host status.", m_name);
				m_host = self->m_self;
			}
			// detect if a peer has been elected
			else
			{
				bdHashMap<int, PeerStateRef>::Iterator it = m_peerStates.getIterator();
				while(it)
				{
					// check current peer state
					PeerStateRef peer = m_peerStates.getValue(it);
					m_peerStates.next(it);
			
					// promoted peer
					if(	peer->m_self == peer->m_host && 
						peer->m_self == peer->m_electionCandidate && 
						peer->m_self != m_host)
					{

						LOGINFO(g_log, "<peer(%s)> '%s' has been promoted to host status.", m_name, peer->m_name);
						m_host = peer->m_self;
					}
				}
				m_peerStates.releaseIterator(it);
			}
		}

		void updateSqn()
		{
			U64 now = Time::GetPerformanceCounter();
						
			if((now - m_sqnTimestamp) > 1000 * 1000)
			{
				m_sqnTimestamp = now;
				m_sqn++;
			}
		}

		void update()
		{
			updateSqn();
			updateSelf();
			updateConnections();
			updateMigration();
		}

		const int						m_self;
		char							m_name[128];
		int								m_host;
		int								m_sqn;	
		U64								m_sqnTimestamp;
		bdHashMap<int, PeerStateRef>	m_peerStates;
		PeerStateRef					m_sync;
	};

	class C_MigrationSession
	{
	public:
		C_MigrationSession()
		{}

		~C_MigrationSession()
		{}

		void initialise();
		void shutdown();
		void update();
		void render();
		
		PeerRef	newPeer();
		PeerRef	newPeer(int self, int host);
		void	deletePeer(PeerRef peer);
		PeerRef	getPeer(int id);
		PeerRef	getHost();

		void updateInputs();
		void updateHost();
		void transmitStates();
		void updatePeers();

		bdHashMap<int, PeerRef> m_peers;
	};

	PeerRef	C_MigrationSession::newPeer()
	{
		PeerRef host = getHost();
		if(host.isNull()) return NULL;
		
		for(int i = 0; i < 20; i ++)
		{
			if(getPeer(i).isNull())
			{
				PeerRef	peer = newPeer(i, host->m_self);
				return peer;
			}
		}
		return NULL;
	}

	PeerRef	C_MigrationSession::newPeer(int self, int host)
	{
		char name[32];
		sprintf_s(name, sizeof(name), "%c", 'A' + self);
		PeerRef peer = new Peer(self, host, name);
		m_peers.put(self, peer);
		return peer;
	}

	void C_MigrationSession::deletePeer(PeerRef peer)
	{
		if(peer.notNull())
			m_peers.remove(peer->m_self, peer);

		bdHashMap<int, PeerRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			PeerRef p = m_peers.getValue(it);
			m_peers.next(it);

			// signal disconnection.
			p->invalidateState(peer->m_self);
		}
		m_peers.releaseIterator(it);
	}

	PeerRef	C_MigrationSession::getHost()
	{
		bdHashMap<int, PeerRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			PeerRef peer = m_peers.getValue(it);
			m_peers.next(it);
			
			if(peer->m_self == peer->m_host)
			{
				m_peers.releaseIterator(it);
				return peer;
			}
		}
		m_peers.releaseIterator(it);
		return NULL;
	}

	PeerRef	C_MigrationSession::getPeer(int id)
	{
		PeerRef	p;
		m_peers.get(id, p);
		return p;
	}

	void C_MigrationSession::shutdown()
	{
		m_peers.clear();
	}

	void C_MigrationSession::initialise()
	{
		shutdown();
		newPeer(0, 0);
	}

	static int kickpeer = 0;

	void C_MigrationSession::updateInputs()
	{
		if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadLeft, 10, 5))
		{
			if(kickpeer > 0)
				kickpeer--;
		}

		if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadRight, 10, 5))
		{
			if(kickpeer < 20)
				kickpeer++;
		}

		if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_ButtonB, 10, 5))
		{
			PeerRef peer = getPeer(kickpeer);

			if(peer.notNull())
				deletePeer(peer);
		}

		if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_ButtonA, 10, 5))
		{
			if(m_peers.getSize() < 20)
			{
				newPeer();
			}

			if(m_peers.getSize() == 0)
			{
				newPeer(0, 0);
			}
		}

		if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_ButtonY, 10, 5))
		{
			PeerRef host = getHost();
			if(host.notNull())
			{
				host->promotePeer();
			}
		}

	}

	void C_MigrationSession::updateHost()
	{
		PeerRef	host = getHost();
		if(host.isNull())
			return;

		// create / destroy states for each peer
		for(int i = 0; i < 100; i ++)
		{
			PeerRef peer = getPeer(i);
			PeerStateRef state = host->getState(i);
			
			if(state.notNull() && peer.isNull())
			{
				host->removeState(i);
			}

			if(state.isNull() && peer.notNull())
			{
				host->createState(i);
			}
		}
	}

	void C_MigrationSession::transmitStates()
	{
		int a[100];
		int acount = 0;

		int b[100];
		int bcount = 0;

		// list peers
		bdHashMap<int, PeerRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			PeerRef peer = m_peers.getValue(it);
			int key = m_peers.getKey(it);
			m_peers.next(it);
		
			a[acount++] = key;
			b[bcount++] = key;
			if(acount >= 100 || bcount >= 100) 
				break;
		}
		m_peers.releaseIterator(it);
			

		// scramble order
		for(int i = 0; i < netPlatform::Core::instance()->randomInt(10) + 10; i ++)
		{
			int k = netPlatform::Core::instance()->randomInt(acount);
			int l = netPlatform::Core::instance()->randomInt(acount);

			int temp = a[k];
			a[k] = a[l];
			a[l] = temp;
		}

		// scramble order
		for(int i = 0; i < netPlatform::Core::instance()->randomInt(10) + 10; i ++)
		{
			int k = netPlatform::Core::instance()->randomInt(bcount);
			int l = netPlatform::Core::instance()->randomInt(bcount);

			int temp = b[k];
			b[k] = b[l];
			b[l] = temp;
		}

		for(int i = 0; i < acount; i ++)
		{
			for(int j = 0; j < bcount; j ++)
			{
				PeerRef peera = getPeer(a[i]);
				PeerRef peerb = getPeer(b[j]);

				if(peera.notNull() && peerb.notNull() && (peera != peerb))
				{
					PeerStateRef state = peerb->selfState();

					if(state.notNull())
						peera->receiveState(state);
				}
			}
		}
	}


	void C_MigrationSession::updatePeers()
	{	
		PeerRef	host = getHost();

		bdHashMap<int, PeerRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			PeerRef peer = m_peers.getValue(it);
			m_peers.next(it);
			peer->update();
			
			if(host.notNull() && (host != peer))
				peer->receiveState(host->selfState());
		}
		m_peers.releaseIterator(it);
	}

	void C_MigrationSession::update()
	{	
		updateInputs();

		transmitStates();

		updateHost();

		updatePeers();
	}

	void C_MigrationSession::render()
	{
		DbUtil::I_DebugRenderPtr debugRenderPtr = DbUtil::DefaultDebugRenderSys()->Immediate();
		float y = 20.0f;

		char buff[512];
		sprintf_s(buff, sizeof(buff), "button (A)-newPeer(), Button(B)-removePeer(%d), Button(Y)-promotePeer().", kickpeer);
		debugRenderPtr->Draw2dText(Maths::HVec2(40, y), buff, Maths::RGBA(0xffffffff), 32.0f);
		y += 40.0f;

		bdHashMap<int, PeerRef>::Iterator it = m_peers.getIterator();
		while(it)
		{
			PeerRef peer = m_peers.getValue(it);
			m_peers.next(it);
			y = peer->render(debugRenderPtr, y);
		}
		m_peers.releaseIterator(it);
	}
}


C_TestMigrationSection::C_TestMigrationSection(const Util::C_String& name) 
: Framework::C_TestSection(name)
{
	netPlatform::Platform::Startup();

	m_session = new TestMigration::C_MigrationSession();
	m_session->initialise();
}

C_TestMigrationSection::~C_TestMigrationSection()
{
	m_session->shutdown();
	delete m_session;

	netPlatform::Platform::Destroy();
}

void C_TestMigrationSection::OnUpdate()
{
	m_session->update();
}

void C_TestMigrationSection::OnRender2DView()
{
	m_session->render();
}

