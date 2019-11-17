////////////////////////////////////////////////////////////////////////////
//	Created 	: 06.10.2008
//	Author		: Sergey Chechin, Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_RESOURCES_H_INCLUDED
#define XRAY_RESOURCES_H_INCLUDED

#include <xray/resources_queries_result.h>
#include <xray/resources_cook_base.h>
#include <xray/resources_resource_ptr.h>

namespace xray {
namespace resources {

enum	
{
	normal_priority			=	0,
	high_priority			=	3,
	low_priority			=	-3
};

typedef u32					query_flag_enum;
enum
{
	query_flag_recursive	=	1,
};

struct request
{
	static char const	path_separator	=	'|';

	pcstr				path;
	class_id			id;
};

struct creation_request
{
	pcstr				name;
	const_buffer		data;
	class_id			id;
};

char const				physical_path_char	=	'@';

XRAY_CORE_API	void   	start_query_transaction	( );
XRAY_CORE_API	void   	end_query_transaction	( );

XRAY_CORE_API	long   	query_resources			( request const 			requests[], 
												  u32						count, 
												  query_callback const &	, 
												  memory::base_allocator *	,
												  user_data_variant const *	user_data[] = NULL,
												  int						priority	=	0,
												  query_result_for_cook *	parent		=	NULL );

XRAY_CORE_API	long   	query_resource			( pcstr						request_path, 
												  class_id					class_id, 
												  query_callback const &	, 
												  memory::base_allocator *	,
												  user_data_variant	const *	user_data	=	NULL,
												  int						priority	=	0,
												  query_result_for_cook *	parent		=	NULL );

XRAY_CORE_API	void   	query_resources_and_wait( request const *			requests, 
												  u32						count, 
												  query_callback const &	, 
												  memory::base_allocator *	,
												  user_data_variant const *	user_data[] =	NULL,
												  int						priority	=	0,
												  query_result_for_cook *	parent		=	NULL );

XRAY_CORE_API	void   	query_resource_and_wait	( pcstr						request_path, 
												  class_id					class_id, 
												  query_callback const &	, 
												  memory::base_allocator *	,
												  user_data_variant const *	user_data	=	NULL,
												  int						priority	=	0,
												  query_result_for_cook *	parent		=	NULL );

XRAY_CORE_API	long   	query_create_resources	( creation_request const *  requests, 
											 	  u32						count, 
											 	  query_callback const &	, 
											 	  memory::base_allocator *	,
												  user_data_variant const *	user_data[] =	NULL,
											 	  int						priority	=	0,
											 	  query_result_for_cook *	parent		=	NULL);

XRAY_CORE_API	void   	query_create_resources_and_wait
												( creation_request const *  requests, 
											 	  u32						count, 
											 	  query_callback const &	, 
											 	  memory::base_allocator *	,
												  user_data_variant	const *	user_data[] =	NULL,
											 	  int						priority	=	0,
											 	  query_result_for_cook *	parent		=	NULL);

XRAY_CORE_API	long   	query_create_resource	( pcstr						request_name,
												  const_buffer				src_data, 
											 	  class_id					class_id, 
											 	  query_callback const &	, 
											 	  memory::base_allocator *	,
												  user_data_variant	const *	user_data	=	NULL,
											 	  int						priority	=	0,
											 	  query_result_for_cook *	parent		=	NULL);

// request_mask can contain * and ?
XRAY_CORE_API	void   	query_resources_by_mask	( pcstr				request_mask, 
												  class_id			class_id	, 
												  query_callback const &		, 
												  memory::base_allocator *		,
												  query_flag_enum	flags		=	0, 
												  int				priority	=	0,
												  query_result_for_cook *	parent	=	NULL);

XRAY_CORE_API	void	finalize_thread_usage ( bool calling_from_main_thread );
XRAY_CORE_API	void	wait_and_dispatch_callbacks	( bool calling_from_main_thread );

XRAY_CORE_API	void	start_cooks_registration	( );
XRAY_CORE_API	void	finish_cooks_registration	( );

XRAY_CORE_API	void	dispatch_callbacks	( );
XRAY_CORE_API	void	tick				( );

template < int count >
inline			long   	query_resources		( request const				(& requests)[ count ], 
											  query_callback const &	callback, 
											  memory::base_allocator *	allocator,
											  user_data_variant const *	user_data[] =	NULL,
											  int						priority	=	0,
											  query_result_for_cook *	parent		=	NULL)
{
	XRAY_UNREFERENCED_PARAMETER				(parent);
	return query_resources (requests, array_size(requests), callback, allocator, user_data, priority, parent);
}

template < int count >
inline			long   	query_create_resources		( creation_request const	(& requests)[ count ], 
													  query_callback const &	callback, 
													  memory::base_allocator *	allocator,
													  user_data_variant	const *	user_data[]	=	NULL,
													  int						priority	=	0,
													  query_result_for_cook *	parent		=	NULL)
{
	XRAY_UNREFERENCED_PARAMETER				(parent);
	return query_create_resources(requests, array_size(requests), callback, allocator, user_data, priority, parent);
}

} // namespace resources
} // namespace xray

#endif// XRAY_CORE_RESOURCES_H_INCLUDED