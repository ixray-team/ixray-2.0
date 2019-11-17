////////////////////////////////////////////////////////////////////////////
//	Created		: 26.01.2010
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef FS_PLATFORM_CONFIGURATION_H_INCLUDED
#define FS_PLATFORM_CONFIGURATION_H_INCLUDED

namespace xray {

#if XRAY_PLATFORM_WINDOWS
typedef		u64			file_size_type;
#else // #if XRAY_PLATFORM_WINDOWS
typedef		u32			file_size_type;
#endif // #if XRAY_PLATFORM_WINDOWS

namespace fs {

enum pointer_for_fat_size_enum {	pointer_for_fat_32bit, 
									pointer_for_fat_64bit,
#if XRAY_PLATFORM_WINDOWS
									pointer_for_fat_size_platform_default	=	pointer_for_fat_64bit,
#else // #if XRAY_PLATFORM_WINDOWS
									pointer_for_fat_size_platform_default	=	pointer_for_fat_32bit,
#endif // #if XRAY_PLATFORM_WINDOWS
								};

template <pointer_for_fat_size_enum pointer_for_fat_size>
struct platform_to_file_size_type
{
	typedef		u32					file_size_type;
};

template <>
struct platform_to_file_size_type<pointer_for_fat_64bit>
{
	typedef		u64					file_size_type;
};

// forward declarations:
template <pointer_for_fat_size_enum pointer_for_fat_size = pointer_for_fat_size_platform_default>
class fat_node;

class file_system;

} // namespace fs
} // namespace xray

#endif // #ifndef FS_PLATFORM_CONFIGURATION_H_INCLUDED