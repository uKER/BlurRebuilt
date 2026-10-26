// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef REPLICA_ADAPTOR_H
#define REPLICA_ADAPTOR_H

#include <bdExample/bdExampleCommonClientServer/bdExampleCommonCS.h>

#define SPHERE_REPLICA  (BD_USER_TYPE + 1)

BD_REFERENCE(ReplicaAdaptor);

class ReplicaAdaptor : public bdReplica
{
	public:

		ReplicaAdaptor();

		Sphere& getSphere();

		virtual void initialize();

		virtual void applyCommand(bdUserCommand *const command,
								  const bdBool newCommand);

		virtual void serialize(bdBitBufferRef buffer);

		virtual bdBool deserialize(bdBitBufferRef buffer);

		virtual void extrapolate(const bdFloat32 secs, 
								 const bdBool isReplay);

		virtual void interpolate(bdInterpolationInterface *const targetState, 
								 const bdFloat32 targetRatio);


	protected:

		Sphere m_sphere;
};

#endif // REPLICA_ADAPTOR_H
