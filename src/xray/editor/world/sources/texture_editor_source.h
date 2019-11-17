////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef TEXTURE_EDITOR_SOURCE_H_INCLUDED
#define TEXTURE_EDITOR_SOURCE_H_INCLUDED

#include "texture_options.h"

using namespace System;

using xray::editor::controls::tree_view_source;
using xray::editor::controls::tree_view;

typedef xray::editor::texture_options::ETType ETType;

namespace xray {
	namespace editor {

		ref class texture_editor_source_filter
		{
		public:
			texture_editor_source_filter(ETType* filter_type):
				m_filter_type(filter_type)
			{}

			ETType* m_filter_type;
		};
		ref class texture_editor_source:tree_view_source
		{
		public:
			texture_editor_source::texture_editor_source();
			texture_editor_source::texture_editor_source(Object^ filter);			

		private:
			tree_view^				m_parent;
			texture_options::ETType	m_filter_type;

		public:
			virtual property tree_view^		parent
			{
				tree_view^			get	(){return m_parent;};
				void				set	(tree_view^ value){m_parent = value;};
			};

		private:
			void	on_fs_iterator_ready	(xray::resources::fs_iterator fs_it);
			void	process_fs				(xray::resources::fs_iterator it,  String^ file_path);

		public:
			virtual void refresh();

		}; // class texture_editor_source

	}//namespace editor
}//namespace xray

#endif // #ifndef TEXTURE_EDITOR_SOURCE_H_INCLUDED