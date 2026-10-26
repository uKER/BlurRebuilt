#ifndef ARCHIVE_SERIALISE_H
#define ARCHIVE_SERIALISE_H

#include "btl/utility/types.h"

namespace Arch
{

template<typename Archive, typename T>
FORCE_INLINE void Serialise(Archive& ar, const T& t)
{
	typedef typename Btl::StripConst<T>::Result T_NC;
	const_cast<T_NC&>(t).Serialise(ar);
}

template<typename Archive, typename T>
FORCE_INLINE void Deserialise(Archive& ar, T& t)
{
	t.Deserialise(ar);
}

// If serialise is written using & and has non-const arguments, 
// deserialise may be identical to serialise so there's no need to write it out again.
#define AUTO_DESERIALISE_MEMBER()	template <typename Archive> \
									void Deserialise(Archive& ar) {Serialise(ar);}

// Works if type_ is not a template (if it is, write out without the macro)
#define AUTO_DESERIALISE_FREE(type_)	template<typename Archive> \
										void Deserialise(Archive& ar, type_& t) \
										{Serialise(ar, t);}

// Serialise the base class.
#define SERIALISE_BASE_MEMBER(base_, ar_) ar_ & static_cast<base_&>(*this);
#define SERIALISE_BASE_FREE(base_, ar_, t_) ar_ & static_cast<base_&>(t_);
#define DESERIALISE_BASE_MEMBER(base_, ar_) ar_ & static_cast<base_&>(*this);
#define DESERIALISE_BASE_FREE(base_, ar_, t_) ar_ & static_cast<base_&>(t_);

// Declare serialisation functions as friends of the class
#define FRIEND_SERIALISE(type_) template <typename Archive> friend void Serialise(Archive& ar, type_& t); \
								template <typename Archive> friend void Deserialise(Archive& ar, type_& t);

#define FRIEND_SERIALISE_CONST(type_)	template <typename Archive> friend void Serialise(Archive& ar, const type_& t); \
										template <typename Archive> friend void Deserialise(Archive& ar, type_& t);

#define VIRTUAL_SERIALISATION_KEY_DECL		virtual Arch::SerialisationKey GetSerialisationKey(); \

#define SERIALISATION_KEY(type_)			Arch::SerialisationKey type_::GetSerialisationKey()				\
											{																\
												Arch::SerialisationKey key = Arch::FindKey<type_>();		\
												assert(!key.IsNull() && "No GUID for type");				\
												return key;													\
											}
}

#endif


