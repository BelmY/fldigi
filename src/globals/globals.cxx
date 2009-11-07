// ----------------------------------------------------------------------------
// globals.cxx  --  constants, variables, arrays & functions that need to be
//                  outside of any thread
//
// Copyright (C) 2006
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.  Adapted in part from code contained in gmfsk 
// source code distribution.
//
// fldigi is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ----------------------------------------------------------------------------

#include <config.h>
#include <iosfwd>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cstdio>

#include "globals.h"
#include "modem.h"

using namespace std;

// Elements are in enum trx_mode order. Mode name video-id uses the
// first string (sname), so its length should be a multiple of 2.
const struct mode_info_t mode_info[NUM_MODES] = {
    { MODE_CW, &cw_modem, "CW", "CW", "CW", "CW" }, 

	{ MODE_DOMINOEX4, &dominoex4_modem, "DomEX4", "DominoEX 4", "DOMINOEX4", "DOMINO" },
	{ MODE_DOMINOEX5, &dominoex5_modem, "DomEX5", "DominoEX 5", "DOMINOEX5", "DOMINO" },
	{ MODE_DOMINOEX8, &dominoex8_modem, "DomEX8", "DominoEX 8", "DOMINOEX8", "DOMINO" },
	{ MODE_DOMINOEX11, &dominoex11_modem, "DomX11", "DominoEX 11", "DOMINOEX11", "DOMINO" },
	{ MODE_DOMINOEX16, &dominoex16_modem, "DomX16", "DominoEX 16", "DOMINOEX16", "DOMINO" },
	{ MODE_DOMINOEX22, &dominoex22_modem, "DomX22", "DominoEX 22", "DOMINOEX22", "DOMINO" },

	{ MODE_FELDHELL, &feld_modem, "FELDHELL", "Feld Hell", "", "HELL" },
	{ MODE_SLOWHELL, &feld_slowmodem, "SLOWHELL", "Slow Hell", "", "HELL" }, // Not sure about these after the first one
	{ MODE_HELLX5, &feld_x5modem, "HELLX5", "Feld Hell X5", "", "HELL" },
	{ MODE_HELLX9, &feld_x9modem, "HELLX9", "Feld Hell X9", "", "HELL"},	
	{ MODE_FSKHELL, &feld_FMmodem, "FSK-HELL", "FSK Hell", "", "FMHELL" },
	{ MODE_FSKH105, &feld_FM105modem, "FSK-H105", "FSK Hell-105", "", "FMHELL" },
	{ MODE_HELL80, &feld_80modem, "HELL80", "Hell 80", "", "HELL80" },

	{ MODE_MFSK8, &mfsk8_modem, "MFSK-8", "MFSK-8", "MFSK8", "MFSK8" },
	{ MODE_MFSK16, &mfsk16_modem, "MFSK16", "MFSK-16", "MFSK16", "MFSK16" },
	{ MODE_MFSK32, &mfsk32_modem, "MFSK-32", "MFSK-32", "MFSK32", "MFSK32" },

// experimental modes
	{ MODE_MFSK4, &mfsk4_modem, "MFSK-4", "MFSK-4", "MFSK4", "MFSK4" },
	{ MODE_MFSK11, &mfsk11_modem, "MFSK-11", "MFSK-11", "MFSK11", "MFSK11" },
	{ MODE_MFSK22, &mfsk22_modem, "MFSK-22", "MFSK-22", "MFSK22", "MFSK22" },
	{ MODE_MFSK31, &mfsk31_modem, "MFSK-31", "MFSK-31", "MFSK31", "MFSK31" },
	{ MODE_MFSK64, &mfsk64_modem, "MFSK-64", "MFSK-64", "MFSK64", "MFSK64" },

	{ MODE_MT63_500, &mt63_500_modem, "MT63-500", "MT63-500", "MT63-500", "MT63" },
	{ MODE_MT63_1000, &mt63_1000_modem, "MT63-1XX", "MT63-1000", "MT63-1XX", "MT63" },
	{ MODE_MT63_2000, &mt63_2000_modem, "MT63-2XX", "MT63-2000", "MT63-2XX", "MT63" },

	{ MODE_BPSK31, &psk31_modem, "BPSK31", "BPSK-31", "PSK31", "PSK31" },
	{ MODE_QPSK31, &qpsk31_modem, "QPSK31", "QPSK-31", "QPSK31", "QPSK31" },
	{ MODE_PSK63, &psk63_modem, "BPSK63", "BPSK-63", "PSK63", "PSK63" },
	{ MODE_QPSK63, &qpsk63_modem, "QPSK63", "QPSK-63", "QPSK63", "QPSK63" },
	{ MODE_PSK125, &psk125_modem, "BPSK125", "BPSK-125", "PSK125", "PSK125" },
	{ MODE_QPSK125, &qpsk125_modem, "QPSK125", "QPSK-125", "QPSK125", "QPSK125" },
	{ MODE_PSK250, &psk250_modem, "BPSK250", "BPSK-250", "PSK250", "PSK250" },
	{ MODE_QPSK250, &qpsk250_modem, "QPSK250", "QPSK-250", "QPSK250", "QPSK250" },

	{ MODE_OLIVIA, &olivia_modem, "OLIVIA", "Olivia", "", "OLIVIA" },

	{ MODE_RTTY, &rtty_modem, "RTTY", "RTTY", "RTTY", "RTTY" },

	{ MODE_THOR4, &thor4_modem, "THOR4", "THOR 4", "THOR4", "THOR" },
	{ MODE_THOR5, &thor5_modem, "THOR5", "THOR 5", "THOR5", "THOR" },
	{ MODE_THOR8, &thor8_modem, "THOR8", "THOR 8", "THOR8", "THOR" },
	{ MODE_THOR11, &thor11_modem, "THOR11", "THOR 11", "THOR11", "THOR" },
//	{ MODE_TSOR11, &tsor11_modem, "TSOR11", "TSOR 11", "TSOR11", "TSOR" },
	{ MODE_THOR16, &thor16_modem, "THOR16", "THOR 16", "THOR16", "THOR" },
	{ MODE_THOR22, &thor22_modem, "THOR22", "THOR 22", "THOR22", "THOR" },

	{ MODE_THROB1, &throb1_modem, "THROB1", "Throb 1", "", "THRB" },
	{ MODE_THROB2, &throb2_modem, "THROB2", "Throb 2", "", "THRB" },
	{ MODE_THROB4, &throb4_modem, "THROB4", "Throb 4", "", "THRB" },
	{ MODE_THROBX1, &throbx1_modem, "THRBX1", "ThrobX 1", "", "THRBX" },
	{ MODE_THROBX2, &throbx2_modem, "THRBX2", "ThrobX 2", "", "THRBX" },
	{ MODE_THROBX4, &throbx4_modem, "THRBX4", "ThrobX 4", "", "THRBX" },

	{ MODE_WWV, &wwv_modem, "WWV", "WWV", "", "" },

	{ MODE_ANALYSIS, &anal_modem, "ANALYSIS", "Freq Analysis", "", "" }
};

std::ostream& operator<<(std::ostream& s, const qrg_mode_t& m)
{
	return s << m.rfcarrier << ' ' << m.rmode << ' ' << m.carrier << ' ' << mode_info[m.mode].sname;
}

std::istream& operator>>(std::istream& s, qrg_mode_t& m)
{
	string sMode;
	int mnbr;
	s >> m.rfcarrier >> m.rmode >> m.carrier >> sMode;
// handle case for reading older type of specification string
	if (sscanf(sMode.c_str(), "%d", &mnbr)) {
		m.mode = mnbr;
		return s;
	}
	m.mode = MODE_BPSK31;
	for (mnbr = MODE_CW; mnbr < NUM_MODES; mnbr++)
		if (sMode == mode_info[mnbr].sname) {
			m.mode = mnbr;
			break;
		}
	return s;
}

std::string qrg_mode_t::str(void)
{
	ostringstream s;
	s << setiosflags(ios::left | ios::fixed)
	  << setw(12) << setprecision(3) << rfcarrier/1000.0
	  << setw(8) << rmode
	  << setw(10) << (mode < NUM_MODES ? mode_info[mode].sname : "NONE")
	  << carrier;
	return s.str();
}


band_t band(long long freq_hz)
{
	switch (freq_hz / 1000000LL) {
		case 0: case 1: return BAND_160M;
		case 3: return BAND_80M;
		case 4: return BAND_75M;
		case 5: return BAND_60M;
		case 7: return BAND_40M;
		case 10: return BAND_30M;
		case 14: return BAND_20M;
		case 18: return BAND_17M;
		case 21: return BAND_15M;
		case 24: return BAND_12M;
		case 28 ... 29: return BAND_10M;
		case 50 ... 54: return BAND_6M;
		case 70 ... 71: return BAND_4M;
		case 144 ... 148: return BAND_2M;
		case 222 ... 225: return BAND_125CM;
		case 420 ... 450: return BAND_70CM;
		case 902 ... 928: return BAND_33CM;
		case 1240 ... 1325: return BAND_23CM;
		case 2300 ... 2450: return BAND_13CM;
		case 3300 ... 3500: return BAND_9CM;
		case 5650 ... 5925: return BAND_6CM;
		case 10000 ... 10500: return BAND_3CM;
		case 24000 ... 24250: return BAND_125MM;
		case 47000 ... 47200: return BAND_6MM;
		case 75500 ... 81000: return BAND_4MM;
		case 119980 ... 120020: return BAND_2P5MM;
		case 142000 ... 149000: return BAND_2MM;
		case 241000 ... 250000: return BAND_1MM;
	}

	return BAND_OTHER;
}

band_t band(const char* freq_mhz)
{
	errno = 0;
	double d = strtod(freq_mhz, NULL);
	if (d != 0.0 && errno == 0)
		return band((long long)(d * 1e6));
	else
		return BAND_OTHER;
}

struct band_freq_t {
	const char* band;
	const char* freq;
};

static struct band_freq_t band_names[NUM_BANDS] = {
	{ "160m", "1.8" },
	{ "80m", "3.4" },
	{ "75m", "4.0" },
	{ "60m", "5.3" },
	{ "40m", "7.0" },
	{ "30m", "10.0" },
	{ "20m", "14.0" },
	{ "17m", "18.0" },
	{ "15m", "21.0" },
	{ "12m", "24.0" },
	{ "10m", "28.0" },
	{ "6m", "50.0" },
	{ "4m", "70.0" },
	{ "2m", "144.0" },
	{ "1.25m", "222.0" },
	{ "70cm", "420.0" },
	{ "33cm", "902.0" },
	{ "23cm", "1240.0" },
	{ "13cm", "2300.0" },
	{ "9cm", "3300.0" },
	{ "6cm", "5650.0" },
	{ "3cm", "10000.0" },
	{ "1.25cm", "24000.0" },
	{ "6mm", "47000.0" },
	{ "4mm", "75500.0" },
	{ "2.5mm", "119980.0" },
	{ "2mm", "142000.0" },
	{ "1mm", "241000.0" },
	{ "other", "" }
};

const char* band_name(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].band;
}

const char* band_name(const char* freq_mhz)
{
	return band_name(band(freq_mhz));
}

const char* band_freq(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].freq;
}

const char* band_freq(const char* band_name)
{
	for (size_t i = 0; i < BAND_OTHER; i++)
		if (!strcmp(band_names[i].band, band_name))
			return band_names[i].freq;

	return "";
}
