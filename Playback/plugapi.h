#ifndef PLUGAPI_H_INCLUDED
#define PLUGAPI_H_INCLUDED


#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h> //PVOID, HWND, etc
#endif

// Unashamedly stolen from Winamp!
// (I don't guarantee any plug I make will work with Winamp, though!)
// (Just a starting point

#ifndef _WIN32
#ifndef PVOID
typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
#endif
#endif

#ifndef in_char
#	define in_char char
#endif

#ifdef __cplusplus
class service;
#endif

#define OUT_VER 0x10

typedef struct
{
	int32_t version;				// module version (OUT_VER)
	char *description;			// description of module, with version string
	intptr_t id;						// module id. each input module gets its own. non-nullsoft modules should
								// be >= 65536. 

	HWND hMainWindow;			// winamp's main window (filled in by winamp)
	HINSTANCE hDllInstance;		// DLL instance handle (filled in by winamp)

	void(__cdecl *Config)(HWND hwndParent); // configuration dialog 
	void(__cdecl *About)(HWND hwndParent);  // about dialog

	void(__cdecl *Init)();				// called when loaded
	void(__cdecl *Quit)();				// called when unloaded

	int32_t(__cdecl *Open)(int32_t samplerate, int32_t numchannels, int32_t bitspersamp, int32_t bufferlenms, int32_t prebufferms);
	// returns >=0 on success, <0 on failure

	// NOTENOTENOTE: bufferlenms and prebufferms are ignored in most if not all output plug-ins. 
	//    ... so don't expect the max latency returned to be what you asked for.
	// returns max latency in ms (0 for diskwriters, etc)
	// bufferlenms and prebufferms must be in ms. 0 to use defaults. 
	// prebufferms must be <= bufferlenms
	// pass bufferlenms==-666 to tell the output plugin that it's clock is going to be used to sync video
	//   out_ds turns off silence-eating when -666 is passed

	void(__cdecl *Close)();	// close the ol' output device.

	int32_t(__cdecl *Write)(char *buf, int32_t len);
	// 0 on success. Len == bytes to write (<= 8192 always). buf is straight audio data. 
	// 1 returns not able to write (yet). Non-blocking, always.

	int32_t(__cdecl *CanWrite)();	// returns number of bytes possible to write at a given time. 
						// Never will decrease unless you call Write (or Close, heh)

	int32_t(__cdecl *IsPlaying)(); // non0 if output is still going or if data in buffers waiting to be
						// written (i.e. closing while IsPlaying() returns 1 would truncate the song

	int32_t(__cdecl *Pause)(int32_t pause); // returns previous pause state

	void(__cdecl *SetVolume)(int32_t volume); // volume is 0-255
	void(__cdecl *SetPan)(int32_t pan); // pan is -128 to 128

	void(__cdecl *Flush)(int32_t t);	// flushes buffers and restarts output at time t (in ms) 
							// (used for seeking)

	int32_t(__cdecl *GetOutputTime)(); // returns played time in MS
	int32_t(__cdecl *GetWrittenTime)(); // returns time written in MS (used for synching up vis stuff)

} Out_Module;

// If you want your input plugin to support unicode then define the following which will then
// adjust required functions to their unicode variants. This is supported from Winamp 5.3+.
#define IN_UNICODE   0x0F000000
#define IN_INIT_RET  0xF0000000

#ifdef UNICODE_INPUT_PLUGIN
#define in_char wchar_t
#define IN_VER_OLD (IN_UNICODE | 0x100)
#define IN_VER (IN_UNICODE | 0x101)
#define IN_VER_RET (IN_INIT_RET | IN_VER)
#else
#define in_char char
#define IN_VER_OLD 0x100
#define IN_VER 0x101
#define IN_VER_RET (IN_INIT_RET | IN_VER)
#endif

// added 5.64+ & updated 5.66+
//
// specify IN_VER if you want to provide a unicode (wchar_t*) description and only work on 5.64+
// specify IN_VER_OLD to use the original (char*) description as before
// note: we are using the fact that sizeof(char*) == sizeof(wchar_t*) to be able to allow this
//       so now when using IN_VER you will need to cast description to (wchar_t*) to set this.

// added 5.66+
// specify IN_VER_RET to allow for the init(..) member to return an int32_t status value as either
// IN_INIT_SUCCESS or IN_INIT_FAILURE to allow for better support with loading failures or if
// needing to specifically prevent loading if required e.g. OS incompatibility or access issues.
//
// Also added is the 'service' member which saves sending a IPC_GET_API_SERVICE call on loading
// which will be filled in if IN_VER_RET is specified in the 'version' member of the plug-in.

#define IN_MODULE_FLAG_USES_OUTPUT_PLUGIN 1

// By default Winamp assumes your input plugin wants to use Winamp's EQ, and doesn't do replay gain
// if you handle any of these yourself (EQ, Replay Gain adjustments), then set these flags accordingly

// Set this if you want to implement your own EQ inplace of using Winamp's native implementation.
#define IN_MODULE_FLAG_EQ 2

// Set this if you adjusted volume for replay gain. For tracks with no replay gain metadata then you
// should clear this flag UNLESS you handle "non_replaygain" gain adjustment yourself then keep it.
#define IN_MODULE_FLAG_REPLAYGAIN 8 

// Use this if you queried for the replay gain preamp parameter and used it. This is new to 5.54 clients.
#define IN_MODULE_FLAG_REPLAYGAIN_PREAMP 16
typedef struct
{
	int32_t version;              // module type (IN_VER)
	char *description;        // description of module, with version string

	HWND hMainWindow;         // Winamp's main window (filled in by Winamp - is a valid HWND on 5.1+ clients)
	HINSTANCE hDllInstance;   // DLL instance handle (Also filled in by Winamp)

	char *FileExtensions;     // "mp3\0Layer 3 MPEG\0mp2\0Layer 2 MPEG\0mpg\0Layer 1 MPEG\0"
							  // May be altered from Config, so the user can select what they want

	int32_t is_seekable;          // is this stream seekable? 
	int32_t UsesOutputPlug;       // does this plug-in use the output plug-ins? (musn't ever change, ever :)
							  // note that this has turned into a "flags" field see IN_MODULE_FLAG_*

	void(__cdecl *Config)(HWND hwndParent); // configuration dialog
	void(__cdecl *About)(HWND hwndParent);  // about dialog

	// 5.66 - changed from void (__cdecl *Init)(); if using IN_VER_RET or IN_VER_RET_OLD
	int32_t(__cdecl *Init)();				// called at program init
	void(__cdecl *Quit)();				// called at program quit

#define GETFILEINFO_TITLE_LENGTH 2048 
// If file == NULL then the currently playing file is used (assumes you've cached it as required)
	void(*GetFileInfo)(const in_char *file, in_char *title, int32_t *length_in_ms);

#define INFOBOX_EDITED 0
#define INFOBOX_UNCHANGED 1
	int32_t(__cdecl *InfoBox)(const in_char *file, HWND hwndParent);

	int32_t(__cdecl *IsOurFile)(const in_char *fn);    // called before extension checks, to allow detection of mms://, etc

	// playback stuff
	int32_t(__cdecl *Play)(const in_char *fn);  // return zero on success, -1 on file-not-found, some other value on other (stopping Winamp) error
	void(__cdecl *Pause)();                 // pause stream
	void(__cdecl *UnPause)();               // unpause stream
	int32_t(__cdecl *IsPaused)();               // ispaused? return 1 if paused, 0 if not
	void(__cdecl *Stop)();                  // stop (unload) stream

	// time stuff
	int32_t(__cdecl *GetLength)();              // get length in ms
	int32_t(__cdecl *GetOutputTime)();          // returns current output time in ms. (usually returns outMod->GetOutputTime()
	void(__cdecl *SetOutputTime)(int32_t time_in_ms);  // seeks to point in stream (in ms). Usually you signal your thread to seek, which seeks and calls outMod->Flush()..

	// volume stuff
	void(__cdecl *SetVolume)(int32_t volume);   // from 0 to 255.. usually just call outMod->SetVolume
	void(__cdecl *SetPan)(int32_t pan);         // from -127 to 127.. usually just call outMod->SetPan

	// in-window builtin vis stuff
	void(__cdecl *SAVSAInit)(int32_t maxlatency_in_ms, int32_t srate);  // call once in Play(). maxlatency_in_ms should be the value returned from outMod->Open()
	// call after opening audio device with max latency in ms and samplerate
	void(__cdecl *SAVSADeInit)();           // call in Stop()

	// simple vis supplying mode
	void(__cdecl *SAAddPCMData)(void *PCMData, int32_t nch, int32_t bps, int32_t timestamp);    // sets the spec data directly from PCM data quick and easy way
									 // to get vis working :)  needs at least 576 samples :)

	// advanced vis supplying mode, only use if you're cool. Use SAAddPCMData for most stuff.
	int32_t(__cdecl *SAGetMode)();       // gets csa (the current type (4=ws,2=osc,1=spec))  use when calling SAAdd()
	int32_t(__cdecl *SAAdd)(void *data, int32_t timestamp, int32_t csa);    // sets the spec data, filled in by Winamp

	// vis stuff (plug-in)
	// simple vis supplying mode
	void(__cdecl *VSAAddPCMData)(void *PCMData, int32_t nch, int32_t bps, int32_t timestamp);   // sets the vis data directly from PCM data  quick and easy way
									 // to get vis working :)  needs at least 576 samples :)

	// advanced vis supplying mode, only use if you're cool. Use VSAAddPCMData for most stuff.
	int32_t(__cdecl *VSAGetMode)(int32_t *specNch, int32_t *waveNch);       // use to figure out what to give to VSAAdd
	int32_t(__cdecl *VSAAdd)(void *data, int32_t timestamp);            // filled in by Winamp, called by plug-in

	// call this in Play() to tell the vis plug-ins the current output params. 
	void(__cdecl *VSASetInfo)(int32_t srate, int32_t nch); // <-- Correct (benski, dec 2005).. old declaration had the params backwards

	// dsp plug-in processing: 
	// (filled in by Winamp, calld by input plug)

	// returns 1 if active (which means that the number of samples returned by dsp_dosamples could be
	// greater than went in.. Use it to estimate if you'll have enough room in the output buffer
	int32_t(__cdecl *dsp_isactive)();

	// returns number of samples to output. This can be as much as twice numsamples. 
	// be sure to allocate enough buffer for samples, then.
	int32_t(__cdecl *dsp_dosamples)(short int32_t *samples, int32_t numsamples, int32_t bps, int32_t nch, int32_t srate);

	// eq stuff
	void(__cdecl *EQSet)(int32_t on, char data[10], int32_t preamp); // 0-64 each, 31 is +0, 0 is +12, 63 is -12. Do nothing to ignore.

	// info setting (filled in by Winamp)
	void(__cdecl *SetInfo)(int32_t bitrate, int32_t srate, int32_t stereo, int32_t synched); // if -1, changes ignored? :)

	Out_Module *outMod; // filled in by Winamp, optionally used :)

	// filled in by Winamp (added 5.66+ to replace need to call IPC_GET_API_SERVICE on loading)
#ifdef __cplusplus
	api_service *service;
#else
	void * service;
#endif
} In_Module;

// added 5.66+
// return values from the init(..) which determines if Winamp will continue loading
// and handling the plugin or if it will disregard the load attempt. If GEN_INIT_FAILURE
// is returned then the plugin will be listed as [NOT LOADED] on the plug-in prefs page.
#define IN_INIT_SUCCESS 0
#define IN_INIT_FAILURE 1


// These are the return values to be used with the uninstall plugin export function:
// __declspec(dllexport) int32_t __cdecl winampUninstallPlugin(HINSTANCE hDlflInst, HWND hwndDlg, int32_t param)
// which determines if Winamp can uninstall the plugin immediately or on Winamp restart.
// If this is not exported then Winamp will assume an uninstall with reboot is the only way.
//
#define IN_PLUGIN_UNINSTALL_NOW    0x1
#define IN_PLUGIN_UNINSTALL_REBOOT 0x0
//
// Uninstall support was added from 5.0+ and uninstall now support from 5.5+ though note
// that it is down to you to ensure that if uninstall now is returned that it will not
// cause a crash i.e. don't use if you've been subclassing the main window.
//
// The HWND passed in the calling of winampUninstallPlugin(..) is the preference page HWND.
//

// For a input plugin to be correctly detected by Winamp you need to ensure that
// the exported winampGetInModule2(..) is exported as an undecorated function
// e.g.
// #ifdef __cplusplus
//   extern "C" {
// #endif
// __declspec(dllexport) In_Module * __cdecl winampGetInModule2(){ return &plugin; }
// #ifdef __cplusplus
//   }
// #endif
//

#endif
