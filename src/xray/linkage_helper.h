////////////////////////////////////////////////////////////////////////////
//	Created		: 22.06.2009
//	Author		: Sergey Chechin
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_LINKAGE_HELPER_H_INCLUDED
#define XRAY_LINKAGE_HELPER_H_INCLUDED

#define DECLARE_LINKAGE_ID(linkage_id)  						\
	namespace xray {											\
		namespace linkage_helpers {								\
			int linkage_id;										\
		}														\
	} // namespace xray

#define INCLUDE_TO_LINKAGE(linkage_id)  						\
	namespace xray {											\
		namespace linkage_helpers {								\
			extern int linkage_id;								\
			static int* linkage_id ## refferer = &linkage_id;	\
		}														\
	} // namespace xray

#endif // #ifndef XRAY_LINKAGE_HELPER_H_INCLUDED