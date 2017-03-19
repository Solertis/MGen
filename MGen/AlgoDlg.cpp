// AlgoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MGen.h"
#include "AlgoDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"

// CAlgoDlg dialog

IMPLEMENT_DYNAMIC(CAlgoDlg, CDialog)

CAlgoDlg::CAlgoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_ALGO, pParent)
{

}

CAlgoDlg::~CAlgoDlg()
{
}

void CAlgoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_ALGO, m_tree);
	DDX_Control(pDX, IDC_EDIT_COMMENT, m_comment);
}


BEGIN_MESSAGE_MAP(CAlgoDlg, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_ALGO, &CAlgoDlg::OnTvnSelchangedTreeAlgo)
END_MESSAGE_MAP()


// CAlgoDlg message handlers


BOOL CAlgoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CMainFrame* mf = (CMainFrame*)theApp.m_pMainWnd;
	CString gname;
	for (int i = 0; i < mf->AlgGCount; i++) {
		gname = mf->AlgGroups[i];
		HTREEITEM gti = m_tree.InsertItem(gname, TVI_ROOT, TVI_LAST);
		m_tree.SetItemState(gti, TVIS_BOLD, TVIS_BOLD);
		// Font
		LOGFONT logfont;
		CFont *pFont = GetFont();
		pFont->GetLogFont(&logfont);
		logfont.lfItalic = TRUE;
		//logfont.lfUnderline = TRUE;
		for (int x = 0; x < mf->AlgCount; x++) {
			if (mf->AlgGroup[x] == gname) {
				HTREEITEM gti2 = m_tree.InsertItem(mf->AlgName[x], gti, TVI_LAST);
				m_tree.SetItemData(gti2, x);
				m_tree.Expand(gti2, TVM_EXPAND);
				HTREEITEM gti3 = m_tree.InsertItem("Config 1", gti2, TVI_LAST);
				gti3 = m_tree.InsertItem("Config 2", gti2, TVI_LAST);
				m_tree.SetItemColor(gti3, RGB(0, 200, 0));
				//m_tree.SetItemFont(gti3, logfont);
				//m_tree.SetItemState(gti3, TVIS_BOLD, TVIS_BOLD);
			}
		}
		m_tree.Expand(gti, TVM_EXPAND);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
								// EXCEPTION: OCX Property Pages should return FALSE
}


void CAlgoDlg::OnTvnSelchangedTreeAlgo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	int i = m_tree.GetItemData(m_tree.GetSelectedItem());
	CMainFrame* mf = (CMainFrame*)theApp.m_pMainWnd;
	m_comment.SetWindowTextA(mf->AlgComment[i]);
	*pResult = 0;
}