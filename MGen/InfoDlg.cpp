// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MGen.h"
#include "InfoDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "GLibrary/GMidi.h"
#include "MGenView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CInfoDlg dialog

IMPLEMENT_DYNAMIC(CInfoDlg, CDialog)

CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CInfoDlg::~CInfoDlg()
{
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
	CGMidi *pGen = mf->pGen;
	if (pGen != 0) {
		if (!pGen->mutex_output.try_lock_for(chrono::milliseconds(1000))) {
			mf->WriteLog(5, "InfoDlg mutex timed out: showing nothing");
			return TRUE;
		}
		int mx = mf->mx;
		int my = mf->my;
		int ms = ((CMGenView*)(mf->GetActiveView()))->mouse_step;
		int mv = ((CMGenView*)(mf->GetActiveView()))->mouse_voice;
		int eff;
		CString st, st2;
		// Note start and end
		int i = ms - pGen->coff[ms][mv];
		int ei = ms + pGen->noff[ms][mv] - 1;
		
		m_info.AddText("Melody:\n", RGB(0, 0, 0), CFE_BOLD);
		if (pGen->mel_id[i][mv] > -1) {
			int m1 = i;
			int m2 = i;
			// Find melody start
			for (int x = i; x >= 0; --x) {
				if (pGen->mel_id[x][mv] == pGen->mel_id[i][mv] && !pGen->pause[i][mv]) m1 = x;
			}
			// Find melody finish
			for (int x = i; x < pGen->t_generated; ++x) {
				if (pGen->mel_id[x][mv] == pGen->mel_id[i][mv]) m2 = x;
			}
			int prev_note = -1;
			int nnum = 0;
			for (int x = m1; x <= m2; ++x) {
				// When pause, reset previous note because notes can be the same left and right from the pause
				if (pGen->pause[x][mv]) prev_note = -1;
				if (pGen->note[x][mv] == prev_note || pGen->pause[x][mv]) continue;
				++nnum;
				prev_note = pGen->note[x][mv];
				if (pGen->comment[x][mv].size()) {
					// CGLib::GetNoteName(pGen->note[x][mv])
					st.Format("NOTE %d - %s:\n", nnum, NoteName[pGen->note[x][mv] % 12]);
					eff = 0;
					if (x == i) eff = CFE_BOLD;
					m_info.AddText(st, RGB(0, 0, 0), eff);
					for (int c = 0; c < pGen->comment[x][mv].size(); ++c) {
						m_info.AddText(pGen->comment[x][mv][c]+"\n", 
							RGB(pGen->ccolor[x][mv][c].GetR(), pGen->ccolor[x][mv][c].GetG()/1.5, pGen->ccolor[x][mv][c].GetB()), 0);
					}
				}
			}
			m_info.AddText("\n", RGB(0, 0, 0), 0);
			st.Format("%s\n", pGen->mel_info[pGen->mel_id[i][mv]]);
			m_info.AddText(st, RGB(0, 0, 180), 0);
			m_info.AddText("\n", RGB(0, 0, 0), 0);
		}

		st.Format("Note: %s (midi %d)\n", CGLib::GetNoteName(pGen->note[ms][mv]), pGen->note[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), CFE_BOLD);
		st.Format("Key: %s%s\n", NoteName[pGen->tonic[i][mv]], pGen->minor[i][mv]?"m":"");
		m_info.AddText(st, RGB(0, 0, 0), 0);
		if (pGen->nsr1[i][mv] > 0) {
			st.Format("Note scan range: %d - %d\n", pGen->nsr1[i][mv], pGen->nsr2[i][mv]);
			m_info.AddText(st, RGB(0, 0, 0), 0);
		}

		st2.Empty();
		for (int n = 0; n < pGen->ngraph[i][mv].size(); ++n) {
			if (pGen->ngraph[i][mv][n] > -1) {
				st.Format("%.1f ", pGen->ngraph[i][mv][n]);
				if (!st2.IsEmpty()) st2 += ", ";
				st2 += st;
			}
		}
		if (st2 != "") m_info.AddText("MeloCurve: " + st2 + "\n", RGB(0, 0, 0), 0);

		st.Format("Lengroup: %d\n", pGen->lengroup[i][mv]);
		m_info.AddText(st, RGB(0, 0, 170), 0);
		st.Format("Articulation: %s\n", ArticName[pGen->artic[i][mv]]);
		m_info.AddText(st, RGB(0, 0, 170), 0);
		st.Format("Adapt comment(start) : %s\nAdapt comment(end) : %s\n",
			pGen->adapt_comment[i][mv], pGen->adapt_comment[ei][mv]);
		m_info.AddText(st, RGB(0, 0, 170), 0);
		/*
		m_info.AddText("Note comment:\n", RGB(0, 0, 0), 0);
		for (int c = 0; c < pGen->comment[i][mv].size(); ++c) {
			m_info.AddText(pGen->comment[i][mv][c] + "\n",
				RGB(pGen->ccolor[i][mv][c].GetR(), pGen->ccolor[i][mv][c].GetG()/1.5, pGen->ccolor[i][mv][c].GetB()), 0);
		}
		*/
		st.Format("Note mark: %s\n", pGen->mark[i][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Note length: %d steps\n", pGen->len[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Note velocity: %d\n", pGen->vel[i][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Lyrics from MIDI file: %s\n", pGen->lyrics[i][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);

		st.Format("Playback note start delta: %.3f s (with delta %.3f s)\nPlayback note ending delta: %.3f s (with delta %.3f s)\n",
			pGen->dstime[i][mv] / 1000.0, pGen->stime[i] / pGen->m_pspeed / 10 + pGen->dstime[i][mv] / 1000.0,
			pGen->detime[ei][mv] / 1000.0, pGen->etime[ei] / pGen->m_pspeed / 10 + pGen->detime[ei][mv] / 1000.0);
		m_info.AddText(st, RGB(0, 0, 0), 0);

		st.Format("Step: %d (measure %d)", ms, ms / 8);
		SetWindowText(st);
		//m_info.AddText(st, RGB(0, 0, 0), CFE_BOLD);
		st.Format("Tempo: %.1f bpm (randomized from %.1f bpm)\nStep start time: %.3f s (%.2f ms long), end time %.3f s\n",
			pGen->tempo[ms], pGen->tempo_src[ms], pGen->stime[ms] / pGen->m_pspeed / 10,
			(pGen->etime[ms] - pGen->stime[ms]) * 100 / pGen->m_pspeed, pGen->etime[ms] / pGen->m_pspeed / 10);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		m_info.AddText("\n", RGB(170, 0, 0), 0);

		st.Format("Voice: %d (%s, channel %d, type %d)\n", mv,
			pGen->InstGName[pGen->instr[mv]] + "/" + pGen->InstCName[pGen->instr[mv]],
			pGen->instr_channel[pGen->instr[mv]], pGen->instr_type[pGen->instr[mv]]);
		m_info.AddText(st, RGB(0, 0, 0), CFE_BOLD);
		st.Format("Instrument library: %s\n", pGen->instr_lib[pGen->instr[mv]]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Voice show transpose: %d semitones\n", pGen->show_transpose[mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		if (pGen->midifile_loaded) {
			st.Format("From MIDI file: track %d \"%s\" local-voice %d, channel %d, delta %d ms (%.0f%% croche)\n", pGen->track_id[mv], pGen->track_name[mv], pGen->track_vid[mv], pGen->midi_ch[ms][mv], pGen->midi_delta[ms][mv], (float)(pGen->midi_delta[ms][mv]) / 300.0*(float)(pGen->tempo[ms]));
			m_info.AddText(st, RGB(0, 0, 200), 0);
		}
		m_info.AddText("\n", RGB(0, 0, 0), 0);

		st.Format("Current offset: %d steps\n", pGen->coff[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), CFE_BOLD);
		st.Format("Previous offset : %d steps\nNext offset : %d steps\n",
			pGen->poff[ms][mv], pGen->noff[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Dynamics: %d\n", pGen->dyn[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Vibrato intensity: %d\nVibrato speed: %d\n", pGen->vib[ms][mv], pGen->vibf[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		st.Format("Pause indicator: %d\n", pGen->pause[ms][mv]);
		m_info.AddText(st, RGB(0, 0, 0), 0);
		//AddText("Some text text text\r\n", RGB(250, 100, 0), CFE_BOLD | CFE_ITALIC | CFE_STRIKEOUT | CFE_UNDERLINE);
		m_info.SetSel(0, 0);
		pGen->mutex_output.unlock();

		//SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	CRect rect;
	CRect dialogRect;
	CRect deskRect;
	CWnd::GetDesktopWindow()->GetWindowRect(deskRect);
	GetWindowRect(dialogRect);
	int y = min(50, (deskRect.Height() - dialogRect.Height()) / 2);
	if (mf->mx > deskRect.Width() / 2) {
		rect.left = 0;
		rect.top = y;
		rect.right = dialogRect.Width();
		rect.bottom = y + dialogRect.Height();
	}
	else {
		rect.left = deskRect.Width() - dialogRect.Width();
		rect.top = y;
		rect.right = deskRect.Width();
		rect.bottom = y + dialogRect.Height();
	}
	MoveWindow(rect);
	ShowWindow(SW_SHOW);

	return TRUE;  // return TRUE unless you set the focus to a control
								// EXCEPTION: OCX Property Pages should return FALSE
}
