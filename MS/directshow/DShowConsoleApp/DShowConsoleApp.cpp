// DShowConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <comdef.h>
#include <initguid.h>
#include "steve_shoutcast.h"
#include <time.h>
#include <atomic>
#include <Strsafe.h>
#include <sstream>

static int g_done = 0;
#define PLEASE_QUIT 1
#define ALL_DONE -1


void display_time(int totalSeconds, std::string& s)
{
	totalSeconds += 86400 * 30 * 6;
	int seconds = (totalSeconds % 60);
	int minutes = (totalSeconds % 3600) / 60;
	int hours = (totalSeconds % 86400) / 3600;
	int days = (totalSeconds / 86400);
	
	static char buf[256];
	sprintf(buf, "%02d:%02d:%02d:%02d", days, hours, minutes, seconds);
	s.assign(buf);
	return;
}

std::string now(std::string format = "%d/%m/%Y %R")
{
	time_t rawtime;
	struct tm *info;
	static char buffer[80];

	time(&rawtime);

	info = localtime(&rawtime);

	strftime(buffer, 80, format.c_str(), info);
	std::string ret(buffer);
	return ret;
}



BOOL WINAPI ConsoleHandler(DWORD dwType)
{

	int slept = 0;
	switch (dwType) {
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:

		// set_done();//signal the main thread to terminate

				   //Returning would make the process exit!
				   //We just make the handler sleep until the main thread exits,
				   //or until the maximum execution time for this handler is reached.

		g_done = PLEASE_QUIT;
		while (g_done != ALL_DONE) {
			Sleep(1000);
			slept += 1000;
			if (slept > 10000) {
				return TRUE;
			}
		}


		return TRUE;
	default:
		break;
	}
	return FALSE;
}


template <typename UNK>
struct comvec : public std::vector<UNK*>
{
	~comvec() {
		for (size_t i = 0; i < size(); ++i) {
			UNK** p = &operator[](i);
			if (*p) {
				(*p)->Release();
			}
			p = NULL;
		}
	}
};

struct cominit
{
	cominit() {
		CoInitialize(NULL);
	}

	~cominit() {
		CoUninitialize();
	}
};

void print_err(HRESULT h) {
	if (FAILED(h)) {
		_com_error err(h);
		LPCTSTR errMsg = err.ErrorMessage();
		std::stringstream ss;
		ss << std::hex << h;
		std::cerr << errMsg << " : " << "(" << h << ") " << ss.str() <<  std::endl;
	}
}

#ifndef HR
#define HR(h) print_err(h);  ASSERT(SUCCEEDED(h));
#endif

typedef std::vector <std::string> strvec_t;
using namespace std;


strvec_t renderers()
{
	strvec_t ret;
	// Create the System Device Enumerator.
	HRESULT hr = S_OK;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);

	HR(hr);

	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					_bstr_t bstr(varName.bstrVal);
					const char* p = bstr;
					ret.push_back(p);
				}
				VariantClear(&varName);

				/*/
				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pFilter);
				// Now add the filter to the graph.
				//Remember to release pFilter later.
				/*/
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return ret;
}

HRESULT AddFilterByCLSID(
	IGraphBuilder *pGraph,      // Pointer to the Filter Graph Manager.
	REFGUID clsid,              // CLSID of the filter to create.
	IBaseFilter **ppF,          // Receives a pointer to the filter.
	LPCWSTR wszName             // A name for the filter (can be NULL).
)
{
	*ppF = 0;

	IBaseFilter *pFilter = NULL;

	HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFilter));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->AddFilter(pFilter, wszName);
	if (FAILED(hr))
	{
		goto done;
	}

	*ppF = pFilter;
	(*ppF)->AddRef();

done:
	if (pFilter) pFilter->Release();
	return hr;
}
/// 
/// Create a filter by category and name. Will enumerate all filters 
/// of the given category and return the filter whose name matches, 
/// if any. 
/// 
/// @param Name of filter to create. 
/// @param Filter Will receive the pointer to the interface 
/// for the created filter. 
/// @param FilterCategory Filter category. 
/// 
/// @return true if successful. 
bool CreateFilter(const WCHAR *Name, IBaseFilter **Filter,
	REFCLSID FilterCategory)
{
	ASSERT(Name);
	ASSERT(Filter);
	ASSERT(!*Filter);

	HRESULT hr;

	// Create the system device enumerator. 
	CComPtr<ICreateDevEnum> devenum;
	hr = devenum.CoCreateInstance(CLSID_SystemDeviceEnum);
	if (FAILED(hr))
		return false;

	// Create an enumerator for this category. 
	CComPtr<IEnumMoniker> classenum;
	hr = devenum->CreateClassEnumerator(FilterCategory, &classenum, 0);
	if (hr != S_OK)
		return false;

	// Find the filter that matches the name given. 
	CComVariant name(Name);
	CComPtr<IMoniker> moniker;
	while (classenum->Next(1, &moniker, 0) == S_OK)
	{
		CComPtr<IPropertyBag> properties;
		hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&properties);
		if (FAILED(hr))
			return false;

		CComVariant friendlyname;
		hr = properties->Read(L"FriendlyName", &friendlyname, 0);
		if (FAILED(hr))
			return false;

		if (name == friendlyname)
		{
			hr = moniker->BindToObject(0, 0, IID_IBaseFilter, (void **)Filter);
			return SUCCEEDED(hr);
		}

		moniker.Release();
	}

	// Couldn't find a matching filter. 
	return false;
}

/*/
As an example of how to use these functions, this code snippet instantiates the Microsoft MPEG4 V2 compressor and adds it to the filter graph:

CComPtr compressor;
AddFilter(filtergraph, L"Microsoft MPEG-4 Video Codec V2", &compressor,
CLSID_VideoCompressorCategory, L"VideoCompressor");
/*/

/// 
/// Create a filter by category and name, and add it to a filter 
/// graph. Will enumerate all filters of the given category and 
/// add the filter whose name matches, if any. If the filter could be 
/// created but not added to the graph, the filter is destroyed. 
/// 
/// @param Graph Filter graph. 
/// @param Name of filter to create. 
/// @param Filter Receives a pointer to the filter. 
/// @param FilterCategory Filter category. 
/// @param NameInGraph Name for the filter in the graph, or 0 for no 
/// name. 
/// 
/// @return true if successful. 
bool AddFilter(IFilterGraph *Graph, const WCHAR *Name,
	IBaseFilter **Filter, REFCLSID FilterCategory,
	const WCHAR *NameInGraph)
{
	ASSERT(Graph);
	ASSERT(Name);
	ASSERT(Filter);
	ASSERT(!*Filter);

	if (!CreateFilter(Name, Filter, FilterCategory))
		return false;

	if (FAILED(Graph->AddFilter(*Filter, NameInGraph)))
	{
		(*Filter)->Release();
		*Filter = 0;
		return false;
	}

	return true;
}

typedef comvec<IPin> pinvec_t;

HRESULT GetPins(IBaseFilter* pFilter, pinvec_t& pins)
{
	HRESULT hr = S_OK;
	CComPtr<IEnumPins> pEnum;
	hr = pFilter->EnumPins(&pEnum);
	HR(hr);
	IPin* pin = NULL; ULONG cFetched = 0;


	while (hr == S_OK) {
		hr = pEnum->Next(1, &pin, &cFetched);
		if (hr == S_OK) {
			pins.push_back(pin);
		}
	}
	return S_OK;
}

HRESULT GetPin(const pinvec_t& vec, const PIN_DIRECTION direction, IPin** pPin)
{
	ASSERT(pPin && *pPin == NULL);
	for (auto pin : vec) {
		PIN_DIRECTION dir = direction;
		HRESULT hr = pin->QueryDirection(&dir);
		if (FAILED(hr)) return hr;
		if (dir == direction) {
			*pPin = pin;
			(*pPin)->AddRef();
			break;
		}
	}
	if (*pPin) return S_OK;
	return E_FAIL;
}

HRESULT findSteveShoutcast(IGraphBuilder* pGraph, IShoutcastFilter** ppRet)
{
	CComPtr<IEnumFilters> pEnum;
	HRESULT hr = pGraph->EnumFilters(&pEnum);
	HR(hr);

	hr = E_FAIL;
	while (FAILED(hr))
	{
		ULONG Fetched = 0;
		IBaseFilter* pf = NULL;
		hr = pEnum->Next(1, &pf, &Fetched);
		if (hr == S_OK) {
			pf->Release();
			hr = pf->QueryInterface(ppRet);
			if (FAILED(hr)) {
				continue;
			}
			else {
				return S_OK;
			}
		}

	}

	return E_FAIL;

}

struct shout_callback : public IShoutcastCallback
{

public:
	shout_callback() {
		m_cRef = 1;
	}
	virtual HRESULT STDMETHODCALLTYPE MetaChangeCallback(BSTR sArtistTitle,
		BSTR sURL,
		BSTR sFullMeta)
	{
		_bstr_t bstr(sArtistTitle);
		const std::string st = "StreamTitle=";
		std::string s(bstr);
		size_t pos = s.find(st);
		if (pos != std::string::npos) {
			s = s.substr(st.length());
		}
		const std::string semi = ";";
		pos = s.find(semi);
		size_t l = s.length();
		if (pos == l - 1) {
			s = s.substr(0, l - 1);
		}
		cout << now() << " " << s << endl;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE StreamBuffering(
		/* [in] */ int BufferPercentFull) {
		static int last = 0;
		if (BufferPercentFull > 0) {
			cout << "Buffering ... " << BufferPercentFull << "%" << endl;
		}
		else {
			if (last > 0) {
				cout << "Buffering complete." << endl;
				last = 0;
			}
		}
		last = BufferPercentFull;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OdStreamSeg(
		double seg) {

		return S_OK;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
		if (riid == IID_IUnknown) {
			AddRef();
			*ppv = (IUnknown*)this;
			return S_OK;
		}

		if (riid == IID_IShoutcastCallback) {
			*ppv = (IShoutcastCallback*)this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	};
	STDMETHODIMP_(ULONG) AddRef() {
		m_cRef++;
		return m_cRef;
	};
	STDMETHODIMP_(ULONG) Release() {
		m_cRef--;
		if (m_cRef == 0) {
			delete this;
		}
		return m_cRef;
	};

private:
	DWORD m_cRef;

};


HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker = NULL;
	IRunningObjectTable *pROT = NULL;

	if (FAILED(GetRunningObjectTable(0, &pROT)))
	{
		return E_FAIL;
	}

	const size_t STRING_LENGTH = 256;

	WCHAR wsz[STRING_LENGTH];

	StringCchPrintfW(
		wsz, STRING_LENGTH,
		L"FilterGraph %08x pid %08x",
		(DWORD_PTR)pUnkGraph,
		GetCurrentProcessId()
	);

	HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr))
	{
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pMoniker, pdwRegister);
		pMoniker->Release();
	}
	pROT->Release();

	return hr;
}


int app(int argc, char** argv) {
	try
	{
		BOOL ret = SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		std::string filepath = "icystream://listen.shoutcast.com/80splanet";

		cominit c;
		strvec_t args(argv, argv + argc);
		int devnum = -1;
		auto rends = renderers();
		if (args.size() == 1) {
			// args.push_back("1");
		}
		if (args.size() == 1) {
			cout << "First arg should be file name, can be internet or local. If internet, try icystream:// instead of http://" << endl;
			cout << "Second argument (optional) the number of the output device from the following list: " << endl << endl;

			int i = 0;
			for (const auto& s : rends)
			{
				cout << i << "  " << s << endl;
				i++;
			}
			cout << endl;
		}
		else {
			if (args.size() >= 3) {
				try {
					devnum = std::stoi(args[2]);
					cout << "You specified device number: " << devnum << endl;
					if ((size_t)devnum < rends.size()) {
						cout << "Device name for device " << devnum << " is " << rends[devnum] << endl;
					}
					else {
						cerr << "WARNING: device number is out of range" << endl;
					}
				}
				catch (...)
				{
					devnum = -1;
				}

			}

			if (args.size() >= 2) {
				filepath = args[1];
				cout << "filename you wanted: " << filepath << endl;
			}
		}


		CComPtr<IGraphBuilder> pGraph;
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
			CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);

		HR(hr);

		CComPtr<IFilterGraph2> pfg;
		hr = pGraph->QueryInterface(&pfg);
		HR(hr);

		cout << "Adding MPEG Decoder ..." << endl;
		CComPtr<IBaseFilter> mpegFilter;
		hr = AddFilterByCLSID(pGraph, CLSID_CMpegAudioCodec, &mpegFilter, NULL);
		HR(hr);

		cout << "Geting MPEG Decoder pins ..." << endl;
		CComPtr<IPin> mpegInPin; CComPtr<IPin> mpegOutPin;
		comvec<IPin> pins;
		hr = GetPins(mpegFilter, pins);
		HR(hr);


		cout << "Getting MPEG Decoder in pin ..." << endl;
		hr = GetPin(pins, PINDIR_INPUT, &mpegInPin);
		HR(hr);

		cout << "Getting MPEG Decoder out pin ..." << endl;
		hr = GetPin(pins, PINDIR_OUTPUT, &mpegOutPin);
		HR(hr);


		CComPtr<IBaseFilter> renderer;

		if (devnum >= 0 && devnum < (int)rends.size())
		{

			_bstr_t bstrname(rends[devnum].c_str());
			cout << "Adding desired renderer: " << rends[devnum] << endl;
			bool b = AddFilter(pfg, bstrname, &renderer,
				CLSID_AudioRendererCategory, bstrname);
			if (!b) {
				cerr << "WARNING: could not add renderer: " << rends[devnum] << endl;
			}
			else {
				cout << "Successfully added renderer: " << rends[devnum] << endl;
			}
		}
		else {
			bool b = AddFilter(pfg, L"Default DirectSound Device", &renderer,
				CLSID_AudioRendererCategory, L"Default DirectSound Device");
			if (!renderer) {
				cerr << "WARNING: cannot create Default device. No sound cards, perhaps?" << endl;;
				if (rends.size() == 0) {
					cerr << "Cannot continue. No renderers found." << endl;
					return -77;
				}
				else {
					cout << "No default renderer, trying the first device, which is: " << endl;
					int idx = 0;
					if (rends[idx] == "AudioEnhanceDSRender Filter") { // skip the DSRender filter, it blows us up
						if (rends.size() >= 1) {
							idx = 1;
						}
					}
					cout << rends[idx] << " ..." << endl;
					_bstr_t bstrname(rends[idx].c_str());
					bool b = AddFilter(pfg, bstrname, &renderer,
						CLSID_AudioRendererCategory, bstrname);
					if (!renderer) {
						cerr << "Sorry. I really tried, but I cannot seem to find a suitable renderer" << endl;
						return -88;
					}
				}
			}
		}

		IPin* prenderpin = NULL;
		bool graph_built = false;
		if (renderer) {
			cout << "Getting renderer in pin ... " << endl;
			comvec<IPin> vecscpins;
			hr = GetPins(renderer, vecscpins);
			HR(hr);
			prenderpin = vecscpins.at(0);
			ASSERT(prenderpin);

			cout << "Got renderer in pin ... " << endl;
		}
		else {
			cout << "WARNING: no renderer filter" << endl;
		}

		CComPtr<IBaseFilter> pWM;
		if (filepath.find("http://") == 0 && prenderpin) {
			bool b = AddFilter(pfg, L"WM ASF Reader", &pWM, CLSID_LegacyAmFilterCategory, L"WM ASF Reader");
			ASSERT(b);
			if (b) {
				CComPtr<IFileSourceFilter> psrc;
				hr = pWM->QueryInterface(&psrc);
				HR(hr);

				_bstr_t wfile(filepath.c_str());
				cout << "Loading URL: " << filepath << " ..." << endl;
				hr = psrc->Load(wfile, NULL);
				HR(hr);
				cout << "Loaded " << filepath << endl;
				comvec<IPin> pins;
				hr = GetPins(pWM, pins);
				HR(hr);
				IPin* pwmpin = pins[0];

				hr = pGraph->ConnectDirect(pwmpin, prenderpin, NULL);
				if (FAILED(hr)) {
					hr = pGraph->Connect(pwmpin, prenderpin);
				}
				HR(hr);
				graph_built = true;
			}
		}

		CComPtr<IBaseFilter> pSteve;
		if (filepath.find("icystream://") == 0 && prenderpin) {
			bool b = AddFilter(pfg, L"_DShowShoutcast", &pSteve, CLSID_LegacyAmFilterCategory, L"DShow SHOUTcast");
			ASSERT(b);
			if (b) {
				CComPtr<IFileSourceFilter> psrc;
				hr = pSteve->QueryInterface(&psrc);
				HR(hr);

				_bstr_t wfile(filepath.c_str());
				cout << "Loading URL: " << filepath << " ..." << endl;
				hr = psrc->Load(wfile, NULL);
				if (FAILED(hr)) {
					cerr << "failed to load " << filepath << endl;
					print_err(hr);
					return -777;
				}
				HR(hr);
				cout << "Loaded " << filepath << endl;
				comvec<IPin> pins;
				hr = GetPins(pSteve, pins);
				HR(hr);
				IPin* ppin = pins[0];

				cout << "Trying ConnectDirect from DShowShoutcast source to renderer ..." << endl;
				hr = pGraph->ConnectDirect(ppin, prenderpin, NULL);
				if (FAILED(hr)) {
					cout << "WARNING: Trying Connect (NOT DIRECT) from DShowShoutcast source to renderer ..." << endl;
					hr = pGraph->Connect(ppin, prenderpin);
					if (FAILED(hr)) {
						cerr << "Connect (NOT Direct) from DShowShoutcast source to renderer failed, with error ..." << endl;
						print_err(hr);
						return -888;
					}
				}
				HR(hr);
				graph_built = true;
			}
		}
		_bstr_t bstrfilepath(filepath.c_str());

		cout << "Calling RenderFile ... " << endl;
		if (!graph_built) {
			hr = pGraph->RenderFile(bstrfilepath, NULL);
			if (FAILED(hr)) {
				cout << "RenderFile() gave error: ";
				print_err(hr);
				return -888;
			}
			HR(hr);
		}
		cout << "Looking for Shoutcast filter ..." << endl;

		CComPtr<IShoutcastFilter> psc;
		hr = findSteveShoutcast(pGraph, &psc);
		// just ignore if fails
		shout_callback* myCallback = NULL;
		if (SUCCEEDED(hr))
		{
			cout << "Found Shoutcast filter, creating callback ..." << endl;
			myCallback = new shout_callback;

		}

		if (myCallback) {
			hr = psc->putref_CallbackInterface(myCallback);
			HR(hr);
		}

		CComPtr<IMediaControl> pControl;
		hr = pGraph->QueryInterface(&pControl);
		HR(hr);
		hr = pControl->Run();
		HR(hr);

		cout << "Got control interface." << endl;
		CComPtr<IMediaPosition> pPos;
		hr = pGraph->QueryInterface(&pPos);
		HR(hr);

		// DWORD dwrot = 0;
		// HRESULT ffs = AddToRot(pGraph, &dwrot);
		// we essentially play forever
		std::string stime;

		while (true)
		{
			double pos = 0;
			double dur = 0;
			// NOTE: http:// streams do not support duration!

			if (g_done) break;

			hr = pPos->get_Duration(&dur);
			if (FAILED(hr)) {
				hr = pPos->get_StopTime(&dur);
			}
			if (SUCCEEDED(hr)) {
				cout << "Duration is: " << dur << endl << endl;
			}

			CComPtr<IMediaEventEx> pEvent;
			hr = pGraph->QueryInterface(&pEvent);
			HR(hr);
			long code = 0;
			bool set_cb = false; // set it again, else no meta callback

			while (true) {
				hr = pEvent->WaitForCompletion(500, &code);

				if (hr == S_OK) {
					cout << "\nEvent code was: " << code;
					break;
				}
				if (hr == E_ABORT) {
					// Time-out expired.
					if (g_done == PLEASE_QUIT) {
						pControl->Stop();
						break;
					}
					if (true) {
						double curpos = 0;
						HRESULT myhr = pPos->get_CurrentPosition(&curpos);
						if (SUCCEEDED(myhr)) {
							// cout << "Elapsed: " << curpos << "\r";
							curpos += 2592000;
							display_time((int)curpos, stime);
							cout << "Elapsed: " << stime << "\r";
						}
						if (!set_cb) {
							set_cb = true;
							if (myCallback) {
								hr = psc->putref_CallbackInterface(myCallback);
								HR(hr);
							}
						}

						MSG msg{};
						// need a message pump to get the meta, since its sent by a PostMessage()!
						BOOL bRet = PeekMessage(&msg, NULL, 0, 0, 1);

						if (bRet > 0)  // (bRet > 0 indicates a message that must be processed.)
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
				}
				else {
					if (FAILED(hr)) {
						break;
					}
				}
			}


			if (hr == S_OK) {
				switch (code)
				{

				case EC_COMPLETE:
				{
					cout << "\nGraph finished, with EC_COMPLETE" << endl;
					break;
				}

				case EC_ERRORABORT:
				{
					cout << "\nGraph finished, with EC_ERRORABORT" << endl;
					break;
				}
				case EC_USERABORT:
				{
					cout << "\nGraph finished, with EC_USERABORT" << endl;
					break;
				}
				default: {
					cout << "\nI don't know why the graph completed, but it did." << endl;
				}

				}
			}
			else {
				if (g_done == 0) {
					cout << "\nGraph completed unexpectedly" << endl;
				}
			}


			hr = E_FAIL;
			int play_attempts = 0;
			while (FAILED(hr) && g_done == 0) {
				// whatever happens, stop the graph and restart it:
				hr = pControl->Stop();
				hr = pPos->put_CurrentPosition(0);
				hr = pControl->Run();
				if (SUCCEEDED(hr)) {
					play_attempts = 0;
				}
				else {
					play_attempts++;
				}
				if FAILED(hr) {
					cerr << "Wanted to play again, but play failed, with: " << hr << endl;
					_com_error err(hr);
					LPCTSTR errMsg = err.ErrorMessage();
					cerr << "Error says: " << errMsg << endl;
					if (play_attempts > 5) {
						cerr << "Tried to re-start playback more than 5 times, rebuilding..." << endl;
						return -7777;
					}
					cerr << "Waiting 20 seconds, and trying again..." << endl;
					Sleep(20000);
				}
			}


		}// while (true)

		myCallback->Release(); // "new" creates it with a refcount of 1

	}
	catch (const std::exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
	}
	if (g_done) {
		g_done = ALL_DONE;
	}
	return NOERROR;
}


int main(int argc, char** argv)
{

	int rv = 0;
	while (g_done <=0 )
	{
		cout << "Running app ..." << endl;
		rv = app(argc, argv);
		if (g_done <= 0) {
			cout << "Waiting to invoke the main application again ..." << endl;
			Sleep(5000);
			cout << endl << endl;
			cout << "-----------------------------------------------------" << endl;
		}
		else {
			break;
		}
	}

	return rv;
}

