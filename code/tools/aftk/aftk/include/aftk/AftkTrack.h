#ifndef AFTK_TRACK_H
#define AFTK_TRACK_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_ClipNode;

	class C_Track : public I_Object
	{
	public:
		///\brief Constructor.
		///
		C_Track();

		///\brief Copy constructor.
		///
		///@param rhs The track to copy.
		C_Track(const C_Track& rhs);

		///\brief Destructor.
		///
		virtual ~C_Track();

		///\brief Asignment operator.
		///
		///@param rhs The track to copy.
		///@return A reference to this track.
		const C_Track& operator=(const C_Track& rhs);

		///\brief Adds a new clip to this track
		///
		///@param pClipNode The clip to add.
		///@return The index of this clip in this track.
		U32	AddClipNodePtr(C_ClipNode* pClipNode);

		///\brief Sets whether this track is enabled or not.
		///
		///@param flag C_Track enabled state.
		void SetIsEnabled(bool flag);

		///\brief Returns a clip node.
		///
		///@param instance The clip node index.
		///@return The clip node.
		C_ClipNode* GetClipNodePtr(U32 instance);

		///\brief Returns a clip node.
		///
		///@param instance The clip node index.
		///@return The clip node.
		const C_ClipNode* GetClipNodePtr(U32 instance) const;

		///\brief Returns the number of clip nodes in this track.
		///
		///@return The number of clip nodes.
		U32 GetNumClipNodes() const;

		///\brief Returns the lowest value of x for which there is a key in this track.
		///
		///Note that the results of this are NOT cached, they are re-evaluated for each call.
		///@return The lowest value of x.
		F32 GetStartX() const;

		///\brief Returns the highest value of x for which there is a key in this track.
		///
		///Note that the results of this are NOT cached, they are re-evaluated for each call.
		///@return The highest value of x.
		F32 GetEndX() const;

		///\brief Returns the duration of this track in frames.
		///
		///Note that the results of this are NOT cached, they are re-evaluated for each call.
		///@return The duration of this track in frames.
		F32 GetLength() const;

		///\brief Returns whether this track is enabled or not.
		///
		///@return C_Track enabled state.
		bool IsEnabled() const;

		///\brief Returns the clip which is active at the given value of x.
		///
		///Note: Clips should not be allowed to overlap.
		///If no clip is active, 0 is returned.
		///@param x The value of x to query for.
		///@return The active clip.
		C_ClipNode* GetActiveClip(F32 x) const;

		///\brief Deletes a clip node from this track.
		///
		///@param pClipNode The clip node to delete.
		void DeleteClipNodePtr(C_ClipNode* pClipNode);

		///\brief Deletes a clip node from this track.
		///
		///@param instance The index of the clip node in this track to delete.
		void DeleteClipNodePtr(U32 instance);

		void DeleteClipNodes();

		///\brief Get this objects's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this object. For example, spline, camera actor etc.
		///
		///@return object's type enum.
		virtual E_ObjType GetType() const;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool HasType(E_ObjType type) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		///\brief Retruns the name of this object.
		///
		///@return object name.
		virtual const std::string& GetName() const;

		virtual std::string GetPath() const;

	protected:
		Ut::UtVector< Ut::DynamicVector<C_NullPlug*> > m_clipNodes;
		bool m_enabled;
		static const std::string m_trackName;
		friend class C_NativeTrack;
	};
}

#endif

