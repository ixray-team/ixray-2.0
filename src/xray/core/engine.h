////////////////////////////////////////////////////////////////////////////
//	Created 	: 21.09.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_CORE_ENGINE_H_INCLUDED
#define XRAY_CORE_ENGINE_H_INCLUDED

namespace xray {
namespace core {

struct XRAY_NOVTABLE engine {
	virtual			~engine					( ) { }
	virtual void 	exit					( int exit_code ) = 0;
	virtual void 	set_exit_code			( int exit_code ) = 0;
	virtual int  	get_exit_code			( ) const = 0;
	virtual	pcstr	get_resource_path		( ) const = 0;
	virtual	pcstr	get_mounts_path			( ) const = 0;
	virtual	pcstr	get_underscore_G_path	( ) const = 0;
}; // class engine

} // namespace core
} // namespace xray

#endif // #ifndef XRAY_CORE_ENGINE_H_INCLUDED