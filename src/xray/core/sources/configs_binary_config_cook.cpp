////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Dmitriy Iassenev
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "configs_binary_config_cook.h"
#include "configs_binary_config.h"
#include <xray/fs_utils.h>
#include <xray/resources_fs.h>
#include <xray/memory_stream.h>

namespace xray {
namespace core {
namespace configs {

mutable_buffer   binary_config_cook_impl::allocate_resource	(resources::query_result_for_cook &	in_query, 
														 const_buffer						raw_file_data, 
														 bool								file_exist)
{
	XRAY_UNREFERENCED_PARAMETERS	(&in_query, &raw_file_data, file_exist);
	return							mutable_buffer (XRAY_MALLOC_IMPL(memory::g_mt_allocator, sizeof(xray::configs::binary_config), "lua"), 
													sizeof(xray::configs::binary_config));
}

void   binary_config_cook_impl::destroy_resource	(resources::unmanaged_resource * resource)
{
	binary_config * config			= static_cast_checked<binary_config *>(resource);
	R_ASSERT						(config);
	config->~binary_config			();
}

void   binary_config_cook_impl::deallocate_resource	(pvoid buffer)
{
	XRAY_FREE_IMPL					(memory::g_mt_allocator, buffer);
}

void   binary_config_cook_impl::create_resource (resources::query_result_for_cook &	in_out_query, 
											const_buffer						raw_file_data,
											mutable_buffer						in_out_unmanaged_resource_buffer)
{
	binary_config * const config	= new (in_out_unmanaged_resource_buffer.c_ptr()) binary_config 
									((pcbyte)raw_file_data.c_ptr(), raw_file_data.size(), & memory::g_mt_allocator);

	in_out_query.set_unmanaged_resource	(config, resources::memory_type_non_cacheable_resource, in_out_unmanaged_resource_buffer.size());
	in_out_query.finish_query			(result_success);
}






pcstr resources_converted = "resources.converted/";
pcstr resources = "resources/";

void change_substring(fs::path_string& src_and_dest, pcstr what, pcstr to)
{
	xray::fs::path_string result;
	xray::fs::path_string::size_type pos = src_and_dest.find(what);

	if(pos!=src_and_dest.npos)
	{
		u32 what_len = xray::strings::length(what);
		result.assign(src_and_dest.begin(), src_and_dest.begin()+pos); // head
		result.append(to); // body
		result.append(src_and_dest.begin()+pos+what_len, src_and_dest.end()); // tail
		src_and_dest = result;
	}
}

binary_config_cook::binary_config_cook()
:super(resources::binary_config_class, reuse_true, use_user_thread_id)
{
}

void binary_config_cook::translate_query(resources::query_result& parent)
{
	// ready config query 
	fs::path_string req_path = parent.get_requested_path();

	// compare source and converted resource file age
	fs::path_info source_info;
	resources::get_path_info_by_logical_path(&source_info, req_path.c_str(), parent.get_user_allocator());

	fs::path_string	converted_resource_path = req_path;
	change_substring(converted_resource_path, resources, resources_converted);
	
	fs::path_info resource_info;
	fs::path_info::type_enum res_game = resources::get_path_info_by_logical_path(&resource_info, converted_resource_path.c_str(), parent.get_user_allocator());

	if(res_game==fs::path_info::type_file && // has converted entry
		source_info.file_last_modify_time<resource_info.file_last_modify_time)
	{// load converted binary config
		resources::query_resource(
			converted_resource_path.c_str(),
			resources::config_class,
			boost::bind(&binary_config_cook::on_binary_config_loaded, this, _1, &parent),
			parent.get_user_allocator()
			);
	}
	else
	{// load lua config
		resources::query_resource(
			req_path.c_str(),
			resources::config_lua_class,
			boost::bind(&binary_config_cook::on_lua_config_loaded, this, _1, &parent),
			parent.get_user_allocator()
			);
	}
}

void binary_config_cook::on_lua_config_loaded(resources::queries_result& data, resources::query_result_for_cook* parent_query)
{
	R_ASSERT(data.is_successful());

	xray::resources::query_result_for_user const& result = data[0];

	xray::configs::lua_config_ptr source_config_ptr = static_cast_resource_ptr<xray::configs::lua_config_ptr>(result.get_unmanaged_resource());
	xray::memory::stream str(parent_query->get_user_allocator());
	xray::configs::create_binary_config_buffer(source_config_ptr->get_root(), str);
	mutable_buffer buffer(str.get_buffer(), str.get_buffer_size());
	fs::path_string	converted_resource_path = parent_query->get_requested_path();
	change_substring(converted_resource_path, resources, resources_converted);
	parent_query->set_request_path(converted_resource_path.c_str());
	parent_query->save_generated_resource(buffer);
	xray::configs::binary_config_ptr resource_config_ptr = xray::configs::create_binary_config(buffer);
	parent_query->set_unmanaged_resource(resource_config_ptr.c_ptr(), xray::resources::memory_type_non_cacheable_resource, sizeof(binary_config));
	parent_query->finish_query(result_success);
}

void binary_config_cook::on_binary_config_loaded(resources::queries_result& data, resources::query_result_for_cook* parent_query)
{
	R_ASSERT(data.is_successful());

	xray::resources::query_result_for_user const& result = data[0];

	xray::configs::binary_config_ptr resource_config_ptr = static_cast_resource_ptr<xray::configs::binary_config_ptr>(result.get_unmanaged_resource());

	parent_query->set_unmanaged_resource(resource_config_ptr.c_ptr(), xray::resources::memory_type_non_cacheable_resource, sizeof(binary_config));
	parent_query->finish_query(result_success);
}

void binary_config_cook::delete_resource(resources::unmanaged_resource * resource)
{
	binary_config* config = static_cast_checked<binary_config*>(resource);
	R_ASSERT(config);
	config->~binary_config();
}

} // namespace configs
} // namespace core
} // namespace xray
