//-------------------------------------------------------------------------------------------
//	Created		: 20.01.2010
//	Author		: Sergey Prishchepa
//	Copyright (C) GSC Game World - 2010
//-------------------------------------------------------------------------------------------

#ifndef ITEM_PROPERTIES_PANEL_BASE_H_INCLUDED
#define ITEM_PROPERTIES_PANEL_BASE_H_INCLUDED

#include "property_grid.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace Flobbster::Windows::Forms;

using xray::editor::wpf_controls::property_grid_host;


namespace xray {
namespace editor {
namespace controls {
	ref class document_editor_base;

	public ref class item_properties_panel_base : public WeifenLuo::WinFormsUI::Docking::DockContent
	{
	public:
		item_properties_panel_base(document_editor_base^ ed, String^ panel_name):m_editor(ed)
		{
			InitializeComponent();
			this->Text = panel_name;
			in_constructor();
		}
		item_properties_panel_base(document_editor_base^ ed):m_editor(ed)
		{
			InitializeComponent();
			in_constructor();
		}
		virtual	void	show_properties		(Object^ object);

	protected:
		~item_properties_panel_base()
		{
			if(components)
				delete components;
		}

	private:
						item_properties_panel_base(){};
		void			in_constructor			();

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->SuspendLayout();
			// 
			// item_properties_panel_base
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(292, 273);
			this->DockAreas = static_cast<WeifenLuo::WinFormsUI::Docking::DockAreas>(((((WeifenLuo::WinFormsUI::Docking::DockAreas::Float | WeifenLuo::WinFormsUI::Docking::DockAreas::DockLeft) 
				| WeifenLuo::WinFormsUI::Docking::DockAreas::DockRight) 
				| WeifenLuo::WinFormsUI::Docking::DockAreas::DockTop) 
				| WeifenLuo::WinFormsUI::Docking::DockAreas::DockBottom));
			this->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(204)));
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"item_properties_panel_base";
			this->Text = L"Item properties";
			this->ResumeLayout(false);

		}
#pragma endregion

	protected:
		property_grid_host^					m_property_grid_host;
		document_editor_base^				m_editor;

	private:
		System::ComponentModel::Container^	components;

	public:
		property property_grid_host^					property_grid_control
		{
			property_grid_host^	get	()					{return m_property_grid_host;}
		}
	}; // class item_properties_panel_base
} // namespace controls
} // namespace editor
} // namespace xray
#endif 