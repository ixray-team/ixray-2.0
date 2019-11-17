////////////////////////////////////////////////////////////////////////////
//	Created		: 12.03.2009
//	Author		: Andrew Kolomiets
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_SOLID_VISUAL_H_INCLUDED
#define TOOL_SOLID_VISUAL_H_INCLUDED

#include "tool_base.h"

namespace xray {
namespace editor {

ref class solid_visual_tool_tab;

public ref class tool_solid_visual :public tool_base
{
	typedef tool_base	super;
public:
						tool_solid_visual		(level_editor^ le);
	virtual				~tool_solid_visual		();

	virtual object_base^ load_object			(xray::configs::lua_config_value const& t) override;
	virtual object_base^ create_object			(System::String^ name) override;
	virtual object_base^ create_raw_object		(System::String^ id) override;
	virtual void		destroy_object			(object_base^ o) override;
	virtual tool_tab^	ui						() override;

	void				load_library			();
private:
	void				process_recursive_names	(xray::resources::fs_iterator it, System::Windows::Forms::TreeNodeCollection^ nodes, int& count);
	void				on_library_fs_iterator_ready(xray::resources::fs_iterator fs_it);
	void				get_active_tool_name	(System::String^% name);

	solid_visual_tool_tab^	m_tool_tab;
}; // class tool_solid_visual

} // namespace editor
} // namespace xray

#endif // #ifndef TOOL_SOLID_VISUAL_H_INCLUDED