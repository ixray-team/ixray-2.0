#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace xray {
namespace editor {
	
	ref class level_editor;
	/// <summary>
	/// Summary for project_tab
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class project_tab : public WeifenLuo::WinFormsUI::Docking::DockContent
	{
	private:
			void	in_constructor		( );
	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialog;
	private: System::Windows::Forms::ImageList^  imageList1;
	private: System::Windows::Forms::ContextMenuStrip^  global_view_menu;

	private: System::Windows::Forms::ToolStripMenuItem^  addToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  newFilterToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeToolStripMenuItem;








			 level_editor^		m_level_editor;
	public:
		project_tab(level_editor^ le):m_level_editor(le)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			in_constructor	( );
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~project_tab()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::StatusStrip^  statusStrip;









	public: xray::editor::controls::tree_view^  treeView;
	private: 
	public: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabel1;


	private: System::Windows::Forms::ToolStripProgressBar^  toolStripProgressBar1;
	private: System::ComponentModel::IContainer^  components;







	protected: 

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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(project_tab::typeid));
			this->statusStrip = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripProgressBar1 = (gcnew System::Windows::Forms::ToolStripProgressBar());
			this->treeView = (gcnew xray::editor::controls::tree_view());
			this->global_view_menu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->addToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->newFilterToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->imageList1 = (gcnew System::Windows::Forms::ImageList(this->components));
			this->folderBrowserDialog = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->statusStrip->SuspendLayout();
			this->global_view_menu->SuspendLayout();
			this->SuspendLayout();
			// 
			// statusStrip
			// 
			this->statusStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->toolStripStatusLabel1, 
				this->toolStripProgressBar1});
			this->statusStrip->Location = System::Drawing::Point(0, 358);
			this->statusStrip->Name = L"statusStrip";
			this->statusStrip->Size = System::Drawing::Size(295, 22);
			this->statusStrip->TabIndex = 1;
			// 
			// toolStripStatusLabel1
			// 
			this->toolStripStatusLabel1->Name = L"toolStripStatusLabel1";
			this->toolStripStatusLabel1->Size = System::Drawing::Size(59, 17);
			this->toolStripStatusLabel1->Text = L"Objects: 0";
			// 
			// toolStripProgressBar1
			// 
			this->toolStripProgressBar1->Name = L"toolStripProgressBar1";
			this->toolStripProgressBar1->Size = System::Drawing::Size(100, 16);
			this->toolStripProgressBar1->Step = 1;
			this->toolStripProgressBar1->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->toolStripProgressBar1->Value = 50;
			this->toolStripProgressBar1->Visible = false;
			// 
			// treeView
			// 
			this->treeView->auto_expand_on_filter = false;
			this->treeView->BackColor = System::Drawing::SystemColors::Window;
			this->treeView->ContextMenuStrip = this->global_view_menu;
			this->treeView->Dock = System::Windows::Forms::DockStyle::Fill;
			this->treeView->filter_visible = true;
			this->treeView->ImageIndex = 0;
			this->treeView->ImageList = this->imageList1;
			this->treeView->Indent = 27;
			this->treeView->is_multiselect = false;
			this->treeView->is_selectable_groups = true;
			this->treeView->ItemHeight = 20;
			this->treeView->LabelEdit = true;
			this->treeView->Location = System::Drawing::Point(0, 0);
			this->treeView->Name = L"treeView";
			this->treeView->PathSeparator = L"/";
			this->treeView->SelectedImageIndex = 0;
			this->treeView->Size = System::Drawing::Size(295, 358);
			this->treeView->source = nullptr;
			this->treeView->TabIndex = 3;
			this->treeView->AfterLabelEdit += gcnew System::Windows::Forms::NodeLabelEditEventHandler(this, &project_tab::treeView_AfterLabelEdit);
			this->treeView->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &project_tab::treeView_MouseDown);
			this->treeView->BeforeLabelEdit += gcnew System::Windows::Forms::NodeLabelEditEventHandler(this, &project_tab::treeView_BeforeLabelEdit);
			// 
			// global_view_menu
			// 
			this->global_view_menu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->addToolStripMenuItem, 
				this->removeToolStripMenuItem});
			this->global_view_menu->Name = L"treeViewContextMenu";
			this->global_view_menu->Size = System::Drawing::Size(118, 48);
			this->global_view_menu->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &project_tab::global_view_menu_Opening);
			// 
			// addToolStripMenuItem
			// 
			this->addToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->newFilterToolStripMenuItem});
			this->addToolStripMenuItem->Name = L"addToolStripMenuItem";
			this->addToolStripMenuItem->Size = System::Drawing::Size(117, 22);
			this->addToolStripMenuItem->Text = L"Add";
			// 
			// newFilterToolStripMenuItem
			// 
			this->newFilterToolStripMenuItem->Name = L"newFilterToolStripMenuItem";
			this->newFilterToolStripMenuItem->Size = System::Drawing::Size(152, 22);
			this->newFilterToolStripMenuItem->Text = L"New Filter";
			this->newFilterToolStripMenuItem->Click += gcnew System::EventHandler(this, &project_tab::newFilterToolStripMenuItem_Click);
			// 
			// removeToolStripMenuItem
			// 
			this->removeToolStripMenuItem->Name = L"removeToolStripMenuItem";
			this->removeToolStripMenuItem->Size = System::Drawing::Size(117, 22);
			this->removeToolStripMenuItem->Text = L"Remove";
			this->removeToolStripMenuItem->Click += gcnew System::EventHandler(this, &project_tab::removeToolStripMenuItem_Click);
			// 
			// imageList1
			// 
			this->imageList1->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^  >(resources->GetObject(L"imageList1.ImageStream")));
			this->imageList1->TransparentColor = System::Drawing::Color::Transparent;
			this->imageList1->Images->SetKeyName(0, L"group");
			this->imageList1->Images->SetKeyName(1, L"mesh");
			this->imageList1->Images->SetKeyName(2, L"point_light");
			this->imageList1->Images->SetKeyName(3, L"spot_light");
			this->imageList1->Images->SetKeyName(4, L"volume_light");
			this->imageList1->Images->SetKeyName(5, L"ambient_light");
			this->imageList1->Images->SetKeyName(6, L"area_light");
			this->imageList1->Images->SetKeyName(7, L"camera");
			this->imageList1->Images->SetKeyName(8, L"curve");
			this->imageList1->Images->SetKeyName(9, L"default_object_set");
			this->imageList1->Images->SetKeyName(10, L"directional_light");
			this->imageList1->Images->SetKeyName(11, L"joint");
			// 
			// folderBrowserDialog
			// 
			this->folderBrowserDialog->ShowNewFolderButton = false;
			// 
			// project_tab
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(295, 380);
			this->Controls->Add(this->treeView);
			this->Controls->Add(this->statusStrip);
			this->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(204)));
			this->HideOnClose = true;
			this->Name = L"project_tab";
			this->TabText = L"project";
			this->Text = L"project";
			this->statusStrip->ResumeLayout(false);
			this->statusStrip->PerformLayout();
			this->global_view_menu->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void newFilterToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void removeToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
	private: System::Void global_view_menu_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e);
	private: System::Void treeView_BeforeLabelEdit(System::Object^  sender, System::Windows::Forms::NodeLabelEditEventArgs^  e);
	private: System::Void treeView_AfterLabelEdit(System::Object^  sender, System::Windows::Forms::NodeLabelEditEventArgs^  e);
	private: System::Void treeView_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
};
}
}
