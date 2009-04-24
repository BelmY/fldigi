/*
 *    rigMEM.cxx  --  rigMEM (rig control) interface
 *
 */

#include <config.h>

#if !defined(__CYGWIN__) && !defined(__APPLE__)
#  include <sys/ipc.h>
#  include <sys/msg.h>
#  include <sys/shm.h>
#endif
#include <sys/time.h>

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "threads.h"
#include "qrunner.h"
#include "misc.h"
#include "configuration.h"

#include "rigMEM.h"
#include "fl_digi.h"
#include "main.h"
#include "rigsupport.h"
#include "rigdialog.h"

#include "debug.h"

LOG_SET_SOURCE(debug::LOG_RIGCONTROL);

/* ---------------------------------------------------------------------- */

#define PTT_OFF false
#define PTT_ON  true

static pthread_t rigMEM_thread;
static bool rigMEM_exit = false;
static bool rigMEM_enabled;

static void *rigMEM_loop(void *args);

static bool rigMEM_PTT = PTT_OFF;
static bool rig_qsy = false;
static bool TogglePTT = false;
static bool rigMEMisPTT = false;
static long long qsy_f;
static long long qsy_fmid;

#if !defined(__CYGWIN__) && !defined(__APPLE__)
// Linux & *BSD interface to Kachina

struct ST_SHMEM {
	int  flag;
	long freq;
	long midfreq;
}sharedmem;


struct ST_SHMEM *freqflag;
struct ST_SHMEM noshare;

void *shared_memory = (void *)0;
int  shmid = -1;

key_t hash (char *str)
{
	key_t key = 1;
	unsigned int i;
	for (i = 0; i < strlen(str); i++)
		key *= str[i];
	return key;
}

bool rigMEM_init(void)
{
	rigMEM_enabled = false;
	noshare.freq = 7070000L;
	noshare.midfreq = 1000L;
	noshare.flag = 2;
	freqflag = &noshare;

	shmid = shmget ((key_t)1234, sizeof(sharedmem), 0666 | IPC_CREAT);

	if (shmid < 0) {
		LOG_ERROR("shmget failed");
		 return false;
	}
 	shared_memory = shmat (shmid, (void *)0, 0);
 	if (shared_memory == (void *)-1) {
		LOG_ERROR("shmat failed");
		return false;
 	}
	freqflag = (struct ST_SHMEM *) shared_memory;

	rig_qsy = false;
	rigMEM_PTT = PTT_OFF;
	TogglePTT = false;
	rigMEMisPTT = false;
	
	if (pthread_create(&rigMEM_thread, NULL, rigMEM_loop, NULL) < 0) {
		LOG_ERROR("rigMEM init: pthread_create failed");
		return false;
	} 

	rigMEM_enabled = true;
	
	init_rigMEM_RigDialog();
		
	return true;
}

void rigMEM_close(void)
{
	if (!rigMEM_enabled) return;

// tell the rigMEM thread to kill it self
	rigMEM_exit = true;

// and then wait for it to die
	pthread_join(rigMEM_thread, NULL);
	rigMEM_enabled = false;
	rigMEM_exit = false;

	wf->rfcarrier(0L);
	wf->USB(true);

}

static void *rigMEM_loop(void *args)
{
	SET_THREAD_ID(RIGCTL_TID);

	int sb = true;

	freqflag->freq = 0L;
	freqflag->flag = -1;

	for (;;) {
	/* see if we are being canceled */
		if (rigMEM_exit)
			break;

//		MilliSleep(20);

		if (rig_qsy) {
			freqflag->freq = qsy_f;
			freqflag->flag = -2; // set frequency
			MilliSleep(20);
			if (qsy_fmid > 0) {
				if (active_modem->freqlocked() == true) {
					active_modem->set_freqlock(false);
					active_modem->set_freq((int)qsy_fmid);
					active_modem->set_freqlock(true);
				} else
					active_modem->set_freq((int)qsy_fmid);
				wf->carrier((int)qsy_fmid);
				wf->rfcarrier(freqflag->freq);
				wf->movetocenter();
			} else
				wf->rfcarrier(freqflag->freq);
			rig_qsy = false;
		} else if (TogglePTT) {
			if (rigMEM_PTT == PTT_ON)
				freqflag->flag = -3;
			else
				freqflag->flag = -4;
			TogglePTT = false;
			MilliSleep(20);
		}// else
//			MilliSleep(20);

		if (freqflag->flag >= 0) {
			freqflag->flag = -1; // read frequency
			sb = false;
		}
	/* delay for 20 msec interval */
		MilliSleep(20);

	/* if rigMEM control program running it will update the freqflag structure every 10 msec */
		if (freqflag->flag > -1) {
			if ((freqflag->flag & 0x10) == 0x10)
				rigMEMisPTT = true;
			if ((freqflag->flag & 0x0F) == 2)
				sb = false;
			else
				sb = true;
			
			if (wf->rfcarrier() != freqflag->freq)
				wf->rfcarrier(freqflag->freq);
			if (FreqDisp->value() != freqflag->freq)
				FreqDisp->value(freqflag->freq);
			if (qsoFreqDisp->value() != freqflag->freq)
				qsoFreqDisp->value(freqflag->freq);

			if (wf->USB() != sb)
				wf->USB(sb);
			if (sb && (strcmp(opMODE->value(), "USB") || strcmp(qso_opMODE->value(),"USB"))) {
				REQ(&Fl_ComboBox::put_value, opMODE, "USB");
				REQ(&Fl_ComboBox::put_value, qso_opMODE, "USB");
			}
			if (!sb && (strcmp(opMODE->value(), "LSB") || strcmp(qso_opMODE->value(),"LSB"))) {
				REQ(&Fl_ComboBox::put_value, opMODE, "LSB");
				REQ(&Fl_ComboBox::put_value, qso_opMODE, "LSB");
			}
		}
	}

	if (shmdt(shared_memory) == -1) {
		fl_message("shmdt failed");
	}

	shmid = -1;

	/* this will exit the rigMEM thread */
	return NULL;
}

#elif defined(__CYGWIN__) // __CYGWIN__
//===================================================
// Windows interface to Kachina / rigCAT
//

FILE *IOout;
FILE *IOin;
int  IOflag;
long IOfreq = 7070000L;
long IOmidfreq = 1000L;
char szmode[80];

bool rigMEM_init(void) 
{ 
	rigMEM_enabled = false;

	rig_qsy = false;
	rigMEM_PTT = PTT_OFF;
	TogglePTT = false;
	rigMEMisPTT = false;
	
	if (pthread_create(&rigMEM_thread, NULL, rigMEM_loop, NULL) < 0) {
		fl_message("rigMEM init: pthread_create failed");
		return false;
	} 
	rigMEM_enabled = true;
	return true;
}

void rigMEM_close(void)
{
	if (!rigMEM_enabled) return;

// delete the ptt control file so Kachina can work stand alone
	remove("c:/RIGCTL/ptt");

// tell the rigMEM thread to kill it self
	rigMEM_exit = true;

// and then wait for it to die
	pthread_join(rigMEM_thread, NULL);
	LOG_DEBUG("rigMEM down");
	rigMEM_enabled = false;
	rigMEM_exit = false;

	wf->rfcarrier(0L);
	wf->USB(true);

}
static void *rigMEM_loop(void *args)
{
	SET_THREAD_ID(RIGCTL_TID);

	int sb = true;

	for (;;) {
	/* see if we are being canceled */
		if (rigMEM_exit)
			break;

//		if (rig_qsy) {
//			freqflag->freq = qsy_f;
//			freqflag->flag = -2; // set frequency
//			MilliSleep(20);
//			if (active_modem->freqlocked() == true) {
//				active_modem->set_freqlock(false);
//				active_modem->set_freq((int)qsy_fmid);
//				active_modem->set_freqlock(true);
//			} else
//				active_modem->set_freq((int)qsy_fmid);
//			wf->carrier((int)qsy_fmid);
//			wf->rfcarrier(freqflag->freq);
//			wf->movetocenter();
//			rig_qsy = false;
//		} else if (TogglePTT) {
		if (TogglePTT) {
			IOout = fopen("c:/RIGCTL/ptt", "w");
			if (IOout) {
				if (rigMEM_PTT == PTT_ON)
					putc('X', IOout);
				else
					putc('R', IOout);
				fclose(IOin);
				TogglePTT = false;
			}
		}
		
		IOin = fopen("c:/RIGCTL/rig", "r");
		if (IOin) {
			fscanf(IOin, "%ld\n", &IOfreq);
			fscanf(IOin, "%s", szmode);
			fclose(IOin);
		}
		if (strcmp(szmode,"LSB") == 0)
			sb = false;
			
		if (wf->rfcarrier() != IOfreq) {
			wf->rfcarrier(IOfreq);
			FreqDisp->value(IOfreq);
			qsoFreqDisp->value(IOfreq);
		}
		if (wf->USB() != sb) {
			wf->USB(sb);
			if (sb) {
				REQ(&Fl_ComboBox::put_value, opMODE, "USB");
				REQ(&Fl_ComboBox::put_value, qso_opMODE, "USB");
			} else  {
				REQ(&Fl_ComboBox::put_value, opMODE, "LSB");
				REQ(&Fl_ComboBox::put_value, qso_opMODE, "LSB");
			}
		}

// delay for 50 msec interval
		MilliSleep(50);

	}

	/* this will exit the rigMEM thread */
	return NULL;
}

#elif defined(__APPLE__)
// FIXME: we should be using an IPC mechanism that works on all Unix variants,
// or not compile rigMEM at all on OS X.

bool rigMEM_init(void) { return false; }
void rigMEM_close(void) { }
static void *rigMEM_loop(void *args) { return NULL; }

#endif

bool rigMEM_active(void)
{
	return (rigMEM_enabled);
}

bool rigMEM_CanPTT(void)
{
	return rigMEMisPTT;
}
	
void setrigMEM_PTT (bool on)
{
  rigMEM_PTT = on;
  TogglePTT = true;
}

void rigMEM_set_qsy(long long f, long long fmid)
{
	if (!rigMEM_enabled)
		return;

	qsy_f = f;
	qsy_fmid = fmid;
	rig_qsy = true;
}

void rigMEM_set_freq(long long f)
{
	if (!rigMEM_enabled)
		return;

	qsy_f = f;
	qsy_fmid = 0;
	rig_qsy = true;

}

void rigMEM_setmode(const string& s)
{
	return; // not implemented for memMAP
}

/* ---------------------------------------------------------------------- */

