////////////////////////////////////////////////////////////////////////////
//	Created		: 17.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_LIGHT_H_INCLUDED
#define TOOL_LIGHT_H_INCLUDED

#include "tool_base.h"
#include <xray/render/base/world.h>
#include <xray/render/base/visual.h>


namespace xray {
namespace editor {

ref class light_tool_tab;

public ref class tool_light  :public tool_base
{
	typedef tool_base	super;
public:
						tool_light				(level_editor^ le);
	virtual				~tool_light				();

	virtual object_base^ create_object			(System::String^ name) override;
	virtual object_base^ load_object			(xray::configs::lua_config_value const& t) override;
	virtual object_base^ create_raw_object		(System::String^ id) override;
	virtual void		destroy_object			(object_base^ o) override;

	virtual void		on_make_new_library_object() override;
	virtual void		on_edit_library_object	(System::String^ name) override;

	virtual void		get_objects_list		(names_list list, enum_list_mode mode) override;

	void				load_library			();
	render::visual_ptr get_preview_model	(u32 idx);
	virtual	void		save_library			() override;
	virtual tool_tab^	ui						() override;

protected:
	virtual void		on_library_loaded		() override;

private:
			void		on_preview_visual_ready	(resources::unmanaged_resource_ptr data);
			void		on_resource_loaded		( xray::resources::queries_result& data );
			void		fill_tree_view			(tree_node_collection^ nodes);

	light_tool_tab^		m_tool_tab;
	cli::array<render::visual_ptr*>^	m_preview_models;
	int										m_preview_models_cnt;
}; // class tool_light

} // namespace editor
} // namespace xray

#endif // #ifndef TOOL_LIGHT_H_INCLUDED