////////////////////////////////////////////////////////////////////////////
//	Created		: 09.02.2010
//	Author		: Evgeniy Obertyukh
//	Copyright (C) GSC Game World - 2010
////////////////////////////////////////////////////////////////////////////

#ifndef RESOURCE_EDITOR_H_INCLUDED
#define RESOURCE_EDITOR_H_INCLUDED

using namespace System;
using namespace System::IO;
using namespace System::Windows;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace xray {
	namespace editor {

		class editor_world;
		interface class resource_document_factory;
		ref class resource_editor_base;
		/// <summary>
		/// Summary for resource_editor
		///
		/// WARNING: If you change the name of this class, you will need to change the
		///          'Resource File Name' property for the managed resource compiler tool
		///          associated with all .resx files this class depends on.  Otherwise,
		///          the designers will not be able to interact properly with localized
		///          resources associated with this form.
		/// </summary>
		public ref class resource_editor : public System::Windows::Forms::Form
		{

		#pragma region | Initialize |

		public:

			resource_editor(editor_world& world, resource_document_factory^ base_resource, controls::tree_view_source^ resources_source)
				:m_world(&world)
			{
				InitializeComponent();
				in_constructor(base_resource, resources_source);
			}


		protected:
			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~resource_editor()
			{
				if (components)
				{
					delete components;
				}
			}

		private:
			void in_constructor(resource_document_factory^ base_resource, controls::tree_view_source^ resources_source);
		private: System::Windows::Forms::Panel^  m_controls_panel;
		private: System::Windows::Forms::Button^  m_cancel_button;

		private: System::Windows::Forms::Button^  m_ok_button;


			/// <summary>
			/// Required designer variable.
			/// </summary>
			System::ComponentModel::Container ^components;

		#pragma region Windows Form Designer generated code

			/// <summary>
			/// Required method for Designer support - do not modify
			/// the contents of this method with the code editor.
			/// </summary>
		protected:
			void InitializeComponent(void)
			{
				System::Windows::Forms::GroupBox^  h_line;
				this->m_controls_panel = (gcnew System::Windows::Forms::Panel());
				this->m_cancel_button = (gcnew System::Windows::Forms::Button());
				this->m_ok_button = (gcnew System::Windows::Forms::Button());
				h_line = (gcnew System::Windows::Forms::GroupBox());
				this->m_controls_panel->SuspendLayout();
				this->SuspendLayout();
				// 
				// h_line
				// 
				h_line->Dock = System::Windows::Forms::DockStyle::Bottom;
				h_line->Location = System::Drawing::Point(0, 482);
				h_line->Name = L"h_line";
				h_line->Size = System::Drawing::Size(653, 3);
				h_line->TabIndex = 1;
				h_line->TabStop = false;
				// 
				// m_controls_panel
				// 
				this->m_controls_panel->Controls->Add(this->m_cancel_button);
				this->m_controls_panel->Controls->Add(this->m_ok_button);
				this->m_controls_panel->Dock = System::Windows::Forms::DockStyle::Bottom;
				this->m_controls_panel->Location = System::Drawing::Point(0, 485);
				this->m_controls_panel->Name = L"m_controls_panel";
				this->m_controls_panel->Size = System::Drawing::Size(653, 54);
				this->m_controls_panel->TabIndex = 0;
				// 
				// m_cancel_button
				// 
				this->m_cancel_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
				this->m_cancel_button->Location = System::Drawing::Point(566, 16);
				this->m_cancel_button->Name = L"m_cancel_button";
				this->m_cancel_button->Size = System::Drawing::Size(75, 23);
				this->m_cancel_button->TabIndex = 0;
				this->m_cancel_button->Text = L"Cancel";
				this->m_cancel_button->UseVisualStyleBackColor = true;
				this->m_cancel_button->Click += gcnew System::EventHandler(this, &resource_editor::m_cancel_button_Click);
				// 
				// m_ok_button
				// 
				this->m_ok_button->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
				this->m_ok_button->Location = System::Drawing::Point(485, 16);
				this->m_ok_button->Name = L"m_ok_button";
				this->m_ok_button->Size = System::Drawing::Size(75, 23);
				this->m_ok_button->TabIndex = 0;
				this->m_ok_button->Text = L"Apply";
				this->m_ok_button->UseVisualStyleBackColor = true;
				this->m_ok_button->Click += gcnew System::EventHandler(this, &resource_editor::m_ok_button_Click);
				// 
				// resource_editor
				// 
				this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->ClientSize = System::Drawing::Size(653, 539);
				this->Controls->Add(h_line);
				this->Controls->Add(this->m_controls_panel);
				this->Name = L"resource_editor";
				this->Text = L"Resource Editor";
				this->Load += gcnew System::EventHandler(this, &resource_editor::resource_editor_Load);
				this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &resource_editor::resource_editor_FormClosing);
				this->m_controls_panel->ResumeLayout(false);
				this->ResumeLayout(false);

			}

		#pragma endregion

		#pragma endregion

		#pragma region |   Fields   |

		private:
			resource_editor_base^			m_multidocument;
			controls::tree_view_source^		m_source;

			Boolean							m_is_closed;

			editor_world*					m_world;

		#pragma endregion

		#pragma region | Properties |

		public:
			property String^ name
			{
				String^		get();
				void		set(String^ value);
			}
			property String^ view_panel_caption
			{
				String^		get();
				void		set(String^ value);
			}
			property String^ properties_panel_caption
			{
				String^		get();
				void		set(String^ value);
			}

		#pragma endregion

		#pragma region |   Methods  |

		private:
			void						m_ok_button_Click				(Object^ sender, EventArgs^ e);
			void						m_cancel_button_Click			(Object^ sender, EventArgs^ e);
			void						resource_editor_Load			(Object^ sender, EventArgs^ e);
			RegistryKey^				editor_registry_key				();

		public:
			void						resource_editor_FormClosing		(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
			void						manual_close					();

		#pragma endregion

		}; // class resource_editor
	}//namespace editor
}//namespace xray

#endif // #ifndef RESOURCE_EDITOR_H_INCLUDED