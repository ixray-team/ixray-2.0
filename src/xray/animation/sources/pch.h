////////////////////////////////////////////////////////////////////////////
//	Created 	: 17.12.2008
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef PCH_H_INCLUDED
#define PCH_H_INCLUDED

#define XRAY_ANIMATION_BUILDING

#ifndef XRAY_STATIC_LIBRARIES
#	define XRAY_ENGINE_BUILDING
#endif // #ifndef XRAY_STATIC_LIBRARIES

#define XRAY_LOG_MODULE_INITIATOR	"animation"
#include <xray/extensions.h>

#if XRAY_PLATFORM_WINDOWS_32 && !defined(MASTER_GOLD)
#	define	XRAY_USE_MAYA_ANIMATION	1
#else // #if XRAY_PLATFORM_WINDOWS_32 && !defined(MASTER_GOLD)
#	define	XRAY_USE_MAYA_ANIMATION	0
#endif // #if XRAY_PLATFORM_WINDOWS_32 && !defined(MASTER_GOLD)

#include "animation_memory.h"

#endif // #ifndef PCH_H_INCLUDED