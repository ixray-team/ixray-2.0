////////////////////////////////////////////////////////////////////////////
//	Created		: 22.04.2010
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_SOUND_SOUND_H_INCLUDED
#define XRAY_SOUND_SOUND_H_INCLUDED

namespace xray {
namespace sound {

class sound_object : public resources::unmanaged_resource 
{
public:
	virtual void				play			( )						= 0;
	virtual void				stop			( )						= 0;
	virtual void				set_position	( float3 const& p )		= 0;
	virtual float3 const&		get_position	( ) const				= 0;

protected:
	XRAY_DECLARE_PURE_VIRTUAL_DESTRUCTOR( sound_object )
}; // class visual

typedef	resources::resource_ptr<
			sound_object,
			resources::unmanaged_intrusive_base
		> sound_ptr;

} // namespace sound
} // namespace xray

#endif // #ifndef XRAY_SOUND_SOUND_H_INCLUDED