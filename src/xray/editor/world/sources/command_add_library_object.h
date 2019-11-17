////////////////////////////////////////////////////////////////////////////
//	Created		: 30.03.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_ADD_LIBRARY_OBJECT_H_INCLUDED
#define COMMAND_ADD_LIBRARY_OBJECT_H_INCLUDED

namespace xray {
namespace editor {
ref class tool_base;

ref class level_editor;


public ref class command_add_library_object :	public xray::editor_base::command
{
public:


	command_add_library_object	(	level_editor^ le, 
									tool_base^ tool, 
									System::String^ library_name, 
									math::float4x4 transform,
									bool select /*= true*/ );

	virtual ~command_add_library_object			();

	virtual bool commit		() override;
	virtual void rollback	() override;

private:	// Data for execute

	level_editor^				m_level_editor;
	tool_base^					m_tool;
	System::String^				m_library_name;
	math::float4x4*				m_transform;
	bool						m_select;
	
private:	// Data for undo
	u32							m_object_id;
	System::String^				m_path;

}; // class command_add_library_object

}// namespace editor
}// namespace xray

#endif // #ifndef COMMAND_ADD_LIBRARY_OBJECT_H_INCLUDED