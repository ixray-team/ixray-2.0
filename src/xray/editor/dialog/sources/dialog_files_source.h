//-------------------------------------------------------------------------------------------
//	Created		: 17.12.2009
//	Author		: Sergey Prishchepa
//	Copyright (C) GSC Game World - 2009
//-------------------------------------------------------------------------------------------
#ifndef DAILOG_FILES_SOURCE_H_INCLUDED
#define DAILOG_FILES_SOURCE_H_INCLUDED

using namespace System;
using namespace System::Windows::Forms;
using xray::editor::controls::tree_view_source;
using xray::editor::controls::tree_view;

namespace xray {
namespace dialog_editor {
	ref class dialogs_node_sorter : public System::Collections::IComparer 
	{
	public:
		virtual int Compare	(Object^ x, Object^ y);
	}; // ref class dialogs_node_sorter

	ref class dialog_files_source : public xray::editor::controls::tree_view_source
	{
	public:
		virtual	void			refresh			();
		virtual property	tree_view^			parent
		{
			tree_view^			get				();
			void				set				(tree_view^ value);
		};

	private:
				void	on_fs_iterator_ready	(xray::resources::fs_iterator fs_it);
				void	process_fs				(xray::resources::fs_iterator it,  String^ file_path);

	private:
		tree_view^	m_parent;
	}; // class raw_files_source
}//namespace dialog_editor
}//namespace xray
#endif // #ifndef DAILOG_FILES_SOURCE_H_INCLUDED