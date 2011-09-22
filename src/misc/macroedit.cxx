// ----------------------------------------------------------------------------
// macroedit.cxx
//
// Copyright (C) 2007-2009
//		Dave Freese, W1HKJ
// Copyright (C) 2009
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include <string>
#ifndef __MINGW32__
#  include <glob.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cassert>

#include <FL/Fl_Pixmap.H>

#include "macros.h"
#include "macroedit.h"
#include "globals.h"
#include "status.h"
#include "fileselect.h"
#include "fl_lock.h"
#include "fl_digi.h"
#include "main.h"
#include "gettext.h"
#include "pixmaps.h"
#include "configuration.h"

using namespace std;

Fl_Double_Window *MacroEditDialog = (Fl_Double_Window *)0;

Fl_Button	*btnMacroEditApply = (Fl_Button *)0;
Fl_Button	*btnMacroEditClose = (Fl_Button *)0;
Fl_Button	*btnInsertMacro = (Fl_Button *)0;
Fl_Input2	*macrotext = (Fl_Input2 *)0;
Fl_Input2	*labeltext = (Fl_Input2 *)0;
static int widths[] = {150, 0};

Fl_Hold_Browser *macroDefs=(Fl_Hold_Browser *)0;

static int iMacro, iType;
static Fl_Input* iInput;

// fl_color(0) is always the foreground colour
#define LINE_SEP "@B0"

void loadBrowser(Fl_Widget *widget) {
	Fl_Browser *w = (Fl_Browser *)widget;
	/* Do not translate the tags lefthand-side */
	w->add(_("<FREQ>\tmy frequency"));
	w->add(_("<MODE>\tmode"));
	w->add(_("<MYCALL>\tmy call"));
	w->add(_("<MYLOC>\tmy locator"));
	w->add(_("<MYNAME>\tmy name"));
	w->add(_("<MYQTH>\tmy QTH"));
	w->add(_("<MYRST>\tmy RST"));
	w->add(_("<VER>\tFldigi version"));

	w->add(LINE_SEP);
	w->add(_("<CALL>\tother call"));
	w->add(_("<INFO1>\tS/N etc."));
	w->add(_("<INFO2>\tIMD etc."));
	w->add(_("<LOC>\tother locator"));
	w->add(_("<NAME>\tother name"));
	w->add(_("<QTH>\tother QTH"));
	w->add(_("<RST>\tother RST"));
	w->add(_("<MAPIT>\tmap on google"));
	w->add(_("<MAPIT:adr/lat/loc>\tmap by value"));
 
	w->add(LINE_SEP);
	w->add(_("<CLRRX>\tclear RX pane"));
	w->add(_("<CLRTX>\tclear TX pane"));

	w->add(LINE_SEP);
	w->add(_("<GET>\ttext to NAME/QTH"));

#ifdef __WIN32__
	w->add(LINE_SEP);
	w->add(_("<TALK:on|off|t>\tDigitalk On, Off, Toggle"));
#endif

	w->add(LINE_SEP);
	w->add(_("<LOG>\tsave QSO data"));
	w->add(_("<LNW>\tlog at xmt time"));
	w->add(_("<CLRLOG>\tclear log fields"));

	w->add(LINE_SEP);
	w->add(_("<QSOTIME>\tQSO time (HHMM))"));
	w->add(_("<ILDT>\tLDT in iso-8601 format"));
	w->add(_("<LDT>\tLocal datetime"));
	w->add(_("<IZDT>\tZDT in iso-8601 format"));
	w->add(_("<ZDT>\tUTC datetime"));
	w->add(_("<LT>\tlocal time HHMM"));
	w->add(_("<ZT>\tzulu time HHMMZ"));
	w->add(_("<LD>\tlocal date YYYY-MM-DD"));
	w->add(_("<ZD>\tzulu date YYYY-MM-DD Z"));

	w->add(LINE_SEP);
	w->add(_("<CNTR>\tcontest counter"));
	w->add(_("<DECR>\tdecrement counter"));
	w->add(_("<INCR>\tincrement counter"));
	w->add(_("<XIN>\texchange in"));
	w->add(_("<XOUT>\texchange out"));
	w->add(_("<XBEG>\texchange begin"));
	w->add(_("<XEND>\texchange end"));
	w->add(_("<SAVEXCHG>\tsave contest out"));

	w->add(LINE_SEP);
	w->add(_("<RX>\treceive"));
	w->add(_("<TX>\ttransmit"));
	w->add(_("<TX/RX>\ttoggle T/R"));
	w->add(_("<SRCHUP>\tsearch UP for signal"));
	w->add(_("<SRCHDN>\tsearch DOWN for signal"));
	w->add(_("<GOHOME>\treturn to sweet spot"));
	w->add(_("<GOFREQ:NNNN>\tmove to freq NNNN Hz"));
	w->add(_("<QSYTO>\tleft-clk QSY button"));
	w->add(_("<QSYFM>\tright-clk QSY button"));

	w->add(LINE_SEP);
	w->add(_("<QSY:FFF.F[:NNNN]>\tqsy to kHz, Hz"));
	w->add(_("<RIGMODE:mode>\tvalid xcvr mode"));
	w->add(_("<FILWID:width>\tvalid xcvr filter width"));

	w->add(LINE_SEP);
	w->add(_("<FILE:>\tinsert text file"));
	w->add(_("<IDLE:NN.nn>\tidle signal for NN.nn sec"));
	w->add(_("<TIMER:NN>\trepeat every NN sec"));
	w->add(_("<TUNE:NN>\ttune signal for NN sec"));
	w->add(_("<WAIT:NN>\tdelay xmt for NN sec"));
	w->add(_("<REPEAT>\trepeat macro continuously"));
	w->add(_("<SKED:hhmm[:YYYYDDMM]>\tschedule execution"));

	w->add(LINE_SEP);
	w->add(_("<CWID>\tCW identifier"));
	w->add(_("<ID>\tsend mode ID in video text"));
	w->add(_("<TEXT>\tvideo text"));
	w->add(_("<TXRSID:on|off|t>\tTx RSID on,off,toggle"));
	w->add(_("<RXRSID:on|off|t>\tRx RSID on,off,toggle"));
	w->add(_("<DTMF:[Wn:][Ln:]chrs>\t[Wait][Len](ms)"));

	w->add(LINE_SEP);
	w->add(_("<POST:+/-nn.n>\tCW QSK post-timing"));
	w->add(_("<PRE:nn.n>\tCW QSK pre-timing"));
	w->add(_("<RISE:nn.n>\tCW rise time"));
	w->add(_("<WPM:NN>\tCW WPM"));

	w->add(LINE_SEP);
	w->add(_("<AFC:on|off|t>\tAFC  on,off,toggle"));
	w->add(_("<LOCK:on|off|t>\tLOCK on,off,toggle"));

	w->add(LINE_SEP);
	w->add(_("<MACROS:>\tchange macro defs file"));

	w->add(LINE_SEP);
	assert(MODE_CONTESTIA < MODE_OLIVIA);
	char s[256];
	for (trx_mode i = 0; i <= MODE_CONTESTIA; i++) {
		snprintf(s, sizeof(s), "<MODEM:%s>", mode_info[i].sname);
		w->add(s);
	}
	// add some Contestia macros
	const char* contestia[] = { "250:8", "500:8", "500:16", "1000:8", "1000:16" };
	for (size_t i = 0; i < sizeof(contestia)/sizeof(*contestia); i++) {
		snprintf(s, sizeof(s), "<MODEM:%s:%s>", mode_info[MODE_CONTESTIA].sname, contestia[i]);
		w->add(s);
	}
	for (trx_mode i = MODE_CONTESTIA + 1; i <= MODE_OLIVIA; i++) {
		snprintf(s, sizeof(s), "<MODEM:%s>", mode_info[i].sname);
		w->add(s);
	}
	assert(MODE_OLIVIA < MODE_RTTY);
	// add some Olivia macros
	const char* olivia[] = { "250:8", "500:8", "500:16", "1000:8", "1000:32" };
	for (size_t i = 0; i < sizeof(olivia)/sizeof(*olivia); i++) {
		snprintf(s, sizeof(s), "<MODEM:%s:%s>", mode_info[MODE_OLIVIA].sname, olivia[i]);
		w->add(s);
	}
	for (trx_mode i = MODE_OLIVIA + 1; i <= MODE_RTTY; i++) {
		snprintf(s, sizeof(s), "<MODEM:%s>", mode_info[i].sname);
		w->add(s);
	}
	// add some RTTY macros
	const char* rtty[] = { "170:45.45:5", "170:50:5", "850:75:5" };
	for (size_t i = 0; i < sizeof(rtty)/sizeof(*rtty); i++) {
		snprintf(s, sizeof(s), "<MODEM:%s:%s>", mode_info[MODE_RTTY].sname, rtty[i]);
		w->add(s);
	}
	for (trx_mode i = MODE_RTTY + 1; i < NUM_MODES; i++) {
		snprintf(s, sizeof(s), "<MODEM:%s>", mode_info[i].sname);
		w->add(s);
	}

#ifndef __MINGW32__
	glob_t gbuf;
	glob(string(ScriptsDir).append("*").c_str(), 0, NULL, &gbuf);
	if (gbuf.gl_pathc == 0) {
		globfree(&gbuf);
		return;
	}
	w->add(LINE_SEP);
	struct stat st;
	for (size_t i = 0; i < gbuf.gl_pathc; i++) {
		if (!(stat(gbuf.gl_pathv[i], &st) == 0
		      && S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR)))
			continue;

		const char* p;
		if ((p = strrchr(gbuf.gl_pathv[i], '/'))) {
			snprintf(s, sizeof(s), "<EXEC>%s</EXEC>", p+1);
			w->add(s);
		}
	}
	globfree(&gbuf);
#else
	w->add("<EXEC>\tlaunch a program");
#endif
}

void cbMacroEditOK(Fl_Widget *w, void *)
{
	if (w == btnMacroEditClose) {
		MacroEditDialog->hide();
		return;
	}

	if (iType == MACRO_EDIT_BUTTON) {
		macros.text[iMacro] = macrotext->value();
		macros.name[iMacro] = labeltext->value();

		if (progdefaults.mbar2_pos) {
			if (iMacro < NUMMACKEYS) {
				btnMacro[iMacro % NUMMACKEYS]->label( macros.name[iMacro].c_str() );
				btnMacro[iMacro % NUMMACKEYS]->redraw_label();
			} else {
				btnMacro[(iMacro % NUMMACKEYS) + NUMMACKEYS]->label( macros.name[iMacro].c_str() );
				btnMacro[(iMacro % NUMMACKEYS) + NUMMACKEYS]->redraw_label();
			}
		} else {
			btnMacro[iMacro % NUMMACKEYS]->label( macros.name[iMacro].c_str() );
			btnMacro[iMacro % NUMMACKEYS]->redraw_label();
		}

		macros.changed = true;
	}
	else if (iType == MACRO_EDIT_INPUT)
		iInput->value(macrotext->value());
}

void cbInsertMacro(Fl_Widget *, void *)
{
	int nbr = macroDefs->value();
	if (!nbr) return;
	string edittext = macrotext->value();
	string text = macroDefs->text(nbr);
	size_t tab = text.find('\t');
	if (tab != string::npos)
		text.erase(tab);
	if (text == LINE_SEP)
		return;
	if (text == "<FILE:>") {
		string filters = "Text\t*." "txt";
		const char* p = FSEL::select(_("Text file to insert"), filters.c_str(),
					 "text." "txt");
		if (p) {
			text.insert(6, p);
		} else
			text = "";
	} else if (text == "<MACROS:>") {
		string filters = "Macrost\t*." "mdf";
		const char* p = FSEL::select(_("Change to Macro file"), filters.c_str(),
					 "macros." "mdf");
		if (p) {
			text.insert(8, p);
		} else
			text = "";
	}
#ifdef __MINGW32__
	else if (text == "<EXEC>") {
		string filters = "Exe\t*." "exe";
		const char* p = FSEL::select(_("Executable file to insert"), filters.c_str(),
					"WordPad.exe");
		if (p) {
			string exefile = p;
			exefile.append("</EXEC>");
			text.insert(6, exefile);
		} else
			text = "";
	}
#endif
	macrotext->insert(text.c_str());
	macrotext->take_focus();
}

Fl_Double_Window* make_macroeditor(void)
{
	Fl_Double_Window* w = new Fl_Double_Window(800, 190, "");

//	Fl_Group *grp1 = new Fl_Group(0,0, 800, 22, "");
	Fl_Group *grp1a = new Fl_Group(452,0,348, 22, "");
	btnInsertMacro = new Fl_Button(452, 2, 40, 20);
	btnInsertMacro->image(new Fl_Pixmap(left_arrow_icon));
	btnInsertMacro->callback(cbInsertMacro);
	Fl_Box *box1a = new Fl_Box(492, 2, 308, 22, "");
	grp1a->end();
	grp1a->resizable(box1a);
//	grp1->end();

	Fl_Group *grp2 = new Fl_Group(0,22, 800, 140, "");
	macrotext = new Fl_Input2(2, 22, 450, 140, _("Macro Text"));
	macrotext->type(FL_MULTILINE_INPUT);
	macrotext->textfont(FL_COURIER);
	macrotext->align(FL_ALIGN_TOP);

	macroDefs = new Fl_Hold_Browser(452, 22, 346, 140, _("Select Tag"));
	macroDefs->column_widths(widths);
	macroDefs->align(FL_ALIGN_TOP);
	grp2->end();

	Fl_Group *grp3a = new Fl_Group(0, 164, 452, 24, "");
	Fl_Box *box3a = new Fl_Box(0, 164, 327, 24, "");
	labeltext = new Fl_Input2(337, 164, 115, 24, _("Macro Button Label"));
	labeltext->textfont(FL_COURIER);
	grp3a->end();
	grp3a->resizable(box3a);

	Fl_Group *grp4 = new Fl_Group(452, 164, 348, 24, "");
	Fl_Box *box4a = new Fl_Box(452, 164, 92, 24, "");

	btnMacroEditApply = new Fl_Button(544, 164, 80, 24, _("Apply"));
	btnMacroEditApply->callback(cbMacroEditOK);

	btnMacroEditClose = new Fl_Button(626 , 164, 80, 24, _("Close"));
	btnMacroEditClose->callback(cbMacroEditOK);
	grp4->end();
	grp4->resizable(box4a);

	w->end();

	w->resizable(grp2);

	w->size_range( 600, 120);

	w->xclass(PACKAGE_NAME);

	loadBrowser(macroDefs);

	return w;
}

void editMacro(int n, int t, Fl_Input* in)
{
	if (!MacroEditDialog)
		MacroEditDialog = make_macroeditor();
	if (t == MACRO_EDIT_BUTTON) {
		string editor_label;
		editor_label.append(_("Macro editor - ")).append(progStatus.LastMacroFile);
		if (editor_label != MacroEditDialog->label())
			MacroEditDialog->copy_label(editor_label.c_str());
		macrotext->value(macros.text[n].c_str());
		labeltext->value(macros.name[n].c_str());
		labeltext->show();
	}
	else if (t == MACRO_EDIT_INPUT) {
		MacroEditDialog->label(in->label());
		macrotext->value(in->value());
		labeltext->hide();
	}
	iMacro = n;
	iType = t;
	iInput = in;
	MacroEditDialog->show();
}
