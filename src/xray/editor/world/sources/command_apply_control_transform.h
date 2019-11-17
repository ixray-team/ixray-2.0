////////////////////////////////////////////////////////////////////////////
//	Created		: 02.04.2009
//	Author		: Armen Abroyan
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_APPLY_CONTROL_TRANSFORM_H_INCLUDED
#define COMMAND_APPLY_CONTROL_TRANSFORM_H_INCLUDED

#include "project_defines.h"


namespace xray {
namespace editor {

ref class transform_control_base;
ref class level_editor;

public ref class command_apply_control_transform : public xray::editor_base::command
{
public:
	command_apply_control_transform	( level_editor^ le, transform_control_base^ control );


	virtual bool commit			() override;
	virtual void rollback		() override;

	virtual bool end_preview	() override;

private:

	level_editor^					m_level_editor;
	transform_control_base^			m_transform_control;
	id_matrices_list				m_id_matrices;
	bool							m_first_run;

}; // class command_apply_control_transform

} // namespace editor
} // namespace xray

#endif // #ifndef COMMAND_APPLY_CONTROL_TRANSFORM_H_INCLUDED