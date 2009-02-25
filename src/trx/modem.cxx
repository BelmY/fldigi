// modem class - base for all modems
//

#include <config.h>

#include "filters.h"

#include "confdialog.h"
#include "modem.h"
#include "configuration.h"

#include "qrunner.h"

#include "status.h"
#include "debug.h"

modem *cw_modem = 0;

modem *mfsk8_modem = 0;
modem *mfsk16_modem = 0;
modem *mfsk32_modem = 0;
// experimental modes
modem *mfsk4_modem = 0;
modem *mfsk11_modem = 0;
modem *mfsk22_modem = 0;
modem *mfsk31_modem = 0;
modem *mfsk64_modem = 0;

modem *mt63_500_modem = 0;
modem *mt63_1000_modem = 0;
modem *mt63_2000_modem = 0;

modem *feld_modem = 0;
modem *feld_slowmodem = 0;
modem *feld_x5modem = 0;
modem *feld_x9modem = 0;
modem *feld_FMmodem = 0;
modem *feld_FM105modem = 0;
modem *feld_80modem = 0;
modem *feld_CMTmodem = 0;

modem *psk31_modem = 0;
modem *psk63_modem = 0;
modem *psk125_modem = 0;
modem *psk250_modem = 0;

modem *qpsk31_modem = 0;
modem *qpsk63_modem = 0;
modem *qpsk125_modem = 0;
modem *qpsk250_modem = 0;

modem *olivia_modem = 0;

modem *rtty_modem = 0;

modem *thor4_modem = 0;
modem *thor5_modem = 0;
modem *thor8_modem = 0;
modem *thor11_modem = 0;
//modem *tsor11_modem = 0;
modem *thor16_modem = 0;
modem *thor22_modem = 0;

modem *dominoex4_modem = 0;
modem *dominoex5_modem = 0;
modem *dominoex8_modem = 0;
modem *dominoex11_modem = 0;
modem *dominoex16_modem = 0;
modem *dominoex22_modem = 0;

modem *throb1_modem = 0;
modem *throb2_modem = 0;
modem *throb4_modem = 0;
modem *throbx1_modem = 0;
modem *throbx2_modem = 0;
modem *throbx4_modem = 0;

modem *wwv_modem = 0;
modem *anal_modem = 0;

trx_mode modem::get_mode()
{
	return mode;
}

modem::modem()
{
	scptr = 0;
	freqlock = false;
	sigsearch = 0;
	bool wfrev = wf->Reverse();
	bool wfsb = wf->USB();
	reverse = wfrev ^ !wfsb;
	historyON = false;
	cap = 0;
	PTTphaseacc = 0.0;
}

void modem::init()
{
	bool wfrev = wf->Reverse();
	bool wfsb = wf->USB();
	reverse = wfrev ^ !wfsb;
	
	if (progdefaults.StartAtSweetSpot) {
//		if (active_modem == cw_modem)
		if (this == cw_modem)
			set_freq(progdefaults.CWsweetspot);
//		else if (active_modem == rtty_modem)
		else if (this == rtty_modem)
			set_freq(progdefaults.RTTYsweetspot);
		else
			set_freq(progdefaults.PSKsweetspot);
	} else if (progStatus.carrier != 0) {
		set_freq(progStatus.carrier);
#if !BENCHMARK_MODE
		progStatus.carrier = 0;
#endif
	} else
		set_freq(wf->Carrier());
}

void modem::set_freq(double freq)
{
	frequency = freq;
//	freqerr = 0.0;
	if (freqlock == false)
		tx_frequency = frequency;
	REQ(put_freq, frequency);
}

void modem::set_freqlock(bool on)
{
	if (on == false)
		tx_frequency = frequency;
	freqlock = on;
}


bool modem::freqlocked()
{
	return freqlock;
}

double modem::get_txfreq(void)
{
	if (mailserver && progdefaults.PSKmailSweetSpot)
		return progdefaults.PSKsweetspot;
	return tx_frequency;
}

double modem::get_txfreq_woffset(void)
{
	if (mailserver && progdefaults.PSKmailSweetSpot)
		return (progdefaults.PSKsweetspot - progdefaults.TxOffset);
	return (tx_frequency - progdefaults.TxOffset);
}

int modem::get_freq()
{
	return (int)(frequency + 0.5);
}

double modem::get_bandwidth(void)
{
	return bandwidth;
}

void modem::set_bandwidth(double bw)
{
	bandwidth = bw;
	put_Bandwidth((int)bandwidth);
}

void modem::set_reverse(bool on)
{
	reverse = on ^ (!wf->USB());
}

void modem::set_metric(double m)
{
	metric = m;
}

double modem::get_metric(void)
{
	return metric;
}


bool modem::get_cwTrack()
{
	return cwTrack;
}

void modem::set_cwTrack(bool b)
{
	cwTrack = b;
}

bool modem::get_cwLock()
{
	return cwLock;
}

void modem::set_cwLock(bool b)
{
	cwLock = b;
}

double modem::get_cwRcvWPM()
{
	return cwRcvWPM;
}

double modem::get_cwXmtWPM()
{
	return cwXmtWPM;
}

void modem::set_cwXmtWPM(double wpm)
{
	cwXmtWPM = wpm;
}
	
int modem::get_samplerate(void)
{
	return samplerate;
}

void modem::set_samplerate(int smprate)
{
	samplerate = smprate;
}

double modem::PTTnco()
{
	PTTphaseacc += TWOPI * 1000 / samplerate;
	if (PTTphaseacc > M_PI)
		PTTphaseacc -= TWOPI;
	return sin(PTTphaseacc);
}

mbuffer<double, 512 * 2, 2> _mdm_scdbl;

void modem::ModulateXmtr(double *buffer, int len) 
{
    if (progdefaults.PTTrightchannel) {
        for (int i = 0; i < len; i++)
            PTTchannel[i] = PTTnco();
            ModulateStereo( buffer, PTTchannel, len);
        return;
    }
	try {
		unsigned n = 4;
		while (scard->Write(buffer, len) == 0 && --n);
		if (n == 0)
			throw SndException("Sound write failed");
	}
	catch (const SndException& e) {
		LOG_ERROR("%s", e.what());
		return;
	}

	if (!progdefaults.viewXmtSignal)
		return;
	for (int i = 0; i < len; i++) {
		_mdm_scdbl[scptr] = buffer[i] * progdefaults.TxMonitorLevel;
		scptr++;
		if (scptr == 512) {
			REQ(&waterfall::sig_data, wf, _mdm_scdbl.c_array(), 512, samplerate );
			scptr = 0;
			_mdm_scdbl.next(); // change buffers
		}
	}
}

void modem::ModulateStereo(double *left, double *right, int len)
{
	try {
		unsigned n = 4;
		while (scard->Write_stereo(left, right, len) == 0 && --n);
		if (n == 0)
			throw SndException("Sound write failed");
	}
	catch (const SndException& e) {
		LOG_ERROR("%s", e.what());
		return;
	}

	if (!progdefaults.viewXmtSignal)
		return;
	for (int i = 0; i < len; i++) {
		_mdm_scdbl[scptr] = left[i] * progdefaults.TxMonitorLevel;
		scptr++;
		if (scptr == 512) {
			REQ(&waterfall::sig_data, wf, _mdm_scdbl.c_array(), 512, samplerate);
			scptr = 0;
			_mdm_scdbl.next(); // change buffers
		}
	}
}


void modem::videoText()
{
	if (trx_state == STATE_TUNE)
		return;
	if (progdefaults.sendtextid == true) {
		wfid_text(progdefaults.strTextid);
	} else if (progdefaults.macrotextid == true) {
		wfid_text(progdefaults.strTextid);
		progdefaults.macrotextid = false;
	}
	if (progdefaults.sendid == true) {
		wfid_text(mode_info[mode].sname);
	} else if (progdefaults.macroid == true) {
		wfid_text(mode_info[mode].sname);
		progdefaults.macroid = false;
	}
}

// CW ID transmit routines

//===========================================================================
// cw transmit routines to send a post amble message
// Define the amplitude envelop for key down events      
// this is 1/2 cycle of a raised cosine                                    
//===========================================================================

void modem::cwid_makeshape()
{
	for (int i = 0; i < 128; i++) cwid_keyshape[i] = 1.0;
	for (int i = 0; i < RT; i++)
		cwid_keyshape[i] = 0.5 * (1.0 - cos (M_PI * i / RT));
}

double modem::cwid_nco(double freq)
{
	cwid_phaseacc += 2.0 * M_PI * freq / samplerate;

	if (cwid_phaseacc > M_PI)
		cwid_phaseacc -= 2.0 * M_PI;

	return sin(cwid_phaseacc);
}

//=====================================================================
// cwid_send_symbol()
// Sends a part of a morse character (one dot duration) of either
// sound at the correct freq or silence. Rise and fall time is controlled
// with a raised cosine shape.
//
// Left channel contains the shaped A2 CW waveform
//=======================================================================

void modem::cwid_send_symbol(int bits)
{
	double freq;
	int i,
		keydown,
		keyup,
		sample = 0, 
		currsym = bits & 1;
	
	freq = tx_frequency - progdefaults.TxOffset;

    if ((currsym == 1) && (cwid_lastsym == 0))
    	cwid_phaseacc = 0.0;

	keydown = cwid_symbollen - RT;
	keyup = cwid_symbollen - RT;
	
	if (currsym == 1) {
		for (i = 0; i < RT; i++, sample++) {
			if (cwid_lastsym == 0)
				outbuf[sample] = cwid_nco(freq) * cwid_keyshape[i];
			else
				outbuf[sample] = cwid_nco(freq);
		}
		for (i = 0; i < keydown; i++, sample++) {
			outbuf[sample] = cwid_nco(freq);
		}
	}
	else {
		for (i = RT - 1; i >= 0; i--, sample++) {
			if (cwid_lastsym == 1) {
				outbuf[sample] = cwid_nco(freq) * cwid_keyshape[i];
			} else {
				outbuf[sample] = 0.0;
			}
		}
		for (i = 0; i < keyup; i++, sample++) {
			outbuf[sample] = 0.0;
		}
	}

	ModulateXmtr(outbuf, cwid_symbollen);
	
	cwid_lastsym = currsym;
}

//=====================================================================
// send_ch()
// sends a morse character and the space afterwards
//=======================================================================

void modem::cwid_send_ch(int ch)
{
	int code;

// handle word space separately (7 dots spacing) 
// last char already had 2 elements of inter-character spacing 

	if ((ch == ' ') || (ch == '\n')) {
		cwid_send_symbol(0);
		cwid_send_symbol(0);
		cwid_send_symbol(0);
		cwid_send_symbol(0);
		cwid_send_symbol(0);
		put_echo_char(ch);
		return;
	}

// convert character code to a morse representation 
	if ((ch < 256) && (ch >= 0)) {
		code = tx_lookup(ch); //cw_tx_lookup(ch);
	} else {
		code = 0x04; 	// two extra dot spaces
	}

// loop sending out binary bits of cw character 
	while (code > 1) {
		cwid_send_symbol(code);// & 1);
		code = code >> 1;
	}

}

void modem::cwid_sendtext (const string& s)
{
	cwid_symbollen = (int)(1.2 * samplerate / progdefaults.CWIDwpm);
	RT = (int) (samplerate * 6 / 1000.0); // 6 msec risetime for CW pulse
	cwid_makeshape();
	cwid_lastsym = 0;
	for (unsigned int i = 0; i < s.length(); i++) {
		cwid_send_ch(s[i]);
	}
}

void modem::cwid()
{
	if (progdefaults.CWid == true || progdefaults.macroCWid == true) {
		string tosend = " DE ";
		tosend += progdefaults.myCall;
		cwid_sendtext(tosend);
		progdefaults.macroCWid = false;
	}
}

//=====================================================================
// transmit processing of waterfall video id
//=====================================================================

int NUMROWS;
int NUMCOLS;
int NUMTONES;
int TONESPACING;
int IDSYMLEN;
int RISETIME;
int CHARSPACE;
int MAXCHARS;
int NUMCHARS;

bool useIDSMALL = true;

#define MAXROWS 14
#define MAXIDSYMLEN 4096
#define MAXTONES 32

struct mfntchr  { char c; int byte[MAXROWS]; };
extern mfntchr  idch1[]; // original id font definition
extern mfntchr  idch2[]; // extended id font definition
extern int		mask[1024];

C_FIR_filter vidfilt;

void modem::wfid_make_pulse()
{
	double risetime = (samplerate / 1000) * RISETIME;
	for (int i = 0; i < IDSYMLEN; i++)
		wfid_txpulse[i] = 1.0;
	for (int i = 0; i < risetime; i++) {
		wfid_txpulse[i] = wfid_txpulse[IDSYMLEN - 1 - i] =
			0.5 * (1 - cos(M_PI * i / risetime));
	}
}

void modem::wfid_make_tones()
{
	if (useIDSMALL) {
		double f;
		double frequency = get_txfreq_woffset();
		for (int j = 0; j < NUMCHARS; j++)
			for (int i = 0; i < NUMTONES; i++) {
				f = frequency + TONESPACING * (NUMTONES - i - j * (NUMTONES + 3));
				wfid_w[i + NUMTONES * j] = 2 * M_PI * f / samplerate;
			}
	} else {
		double f, flo, fhi;
		f = TONESPACING * ( progdefaults.videowidth * (NUMCOLS - 1) + (progdefaults.videowidth) * CHARSPACE) / 2.0;
		f = 2.0 * floor((frequency + f)/2.0);
		fhi = f + 2 * TONESPACING;
		flo = f - (progdefaults.videowidth * (NUMCOLS + CHARSPACE) + 2) * TONESPACING; 
		for (int i = 0; i < NUMCOLS * progdefaults.videowidth; i++) {
			wfid_w[i] = f * 2.0 * M_PI / samplerate;
			f -= TONESPACING;
			if ( (i > 0) && (i % NUMCOLS == 0) )
				f -= TONESPACING * CHARSPACE;
		}
		vidfilt.init_bandpass( 1024, 1, flo/samplerate, fhi/samplerate) ;	
	}
}

void modem::wfid_send(long int symbol)
{
	if (useIDSMALL) {
		int i, j;
		int sym;
		int msk;
		double maximum = 0.0;
	
		for (i = 0; i < IDSYMLEN; i++) {
			wfid_outbuf[i] = 0.0;
			sym = symbol;
			msk = mask[symbol];
			for (j = 0; j < NUMTONES * NUMCHARS; j++) {
				if (sym & 1)
					wfid_outbuf[i] += (msk & 1 ? -1 : 1 ) * sin(wfid_w[j] * i) * wfid_txpulse[i];
				sym = sym >> 1;
				msk = msk >> 1;
			}
			if (fabs(wfid_outbuf[i]) > maximum)
				maximum = fabs(wfid_outbuf[i]);
		}
		if (maximum > 0.0)
			for (i = 0; i < IDSYMLEN; i++)
				wfid_outbuf[i] = 0.9 * wfid_outbuf[i] / maximum;
		
		ModulateXmtr(wfid_outbuf, IDSYMLEN);
	} else {
		int i, j;
		int sym;
		double val;

		for (i = 0; i < IDSYMLEN; i++) {
			wfid_outbuf[i] = 0.0;
			val = 0.0;
			sym = symbol;
			for (j = 0; j < NUMCOLS * progdefaults.videowidth; j++) {
				if ((sym & 1) == 1)
					val += sin(wfid_w[j] * i);
				sym = sym >> 1;
			}
// soft limit the signal
			wfid_outbuf[i] = 0.707 * (1.0 - exp(fabs(val)/-3.0)) * (val >= 0.0 ? 1 : -1);
		}

// band pass filter the hard limited signal

		for (i = 0; i < IDSYMLEN; i++) {
			val = wfid_outbuf[i];
			vidfilt.Irun (val, val);
			wfid_outbuf[i] = val * wfid_txpulse[i];
		}
	
		ModulateXmtr(wfid_outbuf, IDSYMLEN);
	}
}

void modem::wfid_sendchar(char c)
{
// send rows from bottom to top so they appear to scroll down the waterfall correctly
	long int symbol;
	unsigned int n;
	if (c < ' ' || c > '~') return;
	n = c - ' ';
	for (int row = 0; row < NUMROWS; row++) {
		symbol = (idch2[n].byte[NUMROWS - 1 - row]) >> (16 - NUMCOLS);
		wfid_send (symbol);
		if (stopflag)
			return;
	}
}

void modem::wfid_sendchars(string s)
{
	if (useIDSMALL) {
		long int symbol;
		int  len = s.length();
		int  n[NUMCHARS];
		int  c;
		while (len++ < NUMCHARS) s.insert(0," ");

		for (int i = 0; i < NUMCHARS; i++) {
			c = s[i];
			if (c > 'z' || c < ' ') c = ' ';
			c = toupper(c) - ' ';
			n[i] = c;
		}
		for (int row = 0; row < NUMROWS; row++) {
			symbol = 0;
			for (int i = 0; i < NUMCHARS; i++) {
				symbol |= (idch1[n[i]].byte[NUMROWS - 1 -row] >> 3);
				if (i != (NUMCHARS - 1) )
					symbol = symbol << 5;
			}
			wfid_send (symbol);
		}
		wfid_send (0); // row of no tones
	} else {
		long int symbol;
		int  len;
		unsigned int  n[progdefaults.videowidth];
		int  c;
	
		while (s[0] == ' ') s.erase(0);
		len = s.length();

		for (int i = 0; i < len; i++) { //progdefaults.videowidth; i++) {
			c = s[i];
			if (c > '~' || c < ' ') c = ' ';
			c -= ' ';
			n[i] = c;
		}
	
// send rows from bottom to top so they appear to scroll down the waterfall correctly
		for (int row = 0; row < NUMROWS; row++) {
			symbol = 0;
			for (int i = 0; i < len; i++) {//progdefaults.videowidth; i++) {
				symbol |= (idch2[n[i]].byte[NUMROWS - 1 -row] >> (16 - NUMCOLS));
				if (i != (len - 1) )
					symbol = symbol << NUMCOLS;
			}

			if (len < progdefaults.videowidth)
				symbol = symbol << NUMCOLS * (progdefaults.videowidth - len) / 2;

			wfid_send (symbol);
			if (stopflag)
				return;
		}
	}
}

void modem::wfid_text(const string& s)
{
	int len = s.length();
	string video = "Video text: ";
	video += s;
	
	if (progdefaults.ID_SMALL) {
		NUMROWS = 5;
		NUMCHARS = 2;
		NUMTONES = 5;
		TONESPACING = 6;
		IDSYMLEN = 3072;
		RISETIME =20;
		useIDSMALL = true;
	} else {
		NUMROWS = 14;
		NUMCOLS = 8;
		TONESPACING = 8;
		IDSYMLEN = 2048;
		RISETIME = 20;
		CHARSPACE = 2;
		MAXCHARS = 4;
		useIDSMALL = false;
	}

	wfid_make_pulse();
	wfid_make_tones();
	
	put_status(video.c_str());

	if (useIDSMALL) {
		int numlines = 0;
		string tosend;
		while (numlines < len) numlines += NUMCHARS;
		numlines -= NUMCHARS;
		while (numlines >= 0) {
			tosend = s.substr(numlines, NUMCHARS);
			wfid_sendchars(tosend);
			numlines -= NUMCHARS;
			if (stopflag)
				break;
		}
		put_status("");
	} else {
		if (progdefaults.videowidth == 1) {
			for (int i = len - 1; i >= 0; i--) {
				wfid_sendchar(s[i]);
			}
		} else {
			int numlines = 0;
			string tosend;
			while (numlines < len) numlines += progdefaults.videowidth;
			numlines -= progdefaults.videowidth;
			while (numlines >= 0) {
				tosend = s.substr(numlines, progdefaults.videowidth);
				wfid_sendchars(tosend);
				numlines -= progdefaults.videowidth;
				if (stopflag)
					break;
			}
		}
		put_status("");
	}
}

double	modem::wfid_txpulse[MAXIDSYMLEN];
double	modem::wfid_outbuf[MAXIDSYMLEN];
double  modem::wfid_w[MAXTONES];

mfntchr idch1[] = {
{' ', { 0x00, 0x00, 0x00, 0x00, 0x00 }, },
{'!', { 0x80, 0x80, 0x80, 0x00, 0x80 }, },
{'"', { 0xA0, 0x00, 0x00, 0x00, 0x00 }, },
{'#', { 0x50, 0xF8, 0x50, 0xF8, 0x50 }, },
{'$', { 0x78, 0xA0, 0x70, 0x28, 0xF0 }, },
{'%', { 0xC8, 0x10, 0x20, 0x40, 0x98 }, },
{'&', { 0x40, 0xE0, 0x68, 0x90, 0x78 }, },
{ 39, { 0xC0, 0x40, 0x80, 0x00, 0x00 }, },
{'(', { 0x60, 0x80, 0x80, 0x80, 0x60 }, },
{')', { 0xC0, 0x20, 0x20, 0x20, 0xC0 }, },
{'*', { 0xA8, 0x70, 0xF8, 0x70, 0xA8 }, },
{'+', { 0x00, 0x20, 0xF8, 0x20, 0x00 }, },
{',', { 0x00, 0x00, 0x00, 0xC0, 0x40 }, },
{'-', { 0x00, 0x00, 0xF8, 0x00, 0x00 }, },
{'.', { 0x00, 0x00, 0x00, 0x00, 0xC0 }, },
{'/', { 0x08, 0x10, 0x20, 0x40, 0x80 }, },
{'0', { 0x70, 0x98, 0xA8, 0xC8, 0x70 }, },
{'1', { 0x60, 0xA0, 0x20, 0x20, 0x20 }, },
{'2', { 0xE0, 0x10, 0x20, 0x40, 0xF8 }, },
{'3', { 0xF0, 0x08, 0x30, 0x08, 0xF0 }, },
{'4', { 0x90, 0x90, 0xF8, 0x10, 0x10 }, },
{'5', { 0xF8, 0x80, 0xF0, 0x08, 0xF0 }, },
{'6', { 0x70, 0x80, 0xF0, 0x88, 0x70 }, },
{'7', { 0xF8, 0x08, 0x10, 0x20, 0x40 }, },
{'8', { 0x70, 0x88, 0x70, 0x88, 0x70 }, },
{'9', { 0x70, 0x88, 0x78, 0x08, 0x70 }, },
{':', { 0x00, 0xC0, 0x00, 0xC0, 0x00 }, },
{';', { 0xC0, 0x00, 0xC0, 0x40, 0x80 }, },
{'<', { 0x08, 0x30, 0xC0, 0x30, 0x08 }, },
{'=', { 0x00, 0xF8, 0x00, 0xF8, 0x00 }, },
{'>', { 0x80, 0x60, 0x18, 0x60, 0x80 }, },
{'?', { 0xE0, 0x10, 0x20, 0x00, 0x20 }, },
{'@', { 0x70, 0x88, 0xB0, 0x80, 0x78 }, },
{'A', { 0x70, 0x88, 0xF8, 0x88, 0x88 }, },
{'B', { 0xF0, 0x48, 0x70, 0x48, 0xF0 }, },
{'C', { 0x78, 0x80, 0x80, 0x80, 0x78 }, },
{'D', { 0xF0, 0x88, 0x88, 0x88, 0xF0 }, },
{'E', { 0xF8, 0x80, 0xE0, 0x80, 0xF8 }, },
{'F', { 0xF8, 0x80, 0xE0, 0x80, 0x80 }, },
{'G', { 0x78, 0x80, 0x98, 0x88, 0x78 }, },
{'H', { 0x88, 0x88, 0xF8, 0x88, 0x88 }, },
{'I', { 0xE0, 0x40, 0x40, 0x40, 0xE0 }, },
{'J', { 0x08, 0x08, 0x08, 0x88, 0x70 }, },
{'K', { 0x88, 0x90, 0xE0, 0x90, 0x88 }, },
{'L', { 0x80, 0x80, 0x80, 0x80, 0xF8 }, },
{'M', { 0x88, 0xD8, 0xA8, 0x88, 0x88 }, },
{'N', { 0x88, 0xC8, 0xA8, 0x98, 0x88 }, },
{'O', { 0x70, 0x88, 0x88, 0x88, 0x70 }, },
{'P', { 0xF0, 0x88, 0xF0, 0x80, 0x80 }, },
{'Q', { 0x70, 0x88, 0x88, 0xA8, 0x70 }, },
{'R', { 0xF0, 0x88, 0xF0, 0x90, 0x88 }, },
{'S', { 0x78, 0x80, 0x70, 0x08, 0xF0 }, },
{'T', { 0xF8, 0x20, 0x20, 0x20, 0x20 }, },
{'U', { 0x88, 0x88, 0x88, 0x88, 0x70 }, },
{'V', { 0x88, 0x90, 0xA0, 0xC0, 0x80 }, },
{'W', { 0x88, 0x88, 0xA8, 0xA8, 0x50 }, },
{'X', { 0x88, 0x50, 0x20, 0x50, 0x88 }, },
{'Y', { 0x88, 0x50, 0x20, 0x20, 0x20 }, },
{'Z', { 0xF8, 0x10, 0x20, 0x40, 0xF8 }, }
};

// MASK for optimizing power in Waterfall ID signal

int  mask[1024] = {
0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x03, 0x00, 0x01, 0x02, 0x02, 0x04, 0x04, 0x06, 0x04,
0x00, 0x01, 0x02, 0x01, 0x00, 0x01, 0x06, 0x03, 0x00, 0x01, 0x02, 0x01, 0x04, 0x05, 0x0e, 0x02, 
0x00, 0x00, 0x02, 0x03, 0x04, 0x04, 0x04, 0x04, 0x00, 0x09, 0x02, 0x03, 0x04, 0x05, 0x0c, 0x01, 
0x00, 0x10, 0x12, 0x13, 0x04, 0x04, 0x14, 0x04, 0x18, 0x01, 0x1a, 0x02, 0x0c, 0x05, 0x08, 0x1d, 
0x00, 0x01, 0x02, 0x03, 0x04, 0x04, 0x06, 0x04, 0x00, 0x08, 0x0a, 0x03, 0x08, 0x0d, 0x08, 0x02, 
0x10, 0x01, 0x12, 0x02, 0x04, 0x05, 0x06, 0x16, 0x18, 0x10, 0x1a, 0x01, 0x18, 0x05, 0x02, 0x1b, 
0x20, 0x20, 0x02, 0x02, 0x04, 0x04, 0x04, 0x05, 0x28, 0x20, 0x02, 0x20, 0x08, 0x09, 0x04, 0x0d, 
0x10, 0x11, 0x22, 0x02, 0x04, 0x34, 0x02, 0x27, 0x38, 0x39, 0x20, 0x21, 0x14, 0x09, 0x2e, 0x28, 
0x00, 0x01, 0x02, 0x01, 0x00, 0x01, 0x04, 0x01, 0x08, 0x01, 0x08, 0x08, 0x0c, 0x04, 0x02, 0x0d, 
0x10, 0x11, 0x12, 0x01, 0x10, 0x01, 0x04, 0x01, 0x10, 0x11, 0x1a, 0x01, 0x10, 0x05, 0x0e, 0x1d, 
0x00, 0x21, 0x20, 0x22, 0x20, 0x25, 0x22, 0x01, 0x20, 0x09, 0x2a, 0x2a, 0x24, 0x20, 0x08, 0x09, 
0x10, 0x11, 0x10, 0x33, 0x04, 0x04, 0x34, 0x13, 0x38, 0x11, 0x20, 0x19, 0x18, 0x19, 0x06, 0x0c, 
0x40, 0x01, 0x40, 0x43, 0x04, 0x04, 0x40, 0x05, 0x40, 0x41, 0x42, 0x4b, 0x40, 0x4d, 0x0a, 0x41, 
0x50, 0x11, 0x02, 0x12, 0x44, 0x41, 0x40, 0x41, 0x08, 0x10, 0x12, 0x53, 0x40, 0x5c, 0x16, 0x4f, 
0x20, 0x61, 0x20, 0x01, 0x60, 0x25, 0x46, 0x05, 0x28, 0x28, 0x6a, 0x43, 0x40, 0x09, 0x42, 0x4c, 
0x70, 0x31, 0x70, 0x33, 0x60, 0x25, 0x12, 0x50, 0x58, 0x39, 0x2a, 0x09, 0x5c, 0x60, 0x0c, 0x0d, 
0x00, 0x01, 0x02, 0x02, 0x04, 0x01, 0x02, 0x06, 0x00, 0x09, 0x02, 0x08, 0x08, 0x09, 0x02, 0x0e, 
0x10, 0x10, 0x12, 0x02, 0x04, 0x01, 0x04, 0x14, 0x08, 0x19, 0x08, 0x13, 0x10, 0x10, 0x0e, 0x1d, 
0x20, 0x01, 0x02, 0x22, 0x20, 0x20, 0x22, 0x02, 0x28, 0x08, 0x2a, 0x29, 0x08, 0x0c, 0x0c, 0x0e, 
0x10, 0x30, 0x32, 0x10, 0x14, 0x34, 0x06, 0x36, 0x28, 0x01, 0x02, 0x1a, 0x0c, 0x2d, 0x1c, 0x1d, 
0x00, 0x41, 0x42, 0x41, 0x44, 0x04, 0x44, 0x06, 0x08, 0x09, 0x02, 0x41, 0x44, 0x48, 0x02, 0x06, 
0x40, 0x01, 0x12, 0x12, 0x10, 0x11, 0x54, 0x13, 0x58, 0x01, 0x50, 0x5a, 0x04, 0x19, 0x12, 0x57, 
0x60, 0x41, 0x40, 0x02, 0x20, 0x24, 0x02, 0x47, 0x08, 0x41, 0x08, 0x6a, 0x4c, 0x68, 0x6a, 0x47, 
0x10, 0x71, 0x42, 0x43, 0x10, 0x55, 0x26, 0x50, 0x10, 0x58, 0x72, 0x28, 0x2c, 0x39, 0x1a, 0x4f, 
0x80, 0x80, 0x82, 0x02, 0x04, 0x84, 0x02, 0x02, 0x88, 0x88, 0x08, 0x01, 0x08, 0x05, 0x04, 0x0d, 
0x10, 0x01, 0x12, 0x90, 0x84, 0x94, 0x02, 0x90, 0x10, 0x98, 0x9a, 0x98, 0x14, 0x9c, 0x16, 0x0b, 
0x20, 0x20, 0x80, 0xa3, 0x80, 0xa1, 0xa6, 0x87, 0x28, 0x29, 0xa8, 0xa8, 0x08, 0x09, 0x2a, 0x87, 
0x10, 0x80, 0x82, 0x11, 0xb0, 0xa5, 0x32, 0x32, 0xb8, 0xa0, 0x2a, 0x11, 0x9c, 0x81, 0x8e, 0xa4, 
0x40, 0xc0, 0xc2, 0x01, 0x40, 0xc5, 0x80, 0x05, 0xc0, 0x80, 0x4a, 0x09, 0x8c, 0x09, 0x0a, 0x09, 
0x50, 0x41, 0xc0, 0x41, 0x40, 0xc0, 0x12, 0x87, 0x80, 0xc0, 0x82, 0x83, 0x9c, 0xd8, 0x98, 0x16, 
0x20, 0x41, 0x82, 0x83, 0x80, 0xc0, 0x46, 0xc7, 0x48, 0xe0, 0x6a, 0x29, 0x4c, 0x29, 0xc0, 0x8e, 
0x40, 0xc0, 0x50, 0x21, 0x74, 0xf4, 0x32, 0x25, 0x28, 0x98, 0x9a, 0x32, 0x74, 0x58, 0x8e, 0x83, 
0x00, 0x00, 0x02, 0x01, 0x04, 0x05, 0x04, 0x06, 0x00, 0x09, 0x02, 0x02, 0x0c, 0x01, 0x06, 0x04, 
0x00, 0x10, 0x12, 0x13, 0x14, 0x14, 0x12, 0x16, 0x10, 0x19, 0x12, 0x01, 0x04, 0x18, 0x16, 0x17, 
0x20, 0x20, 0x22, 0x03, 0x04, 0x24, 0x24, 0x26, 0x20, 0x20, 0x20, 0x21, 0x0c, 0x2c, 0x2c, 0x24, 
0x20, 0x10, 0x30, 0x20, 0x04, 0x10, 0x04, 0x31, 0x10, 0x28, 0x08, 0x33, 0x0c, 0x05, 0x0c, 0x3d, 
0x40, 0x01, 0x40, 0x02, 0x04, 0x01, 0x44, 0x03, 0x40, 0x08, 0x0a, 0x08, 0x0c, 0x44, 0x4e, 0x44, 
0x10, 0x50, 0x40, 0x11, 0x54, 0x14, 0x06, 0x56, 0x58, 0x49, 0x52, 0x49, 0x18, 0x4d, 0x56, 0x49, 
0x20, 0x41, 0x02, 0x01, 0x44, 0x65, 0x64, 0x05, 0x20, 0x69, 0x6a, 0x23, 0x68, 0x0c, 0x0c, 0x27, 
0x70, 0x50, 0x72, 0x71, 0x54, 0x54, 0x14, 0x56, 0x40, 0x21, 0x22, 0x33, 0x44, 0x79, 0x2e, 0x57, 
0x00, 0x01, 0x80, 0x81, 0x84, 0x85, 0x84, 0x01, 0x88, 0x81, 0x8a, 0x8b, 0x80, 0x80, 0x02, 0x06, 
0x10, 0x90, 0x12, 0x80, 0x94, 0x80, 0x80, 0x11, 0x88, 0x09, 0x90, 0x19, 0x0c, 0x11, 0x06, 0x13, 
0xa0, 0x21, 0xa2, 0x21, 0xa4, 0x04, 0xa6, 0x24, 0x20, 0xa8, 0x8a, 0xa8, 0xa4, 0x85, 0x06, 0x06, 
0x10, 0x20, 0xb2, 0xa0, 0x10, 0xb4, 0x12, 0xb6, 0xb8, 0xa0, 0x18, 0x81, 0x24, 0x85, 0xb6, 0x0c, 
0xc0, 0xc1, 0xc0, 0x01, 0xc0, 0x44, 0x04, 0xc5, 0x08, 0xc0, 0x02, 0x0a, 0x4c, 0xc4, 0x8e, 0x03, 
0x10, 0xd0, 0x82, 0x90, 0x50, 0x11, 0xd4, 0x11, 0x98, 0x88, 0x12, 0x53, 0xd4, 0x81, 0xc4, 0xd4, 
0x20, 0x20, 0x62, 0xe2, 0x24, 0x85, 0xa6, 0xe5, 0x68, 0x69, 0xc0, 0xe9, 0xa0, 0xe9, 0x6c, 0x43, 
0x70, 0x71, 0x60, 0xf1, 0x94, 0x31, 0xf2, 0xc5, 0xf0, 0xc9, 0x8a, 0x79, 0x18, 0x25, 0x68, 0xf6, 
0x100, 0x01, 0x02, 0x100, 0x04, 0x01, 0x02, 0x05, 0x100, 0x08, 0x08, 0x100, 0x10c, 0x05, 0x04, 0x107, 
0x110, 0x11, 0x110, 0x12, 0x10, 0x105, 0x100, 0x12, 0x100, 0x119, 0x0a, 0x119, 0x11c, 0x14, 0x1a, 0x10f, 
0x20, 0x01, 0x02, 0x22, 0x24, 0x04, 0x24, 0x05, 0x100, 0x01, 0x120, 0x128, 0x28, 0x12c, 0x126, 0x2d, 
0x110, 0x10, 0x122, 0x130, 0x104, 0x14, 0x14, 0x117, 0x100, 0x09, 0x138, 0x39, 0x0c, 0x09, 0x12e, 0x13c, 
0x100, 0x40, 0x40, 0x02, 0x40, 0x141, 0x04, 0x107, 0x48, 0x108, 0x08, 0x140, 0x04, 0x140, 0x10e, 0x42, 
0x50, 0x141, 0x102, 0x52, 0x150, 0x114, 0x54, 0x45, 0x10, 0x101, 0x12, 0x153, 0x54, 0x101, 0x44, 0x5a, 
0x160, 0x161, 0x42, 0x101, 0x164, 0x105, 0x64, 0x65, 0x20, 0x121, 0x168, 0x10b, 0x10c, 0x10d, 0x120, 0x6a, 
0x100, 0x101, 0x22, 0x103, 0x54, 0x31, 0x14, 0x33, 0x158, 0x101, 0x50, 0x72, 0x74, 0x105, 0x7a, 0x150, 
0x80, 0x180, 0x82, 0x103, 0x184, 0x85, 0x100, 0x107, 0x80, 0x89, 0x8a, 0x09, 0x04, 0x101, 0x10e, 0x103, 
0x180, 0x90, 0x100, 0x11, 0x194, 0x190, 0x12, 0x107, 0x118, 0x11, 0x12, 0x09, 0x14, 0x09, 0x118, 0x11c, 
0x1a0, 0xa0, 0x1a2, 0x120, 0x1a4, 0x185, 0x126, 0x127, 0x1a0, 0x1a8, 0x8a, 0x123, 0x1a4, 0x18d, 0x2a, 0x22, 
0x80, 0x81, 0x1b0, 0x11, 0x94, 0x1b4, 0x32, 0x25, 0x120, 0xa0, 0x28, 0x8b, 0x120, 0x39, 0x12e, 0x13c, 
0x1c0, 0x1c0, 0x82, 0x41, 0x44, 0x180, 0x106, 0xc5, 0x100, 0x88, 0x180, 0x42, 0x1c4, 0xcd, 0xc4, 0x1c4, 
0x90, 0x90, 0x1c0, 0x103, 0xd4, 0x180, 0x110, 0x52, 0x50, 0x49, 0x52, 0x158, 0x180, 0xd5, 0x1c4, 0x4d, 
0x40, 0x101, 0x102, 0x21, 0x44, 0xc5, 0x1e4, 0xe2, 0x88, 0x1a1, 0x42, 0x22, 0x24, 0xac, 0x1a0, 0x127, 
0x50, 0x41, 0x112, 0xf2, 0x180, 0xc0, 0x1d6, 0x1b0, 0x50, 0x51, 0x32, 0x160, 0x74, 0x1bc, 0x1b0, 0x16f 
};

mfntchr idch2[] = {
{' ', { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'!', { 0x0000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0x0000, 0x0000, 0xC000, 0xC000, 0x0000, 0x0000, 0x0000 }, },
{'"', { 0x0000, 0xD800, 0xD800, 0xD800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'#', { 0x0000, 0x5000, 0x5000, 0xF800, 0xF800, 0x5000, 0x5000, 0xF800, 0xF800, 0x5000, 0x5000, 0x0000, 0x0000, 0x0000 }, },
{'$', { 0x0000, 0x2000, 0x2000, 0x7800, 0xF800, 0xA000, 0xF000, 0x7800, 0x2800, 0xF800, 0xF000, 0x2000, 0x2000, 0x0000 }, },
{'%', { 0x0000, 0x4000, 0xE400, 0xE400, 0x4C00, 0x1800, 0x3000, 0x6000, 0xC800, 0x9C00, 0x9C00, 0x8800, 0x0000, 0x0000 }, },
{'&', { 0x0000, 0x3000, 0x7800, 0x4800, 0x4800, 0x7000, 0xF400, 0x8C00, 0x8800, 0xFC00, 0x7400, 0x0000, 0x0000, 0x0000 }, },
{ 39, { 0x0000, 0x4000, 0x4000, 0xC000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'(', { 0x0000, 0x0000, 0x2000, 0x6000, 0xC000, 0x8000, 0x8000, 0x8000, 0x8000, 0xC000, 0x6000, 0x2000, 0x0000, 0x0000 }, },
{')', { 0x0000, 0x0000, 0x8000, 0xC000, 0x6000, 0x2000, 0x2000, 0x2000, 0x2000, 0x6000, 0xC000, 0x8000, 0x0000, 0x0000 }, },
{'*', { 0x0000, 0x0000, 0x0000, 0x1000, 0x1000, 0xFE00, 0x7C00, 0x3800, 0x6C00, 0x4400, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'+', { 0x0000, 0x0000, 0x0000, 0x2000, 0x2000, 0x2000, 0xF800, 0xF800, 0x2000, 0x2000, 0x2000, 0x0000, 0x0000, 0x0000 }, },
{',', { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xC000, 0xC000, 0xC000, 0x4000, 0xC000, 0x8000 }, },
{'-', { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0x0000, 0x0000 }, },
{'.', { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xC000, 0xC000, 0xC000, 0x0000, 0x0000 }, },
{'/', { 0x0000, 0x0800, 0x0800, 0x1800, 0x1000, 0x3000, 0x2000, 0x6000, 0x4000, 0xC000, 0x8000, 0x8000, 0x0000, 0x0000 }, },
{'0', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8C00, 0x9C00, 0xB400, 0xE400, 0xC400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'1', { 0x0000, 0x0000, 0x1000, 0x3000, 0x7000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0000 }, },
{'2', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x0C00, 0x1800, 0x3000, 0x6000, 0xC000, 0xFC00, 0xFC00, 0x0000, 0x0000 }, },
{'3', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0400, 0x0C00, 0x1800, 0x1C00, 0x0400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'4', { 0x0000, 0x0000, 0x3800, 0x7800, 0x4800, 0xC800, 0x8800, 0xFC00, 0xFC00, 0x0800, 0x0800, 0x0800, 0x0000, 0x0000 }, },
{'5', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x8000, 0x8000, 0xF800, 0xFC00, 0x0400, 0x0400, 0xFC00, 0xF800, 0x0000, 0x0000 }, },
{'6', { 0x0000, 0x0000, 0x7800, 0xF800, 0x8000, 0x8000, 0xF800, 0xFC00, 0x8400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'7', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0400, 0x0400, 0x0C00, 0x1800, 0x3000, 0x2000, 0x2000, 0x2000, 0x0000, 0x0000 }, },
{'8', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x8400, 0x7800, 0xFC00, 0x8400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'9', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x8400, 0xFC00, 0x7C00, 0x0400, 0x0400, 0x7C00, 0x7800, 0x0000, 0x0000 }, },
{':', { 0x0000, 0xC000, 0xC000, 0xC000, 0x0000, 0x0000, 0x0000, 0xC000, 0xC000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{';', { 0x0000, 0x6000, 0x6000, 0x6000, 0x0000, 0x0000, 0x6000, 0x6000, 0x2000, 0x2000, 0xE000, 0xC000, 0x0000, 0x0000 }, },
{'<', { 0x0000, 0x0000, 0x0800, 0x1800, 0x3000, 0x6000, 0xC000, 0xC000, 0x6000, 0x3000, 0x1800, 0x0800, 0x0000, 0x0000 }, },
{'=', { 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0x0000, 0x0000, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'>', { 0x0000, 0x0000, 0x8000, 0xC000, 0x6000, 0x3000, 0x1800, 0x1800, 0x3000, 0x6000, 0xC000, 0x8000, 0x0000, 0x0000 }, },
{'?', { 0x0000, 0x0000, 0x7000, 0xF800, 0x8800, 0x0800, 0x1800, 0x3000, 0x2000, 0x0000, 0x2000, 0x2000, 0x0000, 0x0000 }, },
{'@', { 0x0000, 0x0000, 0x7C00, 0xFE00, 0x8200, 0x8200, 0xB200, 0xBE00, 0xBC00, 0x8000, 0xFC00, 0x7C00, 0x0000, 0x0000 }, },
{'A', { 0x0000, 0x0000, 0x3000, 0x7800, 0xCC00, 0x8400, 0x8400, 0xFC00, 0xFC00, 0x8400, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'B', { 0x0000, 0x0000, 0xF800, 0xFC00, 0x8400, 0x8400, 0xF800, 0xF800, 0x8400, 0x8400, 0xFC00, 0xF800, 0x0000, 0x0000 }, },
{'C', { 0x0000, 0x0000, 0x3800, 0x7C00, 0xC400, 0x8000, 0x8000, 0x8000, 0x8000, 0xC400, 0x7C00, 0x3800, 0x0000, 0x0000 }, },
{'D', { 0x0000, 0x0000, 0xF000, 0xF800, 0x8C00, 0x8400, 0x8400, 0x8400, 0x8400, 0x8C00, 0xF800, 0xF000, 0x0000, 0x0000 }, },
{'E', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x8000, 0x8000, 0xF000, 0xF000, 0x8000, 0x8000, 0xFC00, 0xFC00, 0x0000, 0x0000 }, },
{'F', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x8000, 0x8000, 0xF000, 0xF000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000 }, },
{'G', { 0x0000, 0x0000, 0x3C00, 0x7C00, 0xC000, 0x8000, 0x8C00, 0x8C00, 0x8400, 0xC400, 0x7C00, 0x3800, 0x0000, 0x0000 }, },
{'H', { 0x0000, 0x0000, 0x8400, 0x8400, 0x8400, 0x8400, 0xFC00, 0xFC00, 0x8400, 0x8400, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'I', { 0x0000, 0x0000, 0xF800, 0xF800, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0xF800, 0xF800, 0x0000, 0x0000 }, },
{'J', { 0x0000, 0x0000, 0x0400, 0x0400, 0x0400, 0x0400, 0x0400, 0x0400, 0x8400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'K', { 0x0000, 0x0000, 0x8400, 0x8400, 0x8C00, 0x9800, 0xF000, 0xF000, 0x9800, 0x8C00, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'L', { 0x0000, 0x0000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0xFC00, 0xFC00, 0x0000, 0x0000 }, },
{'M', { 0x0000, 0x0000, 0x8200, 0xC600, 0xEE00, 0xBA00, 0x9200, 0x8200, 0x8200, 0x8200, 0x8200, 0x8200, 0x0000, 0x0000 }, },
{'N', { 0x0000, 0x0000, 0x8400, 0xC400, 0xE400, 0xB400, 0x9C00, 0x8C00, 0x8400, 0x8400, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'O', { 0x0000, 0x0000, 0x3000, 0x7800, 0xCC00, 0x8400, 0x8400, 0x8400, 0x8400, 0xCC00, 0x7800, 0x3000, 0x0000, 0x0000 }, },
{'P', { 0x0000, 0x0000, 0xF800, 0xFC00, 0x8400, 0x8400, 0xFC00, 0xF800, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000 }, },
{'Q', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x8400, 0x8400, 0x8400, 0x9400, 0x9400, 0xFC00, 0x7800, 0x0800, 0x0800 }, },
{'R', { 0x0000, 0x0000, 0xF800, 0xFC00, 0x8400, 0x8400, 0xFC00, 0xF800, 0x8800, 0x8C00, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'S', { 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x8000, 0xF800, 0x7C00, 0x0400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'T', { 0x0000, 0x0000, 0xF800, 0xF800, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x0000, 0x0000 }, },
{'U', { 0x0000, 0x0000, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'V', { 0x0000, 0x0000, 0x8200, 0x8200, 0x8200, 0xC600, 0x4400, 0x6C00, 0x2800, 0x3800, 0x1000, 0x1000, 0x0000, 0x0000 }, },
{'W', { 0x0000, 0x0000, 0x8200, 0x8200, 0x8200, 0x8200, 0x8200, 0x9200, 0x9200, 0x9200, 0xFE00, 0x6C00, 0x0000, 0x0000 }, },
{'X', { 0x0000, 0x0000, 0x8200, 0x8200, 0xC600, 0x6C00, 0x3800, 0x3800, 0x6C00, 0xC600, 0x8200, 0x8200, 0x0000, 0x0000 }, },
{'Y', { 0x0000, 0x0000, 0x8200, 0x8200, 0xC600, 0x6C00, 0x3800, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x0000, 0x0000 }, },
{'Z', { 0x0000, 0x0000, 0xFC00, 0xFC00, 0x0C00, 0x1800, 0x3000, 0x6000, 0xC000, 0x8000, 0xFC00, 0xFC00, 0x0000, 0x0000 }, },
{'[', { 0x0000, 0x0000, 0xE000, 0xE000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0xE000, 0xE000, 0x0000, 0x0000 }, },
{'\\', { 0x0000, 0x8000, 0x8000, 0xC000, 0x4000, 0x6000, 0x2000, 0x3000, 0x1000, 0x1800, 0x0800, 0x0800, 0x0000, 0x0000 }, },
{']', { 0x0000, 0x0000, 0xE000, 0xE000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0xE000, 0xE000, 0x0000, 0x0000 }, },
{'^', { 0x0000, 0x2000, 0x2000, 0x7000, 0x5000, 0xD800, 0x8800, 0x8800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'_', { 0x0000, 0xF800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0x0000, 0x0000 }, },
{'`', { 0x0000, 0xC000, 0xC000, 0xC000, 0xC000, 0x6000, 0x6000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, },
{'a', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0x7C00, 0x0400, 0x7C00, 0xFC00, 0x8400, 0xFC00, 0x7C00, 0x0000, 0x0000 }, },
{'b', { 0x0000, 0x0000, 0x8000, 0x8000, 0xB800, 0xFC00, 0xC400, 0x8400, 0x8400, 0x8400, 0xFC00, 0xF800, 0x0000, 0x0000 }, },
{'c', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0xF800, 0x8000, 0x8000, 0x8000, 0x8000, 0xF800, 0x7800, 0x0000, 0x0000 }, },
{'d', { 0x0000, 0x0000, 0x0400, 0x0400, 0x7400, 0xFC00, 0x8C00, 0x8400, 0x8400, 0x8400, 0xFC00, 0x7C00, 0x0000, 0x0000 }, },
{'e', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0xFC00, 0xFC00, 0x8000, 0xF800, 0x7800, 0x0000, 0x0000 }, },
{'f', { 0x0000, 0x0000, 0x3C00, 0x7C00, 0x4000, 0x4000, 0xF800, 0xF800, 0x4000, 0x4000, 0x4000, 0x4000, 0x0000, 0x0000 }, },
{'g', { 0x0000, 0x0000, 0x0000, 0x7C00, 0xFC00, 0x8400, 0x8400, 0x8C00, 0xFC00, 0x7400, 0x0400, 0x7C00, 0x7800, 0x0000 }, },
{'h', { 0x0000, 0x0000, 0x8000, 0x8000, 0xB800, 0xFC00, 0xC400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'i', { 0x0000, 0x2000, 0x2000, 0x0000, 0xE000, 0xE000, 0x2000, 0x2000, 0x2000, 0x2000, 0xF800, 0xF800, 0x0000, 0x0000 }, },
{'j', { 0x0000, 0x0800, 0x0800, 0x0000, 0x3800, 0x3800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x8800, 0xF800, 0x7000 }, },
{'k', { 0x0000, 0x0000, 0x8000, 0x8800, 0x9800, 0xB000, 0xE000, 0xE000, 0xB000, 0x9800, 0x8800, 0x8800, 0x0000, 0x0000 }, },
{'l', { 0x0000, 0x0000, 0xE000, 0xE000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000, 0xF800, 0xF800, 0x0000, 0x0000 }, },
{'m', { 0x0000, 0x0000, 0x0000, 0x0000, 0xEC00, 0xFE00, 0x9200, 0x9200, 0x8200, 0x8200, 0x8200, 0x8200, 0x0000, 0x0000 }, },
{'n', { 0x0000, 0x0000, 0x0000, 0x0000, 0xB800, 0xFC00, 0xC400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x0000, 0x0000 }, },
{'o', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7800, 0xFC00, 0x8400, 0x8400, 0x8400, 0x8400, 0xFC00, 0x7800, 0x0000, 0x0000 }, },
{'p', { 0x0000, 0x0000, 0x0000, 0x0000, 0xF800, 0xFC00, 0x8400, 0x8400, 0xC400, 0xFC00, 0xB800, 0x8000, 0x8000, 0x8000 }, },
{'q', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7C00, 0xFC00, 0x8400, 0x8400, 0x8C00, 0xFC00, 0x7400, 0x0400, 0x0400, 0x0400 }, },
{'r', { 0x0000, 0x0000, 0x0000, 0x0000, 0xB800, 0xFC00, 0xC400, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000 }, },
{'s', { 0x0000, 0x0000, 0x0000, 0x0000, 0x7C00, 0xFC00, 0x8000, 0xF800, 0x7C00, 0x0400, 0xFC00, 0xF800, 0x0000, 0x0000 }, },
{'t', { 0x0000, 0x0000, 0x4000, 0x4000, 0xF000, 0xF000, 0x4000, 0x4000, 0x4000, 0x4000, 0x7800, 0x3800, 0x0000, 0x0000 }, },
{'u', { 0x0000, 0x0000, 0x0000, 0x0000, 0x8400, 0x8400, 0x8400, 0x8400, 0x8400, 0x8C00, 0xFC00, 0x7400, 0x0000, 0x0000 }, },
{'v', { 0x0000, 0x0000, 0x0000, 0x0000, 0x8200, 0x8200, 0x8200, 0x8200, 0xC600, 0x6C00, 0x3800, 0x1000, 0x0000, 0x0000 }, },
{'w', { 0x0000, 0x0000, 0x0000, 0x0000, 0x8200, 0x8200, 0x8200, 0x9200, 0x9200, 0x9200, 0xFE00, 0x6C00, 0x0000, 0x0000 }, },
{'x', { 0x0000, 0x0000, 0x0000, 0x0000, 0x8200, 0xC600, 0x6C00, 0x3800, 0x3800, 0x6C00, 0xC600, 0x8200, 0x0000, 0x0000 }, },
{'y', { 0x0000, 0x0000, 0x0000, 0x0000, 0x8400, 0x8400, 0x8400, 0x8400, 0x8C00, 0xFC00, 0x7400, 0x0400, 0x7C00, 0x7800 }, },
{'z', { 0x0000, 0x0000, 0x0000, 0x0000, 0xFC00, 0xFC00, 0x1800, 0x3000, 0x6000, 0xC000, 0xFC00, 0xFC00, 0x0000, 0x0000 }, },
{'{', { 0x0000, 0x2000, 0x6000, 0x4000, 0x4000, 0x4000, 0xC000, 0xC000, 0x4000, 0x4000, 0x4000, 0x6000, 0x2000, 0x0000 }, },
{'|', { 0x0000, 0x8000, 0x8000, 0xC000, 0x4000, 0x6000, 0x2000, 0x3000, 0x1000, 0x1800, 0x0800, 0x0800, 0x0000, 0x0000 }, },
{'}', { 0x0000, 0x8000, 0xC000, 0x4000, 0x4000, 0x4000, 0x6000, 0x6000, 0x4000, 0x4000, 0x4000, 0xC000, 0x8000, 0x0000 }, },
{'~', { 0x0000, 0x9800, 0xFC00, 0x6400, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }, }
};


