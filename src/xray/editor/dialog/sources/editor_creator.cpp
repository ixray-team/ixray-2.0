//-------------------------------------------------------------------------------------------
//	Created		: 10.06.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#include "pch.h"
#include "editor_creator.h"
#include "dialog_editor.h"
namespace xray{
namespace dialog_editor{

xray::editor_base::editor_base^ editor_creator::create_editor(System::String^ resources_path)
{
	m_ed = gcnew dialog_editor_impl(resources_path);
	m_ed->initialize();
	m_ed->load(nullptr);
	m_ed->load_settings();
	return m_ed;
}

void editor_creator::destroy_dialog_editor()
{
	//	delete m_ed;
}

void editor_creator::memory_allocator(xray::dialog_editor::allocator_type* allocator)
{
	ASSERT(!g_allocator);
	g_allocator = allocator;
}
} //namespace dialog_editor
} //namespace xray

