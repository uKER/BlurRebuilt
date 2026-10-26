class BaseThread 
{
    protected:
        HANDLE EKillThread;           // When set, thread will be killed
        HANDLE EThreadIsDead;         // Thread sets to indicate thread is finished

    private:
        CWinThread *Thread;

    public:
        static UINT __cdecl ThreadFunc(LPVOID);

    public:
        BaseThread();

        virtual ~BaseThread();

        virtual void Create(int mode = 0);    // Start immediately

        virtual void Destroy(void);

        void DoExecute(void);

        virtual void Execute(void) = 0;       // Derived classes will supply implementation

        virtual void Suspend(void);

        virtual void Resume(void);

};