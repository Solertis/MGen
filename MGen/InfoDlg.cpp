// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MGen.h"
#include "InfoDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "GenTemplate.h"
#include "MGenView.h"

// CInfoDlg dialog

IMPLEMENT_DYNAMIC(CInfoDlg, CDialog)

CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CInfoDlg::~CInfoDlg()
{
}

void CInfoDlg::AddText(CString str, COLORREF color, DWORD dwEffects)
{
	// CFE_BOLD | CFE_ITALIC | CFE_STRIKEOUT | CFE_UNDERLINE 
	// CFE_DISABLED CFE_AUTOCOLOR
	int nOldLines = 0, nNewLines = 0, nScroll = 0;
	long nInsertionPoint = 0;
	CHARFORMAT cf;

	// Save number of lines before insertion of new text
	nOldLines = m_info.GetLineCount();

	// Initialize character format structure
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	cf.dwEffects = dwEffects;	// To disable CFE_AUTOCOLOR
	cf.crTextColor = color;

	// Set insertion point to end of text
	nInsertionPoint = m_info.GetWindowTextLength();
	/*if (nInsertionPoint > 800)
	{
	// Delete first half of text to avoid running into the 64k limit
	m_info.SetSel(0, nInsertionPoint / 2);
	m_info.ReplaceSel("");
	UpdateData(FALSE);
	}*/
	nInsertionPoint = -1;
	m_info.SetSel(nInsertionPoint, -1);

	//  Set the character format
	m_info.SetSelectionCharFormat(cf);

	// Replace selection. Because we have nothing selected, this will simply insert
	// the string at the current caret position.
	m_info.ReplaceSel(str);

	// Get new line count
	//nNewLines = m_info.GetLineCount();

	// Scroll by the number of lines just inserted
	//nScroll = nNewLines - nOldLines;
	//m_info.LineScroll(nScroll);
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_info);
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
END_MESSAGE_MAP()


// CInfoDlg message handlers


BOOL CInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CMainFrame *mf = (CMainFrame *)AfxGetMainWnd();
	CGenTemplate *pGen = mf->pGen;
	if (pGen != 0) {
		if (!pGen->mutex_output.try_lock_for(chrono::milliseconds(1000))) {
			mf->WriteLog(1, "InfoDlg mutex timed out: showing nothing");
			return TRUE;
		}
		int ms = ((CMGenView*)(mf->GetActiveView()))->mouse_step;
		int mv = ((CMGenView*)(mf->GetActiveView()))->mouse_voice;
		//SetWindowText("Dialog: some text");
		CString st;
		st.Format("Step: %d (measure %d)\nVoice: %d\nNote: %s%d (midi %d)\nNote length: %d\n", ms, ms/8, mv, 
			NoteName[pGen->note[ms][mv] % 12], pGen->note[ms][mv] / 12 - 1, pGen->note[ms][mv], pGen->len[ms][mv]);
		AddText(st, RGB(0, 0, 0), CFE_BOLD);
		st.Format("Current offset: %d steps\nPrevious offset: %d steps\nNext offset: %d steps\nAttack: %d\n", pGen->coff[ms][mv], pGen->poff[ms][mv], pGen->noff[ms][mv], pGen->att[ms][mv]);
		AddText(st, RGB(0, 0, 0), 0);
		st.Format("Tempo: %d bpm\nStep start time: %.4f s (%.2f ms long)\nPause indicator: %d", pGen->tempo[ms], pGen->stime[ms]/1000, 
			pGen->ntime[ms] - pGen->stime[ms], pGen->pause[ms][mv]);
		AddText(st, RGB(0, 0, 0), 0);
		//AddText("Some text text text\r\n", RGB(250, 100, 0), CFE_BOLD | CFE_ITALIC | CFE_STRIKEOUT | CFE_UNDERLINE);
		m_info.SetSel(0, 0);
		pGen->mutex_output.unlock();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
								// EXCEPTION: OCX Property Pages should return FALSE
}