// [Blur2PC] T259. Official AMAX DLL stays in an x86 child; game calls never wait on network I/O.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <string.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>
#include "AmaxClientBridge.h"
#include "../AMAX/BridgeProtocol.h"

namespace AmaxClientBridge {
namespace {
    using namespace AmaxClientProtocol;
    enum { Connecting, Open, Closed, Broken };
    const unsigned MaxBuffered = 4 * 1024 * 1024;
    struct Buffer { Buffer* next; unsigned size, offset; unsigned char data[1]; };
    struct Stream {
        Stream* next; int local, remote, state; bool closing;
        Buffer *sendHead, *sendTail, *recvHead, *recvTail;
        unsigned sendBytes, recvBytes;
    };
    struct Session {
        CRITICAL_SECTION lock;
        HANDLE thread, stop, wake, pipe, process, job;
        volatile LONG status;
        const char* error;
        Stream* streams;
        Login login;
        Session() : thread(NULL), stop(NULL), wake(NULL), pipe(INVALID_HANDLE_VALUE), process(NULL), job(NULL),
            status(Stopped), error(""), streams(NULL) { InitializeCriticalSection(&lock); ZeroMemory(&login,sizeof(login)); }
    } s;
    struct Lock { Lock() { EnterCriticalSection(&s.lock); } ~Lock() { LeaveCriticalSection(&s.lock); } };
    Stream* Find(int handle) { for(Stream* p=s.streams;p;p=p->next) if(p->local==handle && !p->closing)return p; return NULL; }
    void FreeBuffers(Buffer* p) { while(p) { Buffer* next=p->next; HeapFree(GetProcessHeap(),0,p);p=next; } }
    bool Append(Buffer*& head,Buffer*& tail,unsigned& total,const void* data,unsigned size) {
        if(size>MaxBuffered-total)return false;
        Buffer* p=(Buffer*)HeapAlloc(GetProcessHeap(),0,sizeof(Buffer)+size);
        if(!p)return false;
        p->next=NULL;p->size=size;p->offset=0;memcpy(p->data,data,size);
        if(tail)tail->next=p;else head=p;tail=p;total+=size;return true;
    }
    unsigned Consume(Buffer*& head,Buffer*& tail,unsigned& total,void* data,unsigned size) {
        unsigned used=0;
        while(head && used<size) {
            unsigned count=head->size-head->offset;if(count>size-used)count=size-used;
            if(data)memcpy((unsigned char*)data+used,head->data+head->offset,count);
            used+=count;head->offset+=count;total-=count;
            if(head->offset==head->size) { Buffer* old=head;head=old->next;HeapFree(GetProcessHeap(),0,old);if(!head)tail=NULL; }
        } return used;
    }
    void Fail(const char* error) {
        Lock guard;s.error=error;InterlockedExchange(&s.status,Failed);
        for(Stream* p=s.streams;p;p=p->next)p->state=Broken;
    }
    bool WaitIO(OVERLAPPED& operation,BOOL immediate,DWORD& count) {
        if(immediate)return true;
        if(GetLastError()!=ERROR_IO_PENDING)return false;
        HANDLE events[]={s.stop,operation.hEvent};
        if(WaitForMultipleObjects(2,events,FALSE,30000)!=WAIT_OBJECT_0+1) {
            CancelIoEx(s.pipe,&operation);
            GetOverlappedResult(s.pipe,&operation,&count,TRUE);
            return false;
        }
        return GetOverlappedResult(s.pipe,&operation,&count,FALSE)!=FALSE;
    }
    bool Transfer(void* data,DWORD bytes,bool write) {
        unsigned char* cursor=(unsigned char*)data;
        OVERLAPPED operation={};operation.hEvent=CreateEventW(NULL,TRUE,FALSE,NULL);
        if(!operation.hEvent)return false;
        bool ok=true;
        while(bytes && ok) {
            ResetEvent(operation.hEvent);DWORD count=0;
            BOOL immediate=write?WriteFile(s.pipe,cursor,bytes,&count,&operation):ReadFile(s.pipe,cursor,bytes,&count,&operation);
            ok=WaitIO(operation,immediate,count) && count!=0;
            if(ok) {cursor+=count;bytes-=count;}
        }
        CloseHandle(operation.hEvent);return ok;
    }
    bool Rpc(unsigned op,int handle,unsigned size,void* data,int& result) {
        Command command={op,handle,size};
        if(!Transfer(&command,sizeof(command),true))return false;
        if(op==Send && !Transfer(data,size,true))return false;
        if(!Transfer(&result,sizeof(result),false))return false;
        if(op==Receive && result>0 && ((unsigned)result>size || !Transfer(data,result,false)))return false;
        return true;
    }
    bool IsFile(const wchar_t* path) {DWORD attributes=GetFileAttributesW(path);return attributes!=INVALID_FILE_ATTRIBUTES && !(attributes&FILE_ATTRIBUTE_DIRECTORY);}
    // [Blur2PC] T261: only inspect the user's extracted redist folder. The ASI
    // is a DLL; the other DLLs in the archive are dependencies, not the client.
    bool FindClientIn(const wchar_t* directory, wchar_t (&dll)[MAX_PATH]) {
        DWORD attributes=GetFileAttributesW(directory);
        if(attributes==INVALID_FILE_ATTRIBUTES || !(attributes&FILE_ATTRIBUTE_DIRECTORY) || (attributes&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        if(swprintf_s(dll,L"%s\\amax_auth.asi",directory)>=0 && IsFile(dll))return true;
        wchar_t pattern[MAX_PATH];
        if(swprintf_s(pattern,L"%s\\*",directory)<0)return false;
        WIN32_FIND_DATAW entry;HANDLE search=FindFirstFileW(pattern,&entry);
        if(search==INVALID_HANDLE_VALUE)return false;
        bool found=false;
        do {
            if(!(entry.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) || (entry.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) || !wcscmp(entry.cFileName,L".") || !wcscmp(entry.cFileName,L".."))continue;
            wchar_t child[MAX_PATH];
            if(swprintf_s(child,L"%s\\%s",directory,entry.cFileName)>=0 && FindClientIn(child,dll)) {found=true;break;}
        } while(FindNextFileW(search,&entry));
        FindClose(search);return found;
    }
    bool FindClient(wchar_t (&dll)[MAX_PATH]) {
        wchar_t directory[MAX_PATH],root[MAX_PATH];
        DWORD length=GetModuleFileNameW(NULL,directory,MAX_PATH);
        if(length && length<MAX_PATH) {
            wchar_t* slash=wcsrchr(directory,L'\\');
            if(slash) {
                *slash=0;
                if(swprintf_s(root,L"%s\\amax_client_files",directory)>=0 && FindClientIn(root,dll))return true;
            }
        }
        DWORD size=GetFullPathNameW(L"amax_client_files",MAX_PATH,root,NULL);
        return size && size<MAX_PATH && FindClientIn(root,dll);
    }
    bool LaunchHost() {
        wchar_t directory[MAX_PATH],host[MAX_PATH],dll[MAX_PATH];
        DWORD length=GetModuleFileNameW(NULL,directory,MAX_PATH);
        if(!length || length>=MAX_PATH)return false;
        wchar_t* slash=wcsrchr(directory,L'\\');if(!slash)return false;*slash=0;
        if(swprintf_s(host,L"%s\\AmaxClientHost.exe",directory)<0 || !IsFile(host)) {Fail("AmaxClientHost.exe is missing beside the game.");return false;}
        if(!FindClient(dll)) {Fail("Official client DLL is missing from amax_client_files.");return false;}
        HCRYPTPROV provider=0;unsigned random[4];
        if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))return false;
        BOOL randomOk=CryptGenRandom(provider,sizeof(random),(BYTE*)random);CryptReleaseContext(provider,0);
        if(!randomOk)return false;
        wchar_t name[128];swprintf_s(name,L"\\\\.\\pipe\\BlurAmax-%08x%08x%08x%08x",random[0],random[1],random[2],random[3]);
        s.pipe=CreateNamedPipeW(name,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,1,MaxTransfer,MaxTransfer,0,NULL);
        if(s.pipe==INVALID_HANDLE_VALUE)return false;
        s.job=CreateJobObjectW(NULL,NULL);if(!s.job)return false;
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if(!SetInformationJobObject(s.job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)))return false;
        wchar_t command[3*MAX_PATH+160];
        if(swprintf_s(command,L"\"%s\" \"%s\" \"%s\"",host,name,dll)<0)return false;
        STARTUPINFOW startup={};startup.cb=sizeof(startup);PROCESS_INFORMATION process={};
        if(!CreateProcessW(host,command,NULL,NULL,FALSE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,directory,&startup,&process))return false;
        s.process=process.hProcess;
        if(!AssignProcessToJobObject(s.job,process.hProcess)) {TerminateProcess(process.hProcess,1);CloseHandle(process.hThread);return false;}
        ResumeThread(process.hThread);CloseHandle(process.hThread);
        OVERLAPPED operation={};operation.hEvent=CreateEventW(NULL,TRUE,FALSE,NULL);DWORD count=0;
        if(!operation.hEvent)return false;
        BOOL connected=ConnectNamedPipe(s.pipe,&operation);
        bool ok=connected || GetLastError()==ERROR_PIPE_CONNECTED;
        if(!ok)ok=WaitIO(operation,FALSE,count);
        CloseHandle(operation.hEvent);
        ULONG clientPid=0;
        if(!ok || !GetNamedPipeClientProcessId(s.pipe,&clientPid) || clientPid!=process.dwProcessId)return false;
        int result=BadProtocol;
        ok=Transfer(&s.login,sizeof(s.login),true) && Transfer(&result,sizeof(result),false);
        SecureZeroMemory(&s.login,sizeof(s.login));
        if(!ok)return false;
        switch(result) {
        case Accepted:return true;
        case LoginRejected:Fail("Official AMAX client rejected authentication.");break;
        case HooksMissing:case HostLayout:Fail("Installed AMAX client is incompatible with this host layout.");break;
        case LoadClient:Fail("Unable to load the installed official AMAX client.");break;
        default:Fail("AMAX companion protocol failed.");break;
        }
        return false;
    }
    DWORD WINAPI Worker(void*) {
        if(!LaunchHost()) {if(GetStatus()!=Failed)Fail("AMAX companion did not authenticate or stopped responding.");return 0;}
        InterlockedExchange(&s.status,Ready);
        HANDLE events[]={s.stop,s.wake};unsigned char data[MaxTransfer];
        while(WaitForSingleObject(s.stop,0)!=WAIT_OBJECT_0) {
            // Only this worker creates/deletes stream objects. Callers use their
            // native socket IDs under the lock, so render/network polling stays nonblocking.
            Stream* current;{Lock guard;current=s.streams;}
            for(Stream* p=current;p;) {
                bool closing;int state;{Lock guard;closing=p->closing;state=p->state;}
                int result=0;
                if(closing) {
                    if(p->remote!=-1 && !Rpc(Close,p->remote,0,NULL,result))goto failed;
                    Lock guard;Stream** link=&s.streams;while(*link!=p)link=&(*link)->next;
                    *link=p->next;Stream* next=p->next;FreeBuffers(p->sendHead);FreeBuffers(p->recvHead);HeapFree(GetProcessHeap(),0,p);p=next;continue;
                }
                // A close can arrive after this iteration visited the old stream.
                // Defer the replacement until its remote socket has actually closed.
                bool transportBusy=false;
                {Lock guard;for(Stream* other=s.streams;other;other=other->next)
                    if(other!=p && other->remote!=-1)transportBusy=true;}
                if(state==Connecting && !transportBusy) {
                    int remote=-1;
                    if(!Rpc(Create,0,0,NULL,remote))goto failed;
                    {Lock guard;p->remote=remote;}
                    if(remote<0) {Lock guard;p->state=Broken;}
                    else {
                        if(!Rpc(Connect,remote,0,NULL,result))goto failed;
                        Lock guard;p->state=result==BD_NET_SUCCESS?Open:Broken;
                    }
                }
                bool open;{Lock guard;open=p->state==Open && !p->closing;}
                if(open) {
                    unsigned size=0;
                    {Lock guard;if(p->sendHead) {size=p->sendHead->size-p->sendHead->offset;if(size>MaxTransfer)size=MaxTransfer;memcpy(data,p->sendHead->data+p->sendHead->offset,size);}}
                    if(size) {
                        if(!Rpc(Send,p->remote,size,data,result))goto failed;
                        Lock guard;
                        if(result>0 && (unsigned)result<=size)Consume(p->sendHead,p->sendTail,p->sendBytes,NULL,result);
                        else if(result!=BD_NET_WOULD_BLOCK)p->state=Broken;
                    }
                    unsigned capacity;{Lock guard;capacity=MaxBuffered-p->recvBytes;if(capacity>MaxTransfer)capacity=MaxTransfer;}
                    if(capacity) {
                        if(!Rpc(Receive,p->remote,capacity,data,result))goto failed;
                        Lock guard;
                        if(result>0) {if(!Append(p->recvHead,p->recvTail,p->recvBytes,data,result))p->state=Broken;}
                        else if(result==0)p->state=Closed;
                        else if(result!=BD_NET_WOULD_BLOCK)p->state=Broken;
                    }
                }
                {Lock guard;p=p->next;}
            }
            if(WaitForMultipleObjects(2,events,FALSE,5)==WAIT_OBJECT_0)break;
        }
        return 0;
    failed:
        Fail("AMAX companion connection closed unexpectedly.");return 0;
    }
    bdBool Owns(int handle) {Lock guard;return Find(handle)!=NULL;}
    bdSocketStatusCode ConnectStream(int handle) {
        Lock guard;Stream* p=Find(handle);
        if(!p) {
            if(GetStatus()!=Ready)return BD_NET_CONNECTION_RESET;
            p=(Stream*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(Stream));if(!p)return BD_NET_ERROR;
            p->local=handle;p->remote=-1;p->state=Connecting;
            Stream** tail=&s.streams;while(*tail)tail=&(*tail)->next;*tail=p;
            SetEvent(s.wake);
        }
        return p->state==Open?BD_NET_SUCCESS:p->state==Connecting?BD_NET_WOULD_BLOCK:BD_NET_CONNECTION_RESET;
    }
    bdInt SendStream(int handle,const void* data,bdUInt size) {
        Lock guard;Stream* p=Find(handle);if(!p)return BD_NET_INVALID_HANDLE;
        if(p->state==Connecting)return BD_NET_WOULD_BLOCK;
        if(p->state!=Open)return BD_NET_CONNECTION_RESET;
        if(!size)return 0;
        if(!Append(p->sendHead,p->sendTail,p->sendBytes,data,size))return BD_NET_WOULD_BLOCK;
        SetEvent(s.wake);return size;
    }
    bdInt ReceiveStream(int handle,void* data,bdUInt size) {
        Lock guard;Stream* p=Find(handle);if(!p)return BD_NET_INVALID_HANDLE;
        if(!size)return 0;
        if(p->recvHead)return Consume(p->recvHead,p->recvTail,p->recvBytes,data,size);
        return p->state==Closed?0:p->state==Broken?BD_NET_CONNECTION_RESET:BD_NET_WOULD_BLOCK;
    }
    void CloseStream(int handle) {Lock guard;Stream* p=Find(handle);if(p) {p->closing=true;SetEvent(s.wake);}}
    bdBool Writable(int handle) {Lock guard;Stream* p=Find(handle);return p && p->state==Open;}
    bdBool BrokenStream(int handle) {Lock guard;Stream* p=Find(handle);return !p || p->state==Broken || p->state==Closed;}
    const bdPlatformStreamSocket::ExternalTransport transport={3074,Owns,ConnectStream,SendStream,ReceiveStream,CloseStream,Writable,BrokenStream};
}
bool IsAvailable() {wchar_t dll[MAX_PATH];return FindClient(dll);}
Status GetStatus() {return (Status)InterlockedCompareExchange(&s.status,0,0);}
const char* Error() {Lock guard;return s.error;}
bool Start(const char* username,const char* password) {
    Stop();
    if(!username || !password || !username[0] || !password[0]
        || strlen(username)>MaxCredential || strlen(password)>MaxCredential) {Fail("AMAX credentials are missing or too long.");return false;}
    s.login.magic=Magic;strcpy_s(s.login.username,username);strcpy_s(s.login.password,password);
    s.error="";s.stop=CreateEventW(NULL,TRUE,FALSE,NULL);s.wake=CreateEventW(NULL,FALSE,FALSE,NULL);
    if(!s.stop || !s.wake) {Stop();Fail("Unable to create AMAX worker events.");return false;}
    InterlockedExchange(&s.status,Starting);
    bdPlatformStreamSocket::setExternalTransport(&transport);
    s.thread=CreateThread(NULL,0,Worker,NULL,0,NULL);
    if(!s.thread) {Stop();Fail("Unable to start AMAX worker.");return false;}
    return true;
}
void Stop() {
    if(s.stop)SetEvent(s.stop);
    if(s.thread) {WaitForSingleObject(s.thread,INFINITE);CloseHandle(s.thread);s.thread=NULL;}
    bdPlatformStreamSocket::setExternalTransport(NULL);
    if(s.pipe!=INVALID_HANDLE_VALUE) {CloseHandle(s.pipe);s.pipe=INVALID_HANDLE_VALUE;}
    // The job contains only the companion this session created; never another game/client.
    if(s.job) {CloseHandle(s.job);s.job=NULL;}
    if(s.process) {WaitForSingleObject(s.process,5000);CloseHandle(s.process);s.process=NULL;}
    if(s.stop) {CloseHandle(s.stop);s.stop=NULL;}if(s.wake) {CloseHandle(s.wake);s.wake=NULL;}
    Lock guard;
    while(s.streams) {Stream* p=s.streams;s.streams=p->next;FreeBuffers(p->sendHead);FreeBuffers(p->recvHead);HeapFree(GetProcessHeap(),0,p);}
    SecureZeroMemory(&s.login,sizeof(s.login));InterlockedExchange(&s.status,Stopped);
}
}
