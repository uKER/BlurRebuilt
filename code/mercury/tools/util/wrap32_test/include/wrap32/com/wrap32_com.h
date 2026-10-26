

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Fri Aug 03 01:46:25 2007
 */
/* Compiler settings for .\COMServer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __wrap32_com_h__
#define __wrap32_com_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IServerWrapper_FWD_DEFINED__
#define __IServerWrapper_FWD_DEFINED__
typedef interface IServerWrapper IServerWrapper;
#endif 	/* __IServerWrapper_FWD_DEFINED__ */


#ifndef __Wrap32_FWD_DEFINED__
#define __Wrap32_FWD_DEFINED__

#ifdef __cplusplus
typedef class Wrap32 Wrap32;
#else
typedef struct Wrap32 Wrap32;
#endif /* __cplusplus */

#endif 	/* __Wrap32_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IServerWrapper_INTERFACE_DEFINED__
#define __IServerWrapper_INTERFACE_DEFINED__

/* interface IServerWrapper */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IServerWrapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3A25EAE3-D61C-4F3D-876E-FB4484215A43")
    IServerWrapper : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE XGTileTextureLevel( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE XGTileSurface( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE XGGetMipTailLevelOffset( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE XGGetTextureDesc( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE XGGetMipTailBaseLevel( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE XGSetTextureHeader( 
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result) = 0;
       
    };
    
#else 	/* C style interface */

    typedef struct IServerWrapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServerWrapper * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServerWrapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServerWrapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IServerWrapper * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IServerWrapper * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IServerWrapper * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IServerWrapper * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *XGTileTextureLevel )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID);
        
        HRESULT ( STDMETHODCALLTYPE *XGTileSurface )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID);
        
        HRESULT ( STDMETHODCALLTYPE *XGGetMipTailLevelOffset )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result);
        
        HRESULT ( STDMETHODCALLTYPE *XGGetTextureDesc )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID);
        
        HRESULT ( STDMETHODCALLTYPE *XGGetMipTailBaseLevel )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result);
        
        HRESULT ( STDMETHODCALLTYPE *XGSetTextureHeader )( 
            IServerWrapper * This,
            /* [in] */ UINT callingPID,
            /* [in] */ UINT sharedMemoryID,
            /* [retval][out] */ UINT *result);
        
        END_INTERFACE
    } IServerWrapperVtbl;

    interface IServerWrapper
    {
        CONST_VTBL struct IServerWrapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerWrapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerWrapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerWrapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerWrapper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IServerWrapper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IServerWrapper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IServerWrapper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IServerWrapper_XMemCreateCompressionContext(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> XMemCreateCompressionContext(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_XMemCompress(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> XMemCompress(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_XMemDestroyCompressionContext(This,callingPID,sharedMemoryID)	\
    (This)->lpVtbl -> XMemDestroyCompressionContext(This,callingPID,sharedMemoryID)

#define IServerWrapper_XGTileTextureLevel(This,callingPID,sharedMemoryID)	\
    (This)->lpVtbl -> XGTileTextureLevel(This,callingPID,sharedMemoryID)

#define IServerWrapper_XGTileSurface(This,callingPID,sharedMemoryID)	\
    (This)->lpVtbl -> XGTileSurface(This,callingPID,sharedMemoryID)

#define IServerWrapper_XGGetMipTailLevelOffset(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> XGGetMipTailLevelOffset(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_XGGetTextureDesc(This,callingPID,sharedMemoryID)	\
    (This)->lpVtbl -> XGGetTextureDesc(This,callingPID,sharedMemoryID)

#define IServerWrapper_XGGetMipTailBaseLevel(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> XGGetMipTailBaseLevel(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_XGSetTextureHeader(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> XGSetTextureHeader(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_CalculateSize(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> CalculateSize(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_GetCompressLibHandle(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> GetCompressLibHandle(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_GetDefaultCompressionTechnique(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> GetDefaultCompressionTechnique(This,callingPID,sharedMemoryID,result)

#define IServerWrapper_CompressSingleMipLevel(This,callingPID,sharedMemoryID,result)	\
    (This)->lpVtbl -> CompressSingleMipLevel(This,callingPID,sharedMemoryID,result)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpcontext][helpstring][id] */ HRESULT STDMETHODCALLTYPE IServerWrapper_XMemCreateCompressionContext_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_XMemCreateCompressionContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XMemCompress_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_XMemCompress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XMemDestroyCompressionContext_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID);


void __RPC_STUB IServerWrapper_XMemDestroyCompressionContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGTileTextureLevel_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID);


void __RPC_STUB IServerWrapper_XGTileTextureLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGTileSurface_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID);


void __RPC_STUB IServerWrapper_XGTileSurface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGGetMipTailLevelOffset_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_XGGetMipTailLevelOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGGetTextureDesc_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID);


void __RPC_STUB IServerWrapper_XGGetTextureDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGGetMipTailBaseLevel_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_XGGetMipTailBaseLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_XGSetTextureHeader_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_XGSetTextureHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_CalculateSize_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_CalculateSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_GetCompressLibHandle_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_GetCompressLibHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_GetDefaultCompressionTechnique_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_GetDefaultCompressionTechnique_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerWrapper_CompressSingleMipLevel_Proxy( 
    IServerWrapper * This,
    /* [in] */ UINT callingPID,
    /* [in] */ UINT sharedMemoryID,
    /* [retval][out] */ UINT *result);


void __RPC_STUB IServerWrapper_CompressSingleMipLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IServerWrapper_INTERFACE_DEFINED__ */



#ifndef __COMServerLib_LIBRARY_DEFINED__
#define __COMServerLib_LIBRARY_DEFINED__

/* library COMServerLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_COMServerLib;

EXTERN_C const CLSID CLSID_Wrap32;

#ifdef __cplusplus

class DECLSPEC_UUID("AE14D6C6-2E7E-484D-A621-E62611B75B42")
Wrap32;
#endif
#endif /* __COMServerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


