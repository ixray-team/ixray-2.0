//-------------------------------------------------------------------------------------------
//	Created		: 10.06.2010
//	Author		: Sergey Pryshchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------
#ifndef EDITOR_CREATOR_H_INCLUDED
#define EDITOR_CREATOR_H_INCLUDED

namespace xray {
namespace editor_base {interface class editor_base;}
namespace dialog_editor {

	public ref class editor_creator
	{
	public:
		static xray::editor_base::editor_base^	create_editor			(System::String^ resources_path);
		static void								destroy_dialog_editor	();
		static void								memory_allocator		(allocator_type* allocator);

	private:
		static xray::editor_base::editor_base^	m_ed;
	}; // ref class editor_creator
} // namespace dialog_editor
} // namespace xray
#endif // #ifndef XRAY_DIALOG_EDITOR_API_H_INCLUDED