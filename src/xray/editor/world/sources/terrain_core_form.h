#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "terrain_quad.h"

namespace xray {
namespace editor {
	ref class tool_terrain;
	ref class terrain_core;
	value struct terrain_node_key;
	/// <summary>
	/// Summary for terrain_core_form
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class terrain_core_form : public System::Windows::Forms::Form
	{
	public:
		terrain_core_form(tool_terrain^ t)
		:m_tool(t)
		{
			InitializeComponent();
			in_constructor		();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~terrain_core_form()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::PropertyGrid^  propertyGrid;
	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::Splitter^  splitter1;
	private: System::Windows::Forms::Panel^  global_map_panel;
	private: System::Windows::Forms::ContextMenuStrip^  global_map_menu;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::ToolStripStatusLabel^  label1;
	private: System::Windows::Forms::Button^  button_move_down;
	private: System::Windows::Forms::Button^  button_move_right;
	private: System::Windows::Forms::Button^  button_move_up;
	private: System::Windows::Forms::Button^  button_move_left;
	private: System::Windows::Forms::Button^  import_button;


	private: System::ComponentModel::IContainer^  components;




	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->import_button = (gcnew System::Windows::Forms::Button());
			this->button_move_down = (gcnew System::Windows::Forms::Button());
			this->button_move_right = (gcnew System::Windows::Forms::Button());
			this->button_move_up = (gcnew System::Windows::Forms::Button());
			this->button_move_left = (gcnew System::Windows::Forms::Button());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->label1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->propertyGrid = (gcnew System::Windows::Forms::PropertyGrid());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->splitter1 = (gcnew System::Windows::Forms::Splitter());
			this->global_map_panel = (gcnew System::Windows::Forms::Panel());
			this->global_map_menu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->panel1->SuspendLayout();
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->Controls->Add(this->import_button);
			this->panel1->Controls->Add(this->button_move_down);
			this->panel1->Controls->Add(this->button_move_right);
			this->panel1->Controls->Add(this->button_move_up);
			this->panel1->Controls->Add(this->button_move_left);
			this->panel1->Controls->Add(this->statusStrip1);
			this->panel1->Controls->Add(this->propertyGrid);
			this->panel1->Controls->Add(this->panel2);
			this->panel1->Dock = System::Windows::Forms::DockStyle::Right;
			this->panel1->Location = System::Drawing::Point(502, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(124, 480);
			this->panel1->TabIndex = 0;
			// 
			// import_button
			// 
			this->import_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->import_button->Location = System::Drawing::Point(3, 268);
			this->import_button->Name = L"import_button";
			this->import_button->Size = System::Drawing::Size(71, 25);
			this->import_button->TabIndex = 8;
			this->import_button->Text = L"Import";
			this->import_button->UseVisualStyleBackColor = true;
			this->import_button->Click += gcnew System::EventHandler(this, &terrain_core_form::import_button_Click);
			// 
			// button_move_down
			// 
			this->button_move_down->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button_move_down->Location = System::Drawing::Point(65, 430);
			this->button_move_down->Name = L"button_move_down";
			this->button_move_down->Size = System::Drawing::Size(25, 25);
			this->button_move_down->TabIndex = 7;
			this->button_move_down->Text = L"|";
			this->button_move_down->UseVisualStyleBackColor = true;
			this->button_move_down->Click += gcnew System::EventHandler(this, &terrain_core_form::button_move_down_Click);
			// 
			// button_move_right
			// 
			this->button_move_right->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button_move_right->Location = System::Drawing::Point(96, 405);
			this->button_move_right->Name = L"button_move_right";
			this->button_move_right->Size = System::Drawing::Size(25, 25);
			this->button_move_right->TabIndex = 6;
			this->button_move_right->Text = L">";
			this->button_move_right->UseVisualStyleBackColor = true;
			this->button_move_right->Click += gcnew System::EventHandler(this, &terrain_core_form::button_move_right_Click);
			// 
			// button_move_up
			// 
			this->button_move_up->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button_move_up->Location = System::Drawing::Point(65, 382);
			this->button_move_up->Name = L"button_move_up";
			this->button_move_up->Size = System::Drawing::Size(25, 25);
			this->button_move_up->TabIndex = 5;
			this->button_move_up->Text = L"^";
			this->button_move_up->UseVisualStyleBackColor = true;
			this->button_move_up->Click += gcnew System::EventHandler(this, &terrain_core_form::button_move_up_Click);
			// 
			// button_move_left
			// 
			this->button_move_left->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->button_move_left->Location = System::Drawing::Point(37, 405);
			this->button_move_left->Name = L"button_move_left";
			this->button_move_left->Size = System::Drawing::Size(25, 25);
			this->button_move_left->TabIndex = 4;
			this->button_move_left->Text = L"<";
			this->button_move_left->UseVisualStyleBackColor = true;
			this->button_move_left->Click += gcnew System::EventHandler(this, &terrain_core_form::button_move_left_Click);
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->label1});
			this->statusStrip1->Location = System::Drawing::Point(0, 458);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(124, 22);
			this->statusStrip1->TabIndex = 3;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// label1
			// 
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(118, 15);
			this->label1->Text = L"toolStripStatusLabel1";
			// 
			// propertyGrid
			// 
			this->propertyGrid->Dock = System::Windows::Forms::DockStyle::Top;
			this->propertyGrid->Location = System::Drawing::Point(0, 46);
			this->propertyGrid->Name = L"propertyGrid";
			this->propertyGrid->Size = System::Drawing::Size(124, 216);
			this->propertyGrid->TabIndex = 2;
			// 
			// panel2
			// 
			this->panel2->Dock = System::Windows::Forms::DockStyle::Top;
			this->panel2->Location = System::Drawing::Point(0, 0);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(124, 46);
			this->panel2->TabIndex = 1;
			// 
			// splitter1
			// 
			this->splitter1->Dock = System::Windows::Forms::DockStyle::Right;
			this->splitter1->Location = System::Drawing::Point(499, 0);
			this->splitter1->Name = L"splitter1";
			this->splitter1->Size = System::Drawing::Size(3, 480);
			this->splitter1->TabIndex = 1;
			this->splitter1->TabStop = false;
			// 
			// global_map_panel
			// 
			this->global_map_panel->BackColor = System::Drawing::SystemColors::GradientActiveCaption;
			this->global_map_panel->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->global_map_panel->ContextMenuStrip = this->global_map_menu;
			this->global_map_panel->Dock = System::Windows::Forms::DockStyle::Fill;
			this->global_map_panel->Location = System::Drawing::Point(0, 0);
			this->global_map_panel->Name = L"global_map_panel";
			this->global_map_panel->Size = System::Drawing::Size(499, 480);
			this->global_map_panel->TabIndex = 2;
			this->global_map_panel->MouseLeave += gcnew System::EventHandler(this, &terrain_core_form::global_map_panel_MouseLeave);
			this->global_map_panel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &terrain_core_form::global_map_panel_Paint);
			this->global_map_panel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &terrain_core_form::global_map_panel_MouseMove);
			this->global_map_panel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &terrain_core_form::global_map_panel_MouseDown);
			this->global_map_panel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &terrain_core_form::global_map_panel_MouseUp);
			// 
			// global_map_menu
			// 
			this->global_map_menu->Name = L"global_map_menu";
			this->global_map_menu->Size = System::Drawing::Size(61, 4);
			this->global_map_menu->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &terrain_core_form::global_map_menu_Opening);
			// 
			// terrain_core_form
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->ClientSize = System::Drawing::Size(626, 480);
			this->Controls->Add(this->global_map_panel);
			this->Controls->Add(this->splitter1);
			this->Controls->Add(this->panel1);
			this->Name = L"terrain_core_form";
			this->Text = L"terrain_core_form";
			this->Deactivate += gcnew System::EventHandler(this, &terrain_core_form::terrain_core_form_Deactivate);
			this->Activated += gcnew System::EventHandler(this, &terrain_core_form::terrain_core_form_Activated);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &terrain_core_form::terrain_core_form_FormClosing);
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void global_map_panel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);
	private: System::Void terrain_core_form_Activated(System::Object^  sender, System::EventArgs^  e);
	private: System::Void terrain_core_form_Deactivate(System::Object^  sender, System::EventArgs^  e);
	private: System::Void terrain_core_form_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
	private: System::Void global_map_menu_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e);
	private: System::Void global_map_panel_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	private: System::Void button_move_left_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void button_move_up_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void button_move_right_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void button_move_down_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void global_map_panel_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	private: System::Void global_map_panel_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	private: System::Void global_map_panel_MouseLeave(System::Object^  sender, System::EventArgs^  e);

private:
		void				in_constructor();
		tool_terrain^		m_tool;
		terrain_core^		m_terrain_core;

		Point				m_first_node;
		int					m_current_view_scale;

		bool				m_mouse_rect_capture;
		terrain_node_key	m_selected_node_lt;
		terrain_node_key	m_selected_node_rb;

		void				set_terrain_node_size_impl	(terrain_node_size value);
		terrain_node_size	get_terrain_node_size_impl	();
		void				set_current_view_scale_impl	(int value);
		terrain_node_key	get_node_key_mouse			();
		void				add_node					(System::Object^ sender, System::EventArgs^);
		void				clear_node					(System::Object^ sender, System::EventArgs^);
public:
	[CategoryAttribute("terrain")]
	property System::String^			background_image;

	[CategoryAttribute("terrain")]
	property Drawing::Size				background_image_size;

	[CategoryAttribute("terrain")]
	property terrain_node_size		node_size{
		terrain_node_size			get	()							{return get_terrain_node_size_impl();}
		void						set (terrain_node_size value)	{set_terrain_node_size_impl(value);}
	}

	[CategoryAttribute("terrain")]
	property int					current_scale{
		int							get	()								{return m_current_view_scale;}
		void						set	(int value)						{set_current_view_scale_impl(value);}
	}
	private: System::Void import_button_Click(System::Object^  sender, System::EventArgs^  e);
};


} // editor namespace
} // xray namespace
