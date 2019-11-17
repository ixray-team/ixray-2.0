#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace xray {
namespace editor {

	ref class tool_terrain;
	ref class editor_control_base;

	ref class terrain_modifier_bump;
	/// <summary>
	/// Summary for terrain_bump_control_tab
	/// </summary>
	public ref class terrain_painter_control_tab : public System::Windows::Forms::UserControl
	{
	public:
		terrain_painter_control_tab(tool_terrain^ t):m_tool(t),m_in_sync(false)
		{
			InitializeComponent();
			in_constructor();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~terrain_painter_control_tab()
		{
			if (components)
			{
				delete components;
			}
		}
	void	on_control_activated(editor_control_base^ c);
	void	on_control_deactivated(editor_control_base^ c);

	void	in_constructor		();
	tool_terrain^				m_tool;






	private: System::Windows::Forms::ListView^  textures_list_view;




	protected: 

	protected: 
		bool						m_in_sync;
	void	sync				(bool b_sync_ui);























	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->textures_list_view = (gcnew System::Windows::Forms::ListView());
			this->SuspendLayout();
			// 
			// textures_list_view
			// 
			this->textures_list_view->Activation = System::Windows::Forms::ItemActivation::OneClick;
			this->textures_list_view->Dock = System::Windows::Forms::DockStyle::Fill;
			this->textures_list_view->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
			this->textures_list_view->HideSelection = false;
			this->textures_list_view->Location = System::Drawing::Point(0, 0);
			this->textures_list_view->MultiSelect = false;
			this->textures_list_view->Name = L"textures_list_view";
			this->textures_list_view->Size = System::Drawing::Size(239, 313);
			this->textures_list_view->TabIndex = 14;
			this->textures_list_view->UseCompatibleStateImageBehavior = false;
			this->textures_list_view->View = System::Windows::Forms::View::List;
			this->textures_list_view->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &terrain_painter_control_tab::textures_list_view_MouseClick);
			this->textures_list_view->DoubleClick += gcnew System::EventHandler(this, &terrain_painter_control_tab::textures_list_view_DoubleClick);
			this->textures_list_view->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &terrain_painter_control_tab::textures_list_view_MouseDown);
			// 
			// terrain_painter_control_tab
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->textures_list_view);
			this->Name = L"terrain_painter_control_tab";
			this->Size = System::Drawing::Size(239, 313);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: 
		void sync							(System::Object^ sender, System::EventArgs^ e);
		void external_property_changed		(editor_control_base^ c);
		void textures_list_view_DoubleClick	(System::Object^  sender, System::EventArgs^  e);
		bool select_texture					(System::String^ active_item);
		void textures_list_view_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
		void textures_list_view_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
};
} // namespace editor
} // namespace xray
