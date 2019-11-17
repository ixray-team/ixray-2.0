#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "tool_tab.h"

namespace xray {
namespace editor {
ref class tool_solid_visual;

	/// <summary>
	/// Summary for solid_visual_tool_tab
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class solid_visual_tool_tab :	public System::Windows::Forms::UserControl, 
												public tool_tab
	{
	public:
		solid_visual_tool_tab(tool_solid_visual^ tool):m_tool(tool)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	public: 
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	public: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;
	private: 



	public: xray::editor::controls::tree_view^  treeView;
	private: 




	private: System::Windows::Forms::ImageList^  imageList1;

	private: 

	public: 
		tool_solid_visual^		m_tool;
	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~solid_visual_tool_tab()
		{
			if (components)
			{
				delete components;
			}
		}
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
			System::Windows::Forms::TreeNode^  treeNode1 = (gcnew System::Windows::Forms::TreeNode(L""));
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(solid_visual_tool_tab::typeid));
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->treeView = (gcnew xray::editor::controls::tree_view());
			this->imageList1 = (gcnew System::Windows::Forms::ImageList(this->components));
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripStatusLabel1});
			this->statusStrip1->Location = System::Drawing::Point(0, 363);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(242, 22);
			this->statusStrip1->TabIndex = 1;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			this->toolStripStatusLabel1->Size = System::Drawing::Size(34, 17);
			this->toolStripStatusLabel1->Text = L"total:";
			// 
			// treeView
			// 
			this->treeView->auto_expand_on_filter = false;
			this->treeView->Dock = System::Windows::Forms::DockStyle::Fill;
			this->treeView->filter_visible = false;
			this->treeView->HideSelection = false;
			this->treeView->ImageIndex = 0;
			this->treeView->ImageList = this->imageList1;
			this->treeView->Indent = 27;
			this->treeView->is_multiselect = false;
			this->treeView->ItemHeight = 20;
			this->treeView->Location = System::Drawing::Point(0, 0);
			this->treeView->Name = L"treeView";
			this->treeView->PathSeparator = L"/";
			treeNode1->Name = L"";
			treeNode1->Text = L"";
			this->treeView->root = treeNode1;
			this->treeView->SelectedImageIndex = 0;
			this->treeView->Size = System::Drawing::Size(242, 363);
			this->treeView->source = nullptr;
			this->treeView->TabIndex = 4;
			this->treeView->AfterSelect += gcnew System::Windows::Forms::TreeViewEventHandler(this, &solid_visual_tool_tab::treeView_AfterSelect);
			// 
			// imageList1
			// 
			this->imageList1->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"imageList1.ImageStream")));
			this->imageList1->TransparentColor = System::Drawing::Color::Transparent;
			this->imageList1->Images->SetKeyName(0, L"group_closed");
			this->imageList1->Images->SetKeyName(1, L"group_open");
			this->imageList1->Images->SetKeyName(2, L"mesh");
			// 
			// solid_visual_tool_tab
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->treeView);
			this->Controls->Add(this->statusStrip1);
			this->Name = L"solid_visual_tool_tab";
			this->Size = System::Drawing::Size(242, 385);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void treeView_AfterSelect(System::Object^  sender, System::Windows::Forms::TreeViewEventArgs^  e);
	public:
	virtual System::Windows::Forms::UserControl^	frame () {return this;}
};

} // namespace editor
} // namespace xray