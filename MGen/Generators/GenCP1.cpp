#include "../stdafx.h"
#include "GenCP1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGenCP1::CGenCP1() {
	av_cnt = 2;
	v_cnt = 2;
	track_id[0] = 2;
	track_id[1] = 1;
}

CGenCP1::~CGenCP1() {
}

void CGenCP1::LoadConfigLine(CString * sN, CString * sV, int idata, float fdata) {
	CheckVar(sN, sV, "cantus_high", &cantus_high);
	CheckVar(sN, sV, "cantus_id", &cantus_id2);
	CheckVar(sN, sV, "slurs_window", &slurs_window);
	LoadRange(sN, sV, "between", &min_between, &max_between);
	CheckVar(sN, sV, "sum_interval", &sum_interval);
	CheckVar(sN, sV, "burst_between", &burst_between);
	CheckVar(sN, sV, "burst_steps", &burst_steps);

	CGenCA1::LoadConfigLine(sN, sV, idata, fdata);
}

void CGenCP1::InitCP() {
	// Set rule colors
	for (int i = 0; i < MAX_SEVERITY; ++i) {
		flag_color[i] = Color(0, 255.0 / MAX_SEVERITY*i, 255 - 255.0 / MAX_SEVERITY*i, 0);
	}
	// Check that method is selected
	if (method == mUndefined) WriteLog(1, "Error: method not specified in algorithm configuration file");
	ac.resize(av_cnt);
	acc.resize(av_cnt);
	acc_old.resize(av_cnt);
	apc.resize(av_cnt);
	apcc.resize(av_cnt);
	asmooth.resize(av_cnt);
	aleap.resize(av_cnt);
	anflags.resize(av_cnt);
	anflagsc.resize(av_cnt);
}

void CGenCP1::MakeNewCP() {
	// Set pitch limits
	if (cantus_high) {
		for (int i = 0; i < c_len; ++i) {
			max_c[i] = ac[cfv][i] - min_between;
			min_c[i] = ac[cfv][i] - max_between;
		}
	}
	else {
		for (int i = 0; i < c_len; ++i) {
			min_c[i] = ac[cfv][i] + min_between;
			max_c[i] = ac[cfv][i] + max_between;
		}
	}
	// Convert limits to chromatic
	for (int i = 0; i < c_len; ++i) {
		min_cc[i] = C_CC(min_c[i], tonic_cur, minor_cur);
		max_cc[i] = C_CC(max_c[i], tonic_cur, minor_cur);
	}
	if (random_seed) {
		RandCantus(ac[cpv], acc[cpv], 0, c_len);
	}
	else {
		FillCantus(acc[cpv], 0, c_len, min_cc);
	}
}

void CGenCP1::SingleCPInit() {
	// Copy cantus
	acc = scpoint;
	// Get diatonic steps from chromatic
	for (int v = 0; v < acc.size(); ++v) {
		for (int i = 0; i < c_len; ++i) {
			ac[v][i] = CC_C(acc[v][i], tonic_cur, minor_cur);
		}
	}
	// Save value for future use;
	acc_old = acc;
	/*
	if (!swa_inrange) {
		GetRealRange(ac[cfv], acc[cfv]);
		ApplySourceRange();
	}
	// Set pitch limits
	// If too wide range is not accepted, correct range to increase scan performance
	if (!accept[37]) {
	for (int i = 0; i < c_len; ++i) {
	min_c[i] = max(minc, c[i] - correct_range);
	max_c[i] = min(maxc, c[i] + correct_range);
	}
	}
	else {
	*/
	if (cantus_high) {
		for (int i = 0; i < c_len; ++i) {
			max_c[i] = min(ac[cfv][i], ac[cpv][i] + correct_range);
			min_c[i] = ac[cpv][i] - correct_range;
		}
	}
	else {
		for (int i = 0; i < c_len; ++i) {
			min_c[i] = max(ac[cfv][i], ac[cpv][i] - correct_range);
			max_c[i] = ac[cpv][i] + correct_range;
		}
	}
	// Convert limits to chromatic
	for (int i = 0; i < c_len; ++i) {
		min_cc[i] = C_CC(min_c[i], tonic_cur, minor_cur);
		max_cc[i] = C_CC(max_c[i], tonic_cur, minor_cur);
	}
	sp1 = 0;
	sp2 = c_len;
	ep1 = max(0, sp1 - 1);
	ep2 = c_len;
	// Clear flags
	++accepted3;
	fill(flags.begin(), flags.end(), 0);
	flags[0] = 1;
	for (int i = 0; i < ep2; ++i) {
		anflagsc[cpv][i] = 0;
	}
	// Matrix scan
	if (task != tEval) {
		// Exit if no violations
		if (!smatrixc) return;
		// Create map
		smap.resize(smatrixc);
		int map_id = 0;
		for (int i = 0; i < c_len; ++i) if (smatrix[i]) {
			smap[map_id] = i;
			++map_id;
		}
		// Shuffled smap
		//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		//::shuffle(smap.begin(), smap.end(), default_random_engine(seed));
		sp1 = 0;
		sp2 = sp1 + s_len; // End of search window
		if (sp2 > smatrixc) sp2 = smatrixc;
		// Record window
		wid = 0;
		wpos1[wid] = sp1;
		wpos2[wid] = sp2;
		// Add last note if this is last window
		// End of evaluation window
		if (method == mScan) {
			ep2 = GetMaxSmap() + 1;
			if (sp2 == smatrixc) ep2 = c_len;
			// Clear scan steps
			FillCantusMap(acc[cpv], smap, 0, smatrixc, min_cc);
			// Can skip flags - full scan must remove all flags
		}
		// For sliding windows algorithm evaluate whole melody
		if (method == mSWA) {
			ep2 = c_len;
			// Cannot skip flags - need them for penalty if cannot remove all flags
			skip_flags = 0;
			// Clear scan steps of current window
			FillCantusMap(acc[cpv], smap, sp1, sp2, min_cc);
		}
		// Minimum element
		ep1 = max(0, GetMinSmap() - 1);
		// Minimal position in array to cycle
		pp = sp2 - 1;
		p = smap[pp];
	}
	else {
		// For single cantus scan - cannot skip flags - must show all
		skip_flags = 0;
	}
}

void CGenCP1::MultiCPInit() {
	MakeNewCP();
	sp1 = 0; // Start of search window
	sp2 = sp1 + s_len; // End of search window
	if (sp2 > c_len - 1) sp2 = c_len - 1;
	// Record window
	wid = 0;
	wpos1[wid] = sp1;
	wpos2[wid] = sp2;
	// Add last note if this is last window
	ep1 = max(0, sp1 - 1);
	ep2 = sp2; // End of evaluation window
	if (ep2 == c_len - 1) ep2 = c_len;
	p = sp2 - 1; // Minimal position in array to cycle
}

void CGenCP1::ScanCPInit() {
	// Get cantus size
	if (task != tGen) c_len = scpoint[0].size();
	// Resize global vectors
	for (int i = 0; i < av_cnt; ++i) {
		ac[i].resize(c_len); // cantus (diatonic)
		acc[i].resize(c_len); // cantus (chromatic)
		anflags[i].resize(c_len, vector<int>(max_flags)); // Flags for each note
		anflagsc[i].resize(c_len); // number of flags for each note
		acc_old[i].resize(c_len); // Cantus diatonic saved for SWA
		apc[i].resize(c_len);
		apcc[i].resize(c_len);
		aleap[i].resize(c_len);
		asmooth[i].resize(c_len);
	}
	hm.resize(c_len);
	hm2.resize(c_len);
	for (int x = 0; x < c_len; ++x) {
		hm[x].resize(3);
		hm2[x].resize(3);
	}
	fpenalty.resize(max_flags);
	wpos1.resize(c_len / s_len + 1);
	wpos2.resize(c_len / s_len + 1);
	ivl.resize(c_len);
	ivlc.resize(c_len);
	civl.resize(c_len);
	civlc.resize(c_len);
	tivl.resize(c_len);
	direct.resize(c_len);
	min_c.resize(c_len);
	max_c.resize(c_len);
	min_cc.resize(c_len);
	max_cc.resize(c_len);
	accepted4.resize(MAX_WIND); // number of accepted canti per window
	accepted5.resize(MAX_WIND); // number of canti with neede flags per window
	flags.resize(max_flags); // Flags for whole cantus
	fstat.resize(max_flags); // number of canti with each flag
	fcor.resize(max_flags, vector<long long>(max_flags)); // Flags correlation matrix
	seed_cycle = 0; // Number of cycles in case of random_seed
	reseed_count = 0;
	nstat.resize(MAX_NOTE);
	nstat2.resize(MAX_NOTE);
	nstat3.resize(MAX_NOTE);
	cycle = 0;
	wscans.resize(MAX_WIND); // number of full scans per window
	wcount = 1; // Number of windows created
	accepted = 0;
	accepted2 = 0;
	accepted3 = 0;
	// Can we skip flags?
	skip_flags = !calculate_blocking && !calculate_correlation && !calculate_stat;
	// Initialize fblock if calculation is needed
	if (calculate_blocking) {
		fblock = vector<vector<vector<long>>>(MAX_WIND, vector<vector<long>>(max_flags, vector<long>(max_flags)));
	}
	// Init best rejected results
	if (best_rejected) {
		rcycle = 0;
		accept_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		rpenalty_min = MAX_PENALTY;
	}
}

int CGenCP1::SendCP() {
	CString st, info, rpst;
	int pos;
	int v;
	Sleep(sleep_ms);
	for (int av = 0; av < av_cnt; ++av) {
		if (cpv) {
			v = svoice + av;
		}
		else {
			v = svoice + 1 - av;
		}
		// Copy cantus to output
		pos = step;
		if (step + real_len >= t_allocated) ResizeVectors(t_allocated * 2);
		for (int x = 0; x < c_len; ++x) {
			for (int i = 0; i < cc_len[x]; ++i) {
				int current_severity = -1;
				if (av == cpv) {
					// Set color
					color[pos + i][v] = Color(0, 100, 100, 100);
				}
				// Set nflag color
				note[pos + i][v] = acc[av][x];
				tonic[pos + i][v] = tonic_cur;
				minor[pos + i][v] = minor_cur;
				if (anflagsc[av][x] > 0) for (int f = 0; f < anflagsc[av][x]; ++f) {
					int fl = anflags[av][x][f];
					if (!i) {
						st = "+ ";
						if (!accept[fl]) st = "- ";
						comment[pos][v] += "\n" + st + FlagName[fl];
						if (show_severity) {
							st.Format(" [%d]", severity[fl]);
							comment[pos][v] += st;
						}
						if (FlagGComment[fl] != "") comment[pos][v] += ". " + FlagGComment[fl];
						if (FlagComment[fl] != "") comment[pos][v] += ". " + FlagComment[fl];
						comment[pos][v] += ". ";
					}
					// Do not show colors for base voice
					if (av == cpv) {
						// Set note color if this is maximum flag severity
						if (severity[fl] > current_severity) {
							current_severity = severity[fl];
							color[pos + i][v] = flag_color[severity[fl]];
						}
					}
				}
				// Add scan range
				if (!i) {
					st.Format("\nNote scan range: %d - %d", min_cc[x], max_cc[x]);
					comment[pos][v] += st;
				}
				len[pos + i][v] = cc_len[x];
				pause[pos + i][v] = 0;
				coff[pos + i][v] = i;
				if (x < real_len / 2)	dyn[pos + i][v] = 60 + 40 * (pos + i - step) / real_len + 20 * rand2() / RAND_MAX;
				else dyn[pos + i][v] = 60 + 40 * (real_len - pos - i + step) / real_len + 20 * rand2() / RAND_MAX;
				// Assign source tempo if exists
				if (cc_tempo[x]) {
					tempo[pos + i] = cc_tempo[x];
				}
				// Generate tempo if no source
				else {
					if (pos + i == 0) {
						tempo[pos + i] = min_tempo + (float)(max_tempo - min_tempo) * (float)rand2() / (float)RAND_MAX;
					}
					else {
						tempo[pos + i] = tempo[pos + i - 1] + randbw(-1, 1);
						if (tempo[pos + i] > max_tempo) tempo[pos + i] = 2 * max_tempo - tempo[pos + i];
						if (tempo[pos + i] < min_tempo) tempo[pos + i] = 2 * min_tempo - tempo[pos + i];
					}
				}
			}
			pos += cc_len[x];
		}
		// Create pause
		note[pos][v] = 0;
		len[pos][v] = 1;
		pause[pos][v] = 1;
		dyn[pos][v] = 0;
		tempo[pos] = tempo[pos - 1];
		coff[pos][v] = 0;
	}
	step = pos;
	++step;
	// Count additional variables
	CountOff(step - real_len - 1, step - 1);
	CountTime(step - real_len - 1, step - 1);
	UpdateNoteMinMax(step - real_len - 1, step - 1);
	UpdateTempoMinMax(step - real_len - 1, step - 1);
	++cantus_sent;
	if (task == tGen) {
		if (!shuffle) {
			Adapt(step - real_len - 1, step - 1);
		}
		// If  window-scan
		st.Format("#%d\nHarmonic difficulty: %.0f", cantus_sent, hdif);
		AddMelody(step - real_len - 1, step - 1, svoice + 1, st);
	}
	else if (task == tEval) {
		if (m_algo_id == 101) {
			// If RSWA
			st.Format("#%d\nHarmonic difficulty: %.0f", cantus_sent, hdif);
		}
		else {
			for (int x = 0; x < max_flags; ++x) {
				if (!accept[x] && fpenalty[x]) {
					st.Format("%d=%.0f", x, fpenalty[x]);
					if (rpst != "") rpst += ", ";
					rpst += st;
				}
			}
			st.Format("%.0f (", rpenalty_cur);
			rpst = st + rpst + ")";
			if (key_eval == "") {
				// If SWA
				st.Format("#%d (from MIDI file %s)\nRule penalty: %s\nDistance penalty: %.0f\nHarmonic difficulty: %.0f", cantus_id+1, midi_file, rpst, dpenalty_cur, hdif);
			}
			else {
				// If evaluating
				st.Format("#%d (from MIDI file %s)\nRule penalty: %s\nHarmonic difficulty: %.0f\nKey selection: %s", cantus_id+1, midi_file, rpst, hdif, key_eval);
			}
		}
		AddMelody(step - real_len - 1, step - 1, svoice+1, st);
	}
	// Send
	t_generated = step;
	if (task == tGen) {
		if (!shuffle) {
			// Add line
			linecolor[t_sent] = Color(255, 0, 0, 0);
			t_sent = t_generated;
		}
	}
	st.Format("Sent: %ld (ignored %ld)", cantus_sent, cantus_ignored);
	SetStatusText(0, st);
	// Check limit
	if (t_generated >= t_cnt) {
		WriteLog(3, "Reached t_cnt steps. Generation stopped");
		return 1;
	}
	return 0;
}

void CGenCP1::ReseedCP()
{
	CString st;
	MultiCPInit();
	// Allow two seed cycles for each accept
	seed_cycle = 0;
	++reseed_count;
	st.Format("Reseed: %d", reseed_count);
	SetStatusText(4, st);
}

int CGenCP1::FailVIntervals() {
	int pico_count = 0;
	// Calculate intervals
	for (int i = 0; i < ep2; ++i) {
		ivl[i] = ac[1][i] - ac[0][i];
		ivlc[i] = ivl[i] % 7;
		civl[i] = acc[1][i] - acc[0][i];
		civlc[i] = civl[i] % 12;
		//if (civlc[i] == 1 || civlc[i] == 2 || civlc[i] == 5 || civlc[i] == 6 || civlc[i] == 10 || civlc[i] == 11) tivl[i] = iDis;
		if (civlc[i] == 3 || civlc[i] == 4 || civlc[i] == 8 || civlc[i] == 9) tivl[i] = iIco;
		else if (civlc[i] == 7 || civlc[i] == 0) tivl[i] = iPco;
		else tivl[i] = iDis;
		if (i < ep2-1) direct[i] = (acc[cfv][i+1] - acc[cfv][i])*(acc[cpv][i+1] - acc[cpv][i]);
	}
	// Check first step
	if (tivl[0] == iDis) FLAG2(83, 0);
	for (int i = 1; i < ep2; ++i) {
		// Unison
		if (!civl[i-1]) {
			// Inside
			if (i && i < c_len - 1) FLAG2(91, i-1);
			// Direct exit
			if (direct[i - 1] > 0) FLAG2(92, i);
		}
		// Disonnance
		if (tivl[i] == iDis) {
			// Upbeat
			if (i % 2) FLAG2(88, i)
			// Downbeat
			else FLAG2(83, i);
		}
		// Perfect consonance
		else if (tivl[i] == iPco) {
			// Prohibit parallel 
			if (civl[i] == civl[i - 1]) FLAG2(84, i)
			// Prohibit combinatory
			else if (civlc[i] == civlc[i - 1]) FLAG2(85, i)
			// Prohibit different
			else if (tivl[i-1] == iPco) FLAG2(86, i)
			// All other cases if previous interval is not pco
			else {
				// Direct movement to pco
				if (direct[i-1] > 0) FLAG2(87, i);
				// Prohibit downbeats and culminations only if not last step
				if (i < ep2 - 1) {
					if (i % 2) {
						// Prohibit culmination
						if (acc[cpv][i] == nmax || acc[cfv][i] == nmax) FLAG2(81, i);
					}
					else {
						// Prohibit downbeat culmination
						if (acc[cpv][i] == nmax || acc[cfv][i] == nmax) FLAG2(82, i)
						// Prohibit downbeat
						else FLAG2(80, i);
					}
				}
			}
		}
		// Long parallel ico
		if (tivl[i] == iIco && ivl[i] == ivl[i - 1]) {
			++pico_count;
			// Two same ico transitions means three intervals already
			if (pico_count == 2) {
				FLAG2(89, i)
			}
			else if (pico_count > 2) {
				FLAG2(96, i)
			}
		}
		else pico_count = 0;
	}
	return 0;
}

void CGenCP1::CalcDpenaltyCP() {
	dpenalty_cur = 0;
	for (int z = 0; z < c_len; z++) {
		int dif = abs(acc_old[cpv][z] - acc[cpv][z]);
		if (dif) dpenalty_cur += step_penalty + pitch_penalty * dif;
	}
}

void CGenCP1::SaveCP() {
	// If rpenalty is same as min, calculate dpenalty
	if (optimize_dpenalty) {
		if (rpenalty_cur == rpenalty_min) {
			CalcDpenaltyCP();
			// Do not save cantus if it has higher dpenalty
			if (dpenalty_cur > dpenalty_min) return;
			// Do not save cantus if it is same as source
			if (!dpenalty_cur) return;
			dpenalty_min = dpenalty_cur;
		}
		// If rpenalty lowered, clear dpenalty
		else {
			dpenalty_min = MAX_PENALTY;
			dpenalty_cur = MAX_PENALTY;
		}
		dpenalty.push_back(dpenalty_cur);
	}
	clib.push_back(acc[cpv]);
	rpenalty.push_back(rpenalty_cur);
	rpenalty_min = rpenalty_cur;
}

void CGenCP1::SaveCPIfRp() {
	// Is penalty not greater than minimum of all previous?
	if (rpenalty_cur <= rpenalty_min) {
		// If rpenalty 0, we can skip_flags (if allowed)
		if (!skip_flags && rpenalty_cur == 0)
			skip_flags = !calculate_blocking && !calculate_correlation && !calculate_stat;
		// Insert only if cc is unique
		if (clib_vs.Insert(acc[cpv]))
			SaveCP();
		// Save flags for SWA stuck flags
		if (rpenalty_cur) best_flags = flags;
	}
}

// Detect repeating notes. Step2 excluding
int CGenCP1::FailSlurs(vector<int> &cc, int step1, int step2) {
  // Number of sequential slurs 
	int scount = 0;
	// Number of slurs in window
	int scount2 = 0;
	for (int i = step1; i < step2; ++i) {
		if (cc[i] == cc[i + 1]) {
		  // Check simultaneous slurs
			if (acc[cfv][i] == acc[cfv][i + 1]) {
				FLAG2(98, i);
			}
			// Check slurs sequence
			++scount;
			if (scount > 1) FLAG2(97, i);
			// Check slurs in window
			++scount2;
			// Subtract old slur
			if ((i >= slurs_window) && (cc[i - slurs_window] == cc[i - slurs_window + 1])) --scount2;
			if (scount2 == 1) FLAG2(93, i)
			else if (scount2 == 2) FLAG2(94, i)
			else if (scount2 > 2) FLAG2(95, i);
		}
		else scount = 0;
	}
	return 0;
}

// Count limits
int CGenCP1::FailCPInterval(int step1, int step2) {
	// Calculate range
	nmin = MAX_NOTE;
	nmax = 0;
	int bsteps = 0;
	for (int i = step1; i < step2; ++i) {
		if (acc[cpv][i] < nmin) nmin = acc[cpv][i];
		if (acc[cpv][i] > nmax) nmax = acc[cpv][i];
		// Check between
		if (ac[1][i] - ac[0][i] > max_between) {
			++bsteps;
			// Flag very far burst
			if (ac[1][i] - ac[0][i] > burst_between) FLAG2(11, i);
			if (bsteps > burst_steps) {
				// Flag long burst only on first overrun
				if (bsteps == burst_steps + 1) FLAG2(11, i)
				// Next overruns are sent to fpenalty
				else fpenalty[37] += bsteps - burst_steps;
			}
		}
		else bsteps = 0;
	}
	return 0;
}

void CGenCP1::ScanCP(int t, int v) {
	CString st, st2;
	int finished = 0;
	// Load master parameters
	task = t;
	svoice = v;

	ScanCPInit();
	if (task == tGen) MultiCPInit();
	else SingleCPInit();
	if (FailWindowsLimit()) return;
	// Need to clear flags, because if skip_flags, they can contain previous prohibited flags
	fill(flags.begin(), flags.end(), 0);
	// Analyze combination
check:
	while (true) {
		//LogCantus(cc);
		// Limit melody interval
		if (task == tGen) {
			if (nmax - nmin > max_interval) goto skip;
			if (c_len == ep2 && nmax - nmin < min_interval) goto skip;
			if (cantus_high) {
				if (cf_nmax - nmin > sum_interval) goto skip;
			}
			else {
				if (nmax - cf_nmin > sum_interval) goto skip;
			}
			ClearFlags(0, ep2);
		}
		else {
			ClearFlags(0, ep2);
			if (nmax - nmin > max_interval) FLAG(37, 0);
			if (cantus_high) {
				if (cf_nmax - nmin > sum_interval) FLAG(7, 0);
			}
			else {
				if (nmax - cf_nmin > sum_interval) FLAG(7, 0);
			}
			if (c_len == ep2 && nmax - nmin < min_interval) FLAG(38, 0);
		}
		if (FailCPInterval(0, ep2)) goto skip;
		if (FailSlurs(acc[cpv], 0, ep2 - 1)) goto skip;
		++accepted3;
		if (need_exit && task != tEval) break;
		// Show status
		if (FailDiatonic(ac[cpv], acc[cpv], 0, ep2, minor_cur)) goto skip;
		GetPitchClass(ac[cpv], acc[cpv], apc[cpv], apcc[cpv], 0, ep2);
		if (minor_cur && FailMinor(apcc[cpv])) goto skip;
		//if (MatchVectors(acc[cpv], test_cc, 0, 2)) 
		//WriteLog(1, "Found");
		if (FailLastNotes(apc[cpv], ep2)) goto skip;
		if (FailNoteSeq(apc[cpv], 0, ep2)) goto skip;
		if (FailIntervals(ep2, ac[cpv], acc[cpv], apc[cpv], apcc[cpv])) goto skip;
		if (FailLeapSmooth(ac[cpv], acc[cpv], ep2, aleap[cpv], asmooth[cpv])) goto skip;
		if (FailOutstandingRepeat(ac[cpv], acc[cpv], aleap[cpv], ep2, repeat_steps2, 2, 76)) goto skip;
		if (FailOutstandingRepeat(ac[cpv], acc[cpv], aleap[cpv], ep2, repeat_steps3, 3, 36)) goto skip;
		if (FailLongRepeat(acc[cpv], aleap[cpv], ep2, repeat_steps5, 5, 72)) goto skip;
		if (FailLongRepeat(acc[cpv], aleap[cpv], ep2, repeat_steps7, 7, 73)) goto skip;
		// Calculate diatonic limits
		nmind = CC_C(nmin, tonic_cur, minor_cur);
		nmaxd = CC_C(nmax, tonic_cur, minor_cur);
		GlobalFill(ac[cpv], ep2, nstat2);
		if (FailVIntervals()) goto skip;
		if (FailStagnation(acc[cpv], nstat, ep2)) goto skip;
		if (FailMultiCulm(acc[cpv], ep2)) goto skip;
		if (FailFirstNotes(apc[cpv], ep2)) goto skip;
		if (FailLeap(ac[cpv], ep2, aleap[cpv], asmooth[cpv], nstat2, nstat3)) goto skip;
		//if (FailMelodyHarm(apc[cpv], 0, ep2)) goto skip;

		SaveBestRejected();
		// If we are window-scanning
		if ((task == tGen || task == tCor) && method == mScan) {
			++accepted2;
			CalcFlagStat();
			if (FailFlagBlock()) goto skip;
			if (FailAccept()) goto skip;
			++accepted4[wid];
			// If this is not last window, go to next window
			if (ep2 < c_len) {
				NextWindow();
				goto check;
			}
			// Check random_choose
			if (random_choose < 100) if (rand2() >= (float)RAND_MAX*random_choose / 100.0) goto skip;
		}
		// Calculate rules penalty if we evaluate or correct cantus without full scan
		else {
			CalcRpenalty();
		}
		// Accept cantus
		++accepted;
		TimeBestRejected();
		if (method == mScan && task == tCor) {
			SaveCP();
		}
		else if (method == mSWA && task == tCor) {
			SaveCPIfRp();
		}
		else {
			if (task == tGen && accept_reseed) {
				if (clib_vs.Insert(acc[cpv])) {
					if (SendCP()) break;
					ReseedCP();
					// Start evaluating without scan
					goto check;
				}
				else {
					++cantus_ignored;
				}
			}
			else {
				if (SendCP()) break;
			}
			// Exit if this is evaluation
			if (task == tEval) return;
		}
	skip:
		ScanLeft(acc[cpv], finished);
		if (finished) {
			// Sliding Windows Approximation
			if (method == mSWA) {
				if (NextSWA()) break;
			}
			// Finish if this is last variant in first window and not SWA
			else if ((p == 0) || (wid == 0)) {
				// If we started from random seed, allow one more full cycle
				if (random_seed) {
					if (seed_cycle) {
						// Infinitely cycle through ranges
						if (random_range && accept_reseed) {
							ReseedCP();
							// Start evaluating without scan
							goto check;
						}
						break;
					}
					// Dont show log if we are reseeding after each accept
					if (!accept_reseed) WriteLog(3, "Random seed allows one more full cycle: restarting");
					++seed_cycle;
				}
				else break;
			}
			BackWindow(acc[cpv]);
			// Clear flag to prevent coming here again
			finished = 0;
			// Goto next variant calculation
			goto skip;
		} // if (finished)
		ScanRight(acc[cpv]);
	}
	if (accepted3 > 100000) ShowScanStatus(acc[cpv]);
	WriteFlagCor();
	ShowFlagStat();
	ShowFlagBlock();
}

void CGenCP1::Generate() {
	CString st;
	InitCP();
	LoadCantus(midi_file);
	if (cantus.size() < 1) return;
	// Choose cantus to use
	cantus_id = randbw(0, cantus.size() - 1);
	if (cantus_id2) {
		if (cantus_id2 < cantus.size()) {
			cantus_id = cantus_id2 - 1;
		}
		else {
			CString est;
			est.Format("Warning: cantus_id in configuration file (%d) is greater than number of canti loaded (%d). Selecting highest cantus.", 
				cantus_id2, cantus.size());
			WriteLog(1, est);
			cantus_id = cantus.size() - 1;
		}
	}
	c_len = cantus[cantus_id].size();
	// Get key
	GetCantusKey(cantus[cantus_id]);
	// Get cantus interval
	GetMelodyInterval(cantus[cantus_id], 0, cantus[cantus_id].size(), cf_nmin, cf_nmax);
	if (tonic_cur == -1) return;
	CalcCcIncrement();
	// Show imported melody
	cc_len = cantus_len[cantus_id];
	cc_tempo = cantus_tempo[cantus_id];
	real_len = accumulate(cantus_len[cantus_id].begin(), cantus_len[cantus_id].end(), 0);
	dpenalty_cur = 0;
	// Create pause
	FillPause(0, real_len, 1);
	// Select rule set
	SelectRuleSet(cf_rule_set);
	ScanCantus(tEval, 0, &(cantus[cantus_id]));
	// Show cantus id
	st.Format("Cantus %d. ", cantus_id + 1);
	comment[0][0] = st + comment[0][0];
	// Go forward
	Adapt(0, real_len);
	t_generated = real_len;
	t_sent = t_generated;
	// Choose level
	if (cantus_high) {
		cpv = 0;
		cfv = 1;
	}
	else {
		cpv = 1;
		cfv = 0;
	}
	// Load first voice
	ac[cfv] = c;
	acc[cfv] = cc;
	apc[cfv] = pc;
	apcc[cfv] = pcc;
	// Set uniform length of each cantus note
	//cc_len.resize(c_len);
	//cc_tempo.resize(c_len);
	//real_len = c_len;
	//for (int i = 0; i < c_len; ++i) cc_len[i] = 1;
	// Generate second voice
	rpenalty_cur = MAX_PENALTY;
	SelectRuleSet(cp_rule_set);
	ScanCP(tGen, 0);
}
