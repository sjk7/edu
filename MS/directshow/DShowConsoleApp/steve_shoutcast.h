// steve_shouctast.h
#pragma once

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Oct 26 22:08:49 2016
*/
/* Compiler settings for iShoutcast.idl:
Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
error checks: allocation ref bounds_check enum stub_data
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __iShoutcast_h__
#define __iShoutcast_h__

#ifdef __cplusplus
extern "C" {
#endif 

	/* Forward Declarations */

#ifndef __IShoutcastFilter_FWD_DEFINED__
#define __IShoutcastFilter_FWD_DEFINED__
	typedef interface IShoutcastFilter IShoutcastFilter;
#endif 	/* __IShoutcastFilter_FWD_DEFINED__ */


#ifndef __IShoutcastCallback_FWD_DEFINED__
#define __IShoutcastCallback_FWD_DEFINED__
	typedef interface IShoutcastCallback IShoutcastCallback;
#endif 	/* __IShoutcastCallback_FWD_DEFINED__ */


	void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
	void __RPC_USER MIDL_user_free(void __RPC_FAR *);


#ifndef __ShoutcastHelperLib_LIBRARY_DEFINED__
#define __ShoutcastHelperLib_LIBRARY_DEFINED__

	/* library ShoutcastHelperLib */
	/* [helpstring][version][uuid] */


	DEFINE_GUID(LIBID_ShoutcastHelperLib, 0xFB7B89A0, 0xF12B, 0x44d5, 0xB9, 0x46, 0x35, 0x36, 0x58, 0x5A, 0x63, 0x15);

#ifndef __IShoutcastFilter_INTERFACE_DEFINED__
#define __IShoutcastFilter_INTERFACE_DEFINED__

	/* interface IShoutcastFilter */
	/* [unique][helpstring][uuid][object] */


	DEFINE_GUID(IID_IShoutcastFilter, 0x26AF3FBB, 0x9118, 0x4750, 0x83, 0x0F, 0x99, 0x2F, 0xD9, 0x11, 0xB4, 0xDA);

#if defined(__cplusplus) && !defined(CINTERFACE)

	MIDL_INTERFACE("26AF3FBB-9118-4750-830F-992FD911B4DA")
		IShoutcastFilter : public IUnknown
	{
	public:
		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_MetaArtistTitle(
			/* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_MetaURL(
			/* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_CallbackInterface(
			/* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal) = 0;

		virtual /* [helpstring][propputref] */ HRESULT STDMETHODCALLTYPE putref_CallbackInterface(
			/* [in] */ IUnknown __RPC_FAR *newVal) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_StreamTitle(
			/* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

		virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Cancel(void) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_IsCancelling(
			/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_BusyLoading(
			/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal) = 0;

		virtual HRESULT STDMETHODCALLTYPE LoadStream(
			LPCOLESTR address) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_LoadResult(
			/* [retval][out] */ long __RPC_FAR *pRetVal) = 0;

		virtual /* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorDesc(
			/* [in] */ LONG lerr,
			/* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

	};

#else 	/* C style interface */

	typedef struct IShoutcastFilterVtbl
	{
		BEGIN_INTERFACE

			HRESULT(STDMETHODCALLTYPE __RPC_FAR *QueryInterface)(
				IShoutcastFilter __RPC_FAR * This,
				/* [in] */ REFIID riid,
				/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

		ULONG(STDMETHODCALLTYPE __RPC_FAR *AddRef)(
			IShoutcastFilter __RPC_FAR * This);

		ULONG(STDMETHODCALLTYPE __RPC_FAR *Release)(
			IShoutcastFilter __RPC_FAR * This);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_MetaArtistTitle)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ BSTR __RPC_FAR *pVal);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_MetaURL)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ BSTR __RPC_FAR *pVal);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_CallbackInterface)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal);

		/* [helpstring][propputref] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *putref_CallbackInterface)(
			IShoutcastFilter __RPC_FAR * This,
			/* [in] */ IUnknown __RPC_FAR *newVal);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_StreamTitle)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ BSTR __RPC_FAR *pVal);

		/* [helpstring] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *Cancel)(
			IShoutcastFilter __RPC_FAR * This);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_IsCancelling)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_BusyLoading)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal);

		HRESULT(STDMETHODCALLTYPE __RPC_FAR *LoadStream)(
			IShoutcastFilter __RPC_FAR * This,
			LPCOLESTR address);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_LoadResult)(
			IShoutcastFilter __RPC_FAR * This,
			/* [retval][out] */ long __RPC_FAR *pRetVal);

		/* [helpstring][propget] */ HRESULT(STDMETHODCALLTYPE __RPC_FAR *get_ErrorDesc)(
			IShoutcastFilter __RPC_FAR * This,
			/* [in] */ LONG lerr,
			/* [retval][out] */ BSTR __RPC_FAR *pVal);

		END_INTERFACE
	} IShoutcastFilterVtbl;

	interface IShoutcastFilter
	{
		CONST_VTBL struct IShoutcastFilterVtbl __RPC_FAR *lpVtbl;
	};



#ifdef COBJMACROS


#define IShoutcastFilter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShoutcastFilter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShoutcastFilter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShoutcastFilter_get_MetaArtistTitle(This,pVal)	\
    (This)->lpVtbl -> get_MetaArtistTitle(This,pVal)

#define IShoutcastFilter_get_MetaURL(This,pVal)	\
    (This)->lpVtbl -> get_MetaURL(This,pVal)

#define IShoutcastFilter_get_CallbackInterface(This,pVal)	\
    (This)->lpVtbl -> get_CallbackInterface(This,pVal)

#define IShoutcastFilter_putref_CallbackInterface(This,newVal)	\
    (This)->lpVtbl -> putref_CallbackInterface(This,newVal)

#define IShoutcastFilter_get_StreamTitle(This,pVal)	\
    (This)->lpVtbl -> get_StreamTitle(This,pVal)

#define IShoutcastFilter_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IShoutcastFilter_get_IsCancelling(This,pRetVal)	\
    (This)->lpVtbl -> get_IsCancelling(This,pRetVal)

#define IShoutcastFilter_get_BusyLoading(This,pRetVal)	\
    (This)->lpVtbl -> get_BusyLoading(This,pRetVal)

#define IShoutcastFilter_LoadStream(This,address)	\
    (This)->lpVtbl -> LoadStream(This,address)

#define IShoutcastFilter_get_LoadResult(This,pRetVal)	\
    (This)->lpVtbl -> get_LoadResult(This,pRetVal)

#define IShoutcastFilter_get_ErrorDesc(This,lerr,pVal)	\
    (This)->lpVtbl -> get_ErrorDesc(This,lerr,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_MetaArtistTitle_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ BSTR __RPC_FAR *pVal);


	void __RPC_STUB IShoutcastFilter_get_MetaArtistTitle_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_MetaURL_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ BSTR __RPC_FAR *pVal);


	void __RPC_STUB IShoutcastFilter_get_MetaURL_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_CallbackInterface_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal);


	void __RPC_STUB IShoutcastFilter_get_CallbackInterface_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propputref] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_putref_CallbackInterface_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [in] */ IUnknown __RPC_FAR *newVal);


	void __RPC_STUB IShoutcastFilter_putref_CallbackInterface_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_StreamTitle_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ BSTR __RPC_FAR *pVal);


	void __RPC_STUB IShoutcastFilter_get_StreamTitle_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_Cancel_Proxy(
		IShoutcastFilter __RPC_FAR * This);


	void __RPC_STUB IShoutcastFilter_Cancel_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_IsCancelling_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal);


	void __RPC_STUB IShoutcastFilter_get_IsCancelling_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_BusyLoading_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ VARIANT_BOOL __RPC_FAR *pRetVal);


	void __RPC_STUB IShoutcastFilter_get_BusyLoading_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IShoutcastFilter_LoadStream_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		LPCOLESTR address);


	void __RPC_STUB IShoutcastFilter_LoadStream_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_LoadResult_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [retval][out] */ long __RPC_FAR *pRetVal);


	void __RPC_STUB IShoutcastFilter_get_LoadResult_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	/* [helpstring][propget] */ HRESULT STDMETHODCALLTYPE IShoutcastFilter_get_ErrorDesc_Proxy(
		IShoutcastFilter __RPC_FAR * This,
		/* [in] */ LONG lerr,
		/* [retval][out] */ BSTR __RPC_FAR *pVal);


	void __RPC_STUB IShoutcastFilter_get_ErrorDesc_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IShoutcastFilter_INTERFACE_DEFINED__ */


#ifndef __IShoutcastCallback_INTERFACE_DEFINED__
#define __IShoutcastCallback_INTERFACE_DEFINED__

	/* interface IShoutcastCallback */
	/* [unique][helpstring][uuid][object] */


	DEFINE_GUID(IID_IShoutcastCallback, 0xF1EE9943, 0x9567, 0x484b, 0xA3, 0x5B, 0x15, 0x90, 0x7A, 0x81, 0xD9, 0xF2);

#if defined(__cplusplus) && !defined(CINTERFACE)

	MIDL_INTERFACE("F1EE9943-9567-484b-A35B-15907A81D9F2")
		IShoutcastCallback : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE MetaChangeCallback(
			BSTR sArtistTitle,
			BSTR sURL,
			BSTR sFullMeta) = 0;

		virtual HRESULT STDMETHODCALLTYPE StreamBuffering(
			/* [in] */ int BufferPercentFull) = 0;

		virtual HRESULT STDMETHODCALLTYPE OdStreamSeg(
			double seg) = 0;

	};

#else 	/* C style interface */

	typedef struct IShoutcastCallbackVtbl
	{
		BEGIN_INTERFACE

			HRESULT(STDMETHODCALLTYPE __RPC_FAR *QueryInterface)(
				IShoutcastCallback __RPC_FAR * This,
				/* [in] */ REFIID riid,
				/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

		ULONG(STDMETHODCALLTYPE __RPC_FAR *AddRef)(
			IShoutcastCallback __RPC_FAR * This);

		ULONG(STDMETHODCALLTYPE __RPC_FAR *Release)(
			IShoutcastCallback __RPC_FAR * This);

		HRESULT(STDMETHODCALLTYPE __RPC_FAR *MetaChangeCallback)(
			IShoutcastCallback __RPC_FAR * This,
			BSTR sArtistTitle,
			BSTR sURL,
			BSTR sFullMeta);

		HRESULT(STDMETHODCALLTYPE __RPC_FAR *StreamBuffering)(
			IShoutcastCallback __RPC_FAR * This,
			/* [in] */ int BufferPercentFull);

		HRESULT(STDMETHODCALLTYPE __RPC_FAR *OdStreamSeg)(
			IShoutcastCallback __RPC_FAR * This,
			double seg);

		END_INTERFACE
	} IShoutcastCallbackVtbl;

	interface IShoutcastCallback
	{
		CONST_VTBL struct IShoutcastCallbackVtbl __RPC_FAR *lpVtbl;
	};



#ifdef COBJMACROS


#define IShoutcastCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShoutcastCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShoutcastCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShoutcastCallback_MetaChangeCallback(This,sArtistTitle,sURL,sFullMeta)	\
    (This)->lpVtbl -> MetaChangeCallback(This,sArtistTitle,sURL,sFullMeta)

#define IShoutcastCallback_StreamBuffering(This,BufferPercentFull)	\
    (This)->lpVtbl -> StreamBuffering(This,BufferPercentFull)

#define IShoutcastCallback_OdStreamSeg(This,seg)	\
    (This)->lpVtbl -> OdStreamSeg(This,seg)

#endif /* COBJMACROS */


#endif 	/* C style interface */



	HRESULT STDMETHODCALLTYPE IShoutcastCallback_MetaChangeCallback_Proxy(
		IShoutcastCallback __RPC_FAR * This,
		BSTR sArtistTitle,
		BSTR sURL,
		BSTR sFullMeta);


	void __RPC_STUB IShoutcastCallback_MetaChangeCallback_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IShoutcastCallback_StreamBuffering_Proxy(
		IShoutcastCallback __RPC_FAR * This,
		/* [in] */ int BufferPercentFull);


	void __RPC_STUB IShoutcastCallback_StreamBuffering_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);


	HRESULT STDMETHODCALLTYPE IShoutcastCallback_OdStreamSeg_Proxy(
		IShoutcastCallback __RPC_FAR * This,
		double seg);


	void __RPC_STUB IShoutcastCallback_OdStreamSeg_Stub(
		IRpcStubBuffer *This,
		IRpcChannelBuffer *_pRpcChannelBuffer,
		PRPC_MESSAGE _pRpcMessage,
		DWORD *_pdwStubPhase);



#endif 	/* __IShoutcastCallback_INTERFACE_DEFINED__ */

#endif /* __ShoutcastHelperLib_LIBRARY_DEFINED__ */

	/* Additional Prototypes for ALL interfaces */

	/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
