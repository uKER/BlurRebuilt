// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_INTERPOLATION_DATA_H
#define BD_INTERPOLATION_DATA_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplica.h>

class bdInterpolationInterface;

/// Data and functions used when interpolating a bdReplica.
/// To enable interpolation you must augment the replicas 
/// bdSecondaryReplicaData with bdInterpolationData, set an interpolation
/// target and activate the interpolation. 
/// bdSecondaryReplicaData::createInterpolationData should be used to create 
/// instances of bdInterpolationData.
class bdInterpolationData
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdInterpolationData();

		/// Destructor.
		~bdInterpolationData();

		/// Sets the object to be used as the interpolation target.
		/// bdInterpolationData will assume ownership of the \a target and
		/// will delete it on destruction. 
		/// \note You should not delete \a target, 
		/// bdInterpolationData::~bdInterpolationData will do it for you.
		/// \param target[in]	A pointer to the object to be used as the
		///						interpolation target.
		void setTarget(bdInterpolationInterface *const target);

		/// Gets the object used as the interpolation target.
		/// \return A pointer to the interpolation target object which may
		///			be BD_NULL.
		bdInterpolationInterface* getTarget();

		/// Activate or deactivate the interpolation.
		/// If \a active is true and an interpolation target has been set
		/// the target will be initialized with from \a replicas state and
		/// interpolation enabled. Otherwise interpolation is disabled. As the
		/// target is initialized from \a replica both the target and 
		/// \a replica must have compatible bdReplica::serialize and 
		/// bdInterpolationInterface::deserialize methods.
		/// \param active[in]	
		/// - True, if possible enables interpolation.
		///	- False, disables any interpolation.
		/// \param replica[in] The replica that is being interpolated.
		void setActive(const bdBool active, 
					   bdReplicaRef replica);

		/// Indicates whether interpolation is active.
		/// \return 
		/// - True if interpolation is enabled.
		///	- False if interpolation is disabled.
		bdBool getActive() const;
		
		/// Specifies the amount of time over which to interpolate.
		/// \param interpolationInterval[in] The time interval in seconds over
		///    which interpolation reaches the target state.
		void setInterpolationInterval(bdFloat32 interpolationInterval);

		/// Calculates how far the replica should be advanced towards the
		/// interpolation target.
		/// \param elapsedTime[in] The time in seconds since the previous 
		/// call to bdInterpolationData::getNextInterpolationStep.
		/// \return A ratio between 0.0 and 1.0.
		bdFloat32 getNextInterpolationStep(const bdFloat32 elapsedTime);

		/// Start a new interpolation interval.
		void startInterpolation();

		/// End the current interpolation interval.
		void endInterpolation();

	protected:

		/// A pointer to the object used as the interpolation target.
		bdInterpolationInterface* m_target;

		/// Indicates whether interpolation is on or off.
		bdBool m_active;		

		/// The average update rate by which we receive updates from the server.
		/// Used to calculate how far we should advance towards the 
		/// interpolation target during the next interpolation step.
		bdFloat32 m_averageUpdateInterval;

		/// The total time elapsed since we started the current interpolation.
		bdFloat32 m_elapsedTime;

		/// The maximum number milliseconds of interpolation to perform.
		bdFloat32 m_interpolationInterval;

		/// Flag to indicate that an interpolation break has occurred in order to suppress
		/// any interpolation
		bdBool m_interpolationBreak;

};

#endif // BD_INTERPOLATION_DATA_H
