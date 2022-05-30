#define _WIN32_WINNT 0x5001
#include <windows.h>
#include <commctrl.h>

HINSTANCE hInst;

char format_string[]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char filename[]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
unsigned int breakpoint_place=0;
int counter=1;
int register_ad=0;

BOOL SetSingleStep(HANDLE hThread)
{
    CONTEXT Ctx;
    Ctx.ContextFlags=CONTEXT_FULL;
    if(GetThreadContext(hThread, &Ctx))
    {
        Ctx.EFlags|=0x100;
        if(SetThreadContext(hThread, &Ctx))
            return true;
    }
    return false;
}

unsigned int ReturnContext(int reg, CONTEXT* ctx)
{
    switch(reg)
    {
    case 0:
        return ctx->Eax;
        break;
    case 1:
        return ctx->Ecx;
        break;
    case 2:
        return ctx->Edx;
        break;
    case 3:
        return ctx->Ebx;
        break;
    case 4:
        return ctx->Esp;
        break;
    case 5:
        return ctx->Ebp;
        break;
    case 6:
        return ctx->Esi;
        break;
    case 7:
        return ctx->Edi;
        break;
    }
    return 0;
}

bool DebugChild(const char* file_path, const char* file_dir)
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    CONTEXT Context;
    DEBUG_EVENT DebugEvent;
    HANDLE hCurrentThread;
    bool bContinue=true;
    bool returnvalid=false;
    bool sysbreak=true;
    unsigned int dwContinueStatus;
    GetStartupInfo(&StartupInfo);
    Context.ContextFlags = CONTEXT_FULL;

    if(!CreateProcess(file_path, 0, 0, 0, true, DEBUG_ONLY_THIS_PROCESS|DEBUG_PROCESS, 0, file_dir, &StartupInfo, &ProcessInfo))
    {
        return false;
    }
    BYTE cc=0xCC;
    BYTE old=0;
    if(!ReadProcessMemory(ProcessInfo.hProcess, (void*)breakpoint_place, &old, 1, 0))
    {
        MessageBoxA(0, "Could not read old data!", "Fail...", MB_ICONERROR);
        ExitProcess(1);
    }
    if(!WriteProcessMemory(ProcessInfo.hProcess, (void*)breakpoint_place, &cc, 1, 0))
    {
        MessageBoxA(0, "Could not set breakpoint!", "Fail...", MB_ICONERROR);
        ExitProcess(1);
    }
    do
    {
        dwContinueStatus = DBG_CONTINUE;
        if(WaitForDebugEvent(&DebugEvent, INFINITE))
        {
            hCurrentThread=OpenThread(THREAD_ALL_ACCESS, FALSE, DebugEvent.dwThreadId);
            if(hCurrentThread)
            {
                if(GetThreadContext(hCurrentThread, &Context))
                {
                    switch(DebugEvent.dwDebugEventCode)
                    {
                    case EXCEPTION_DEBUG_EVENT:
                    {
                        dwContinueStatus=DBG_CONTINUE;
                        switch(DebugEvent.u.Exception.ExceptionRecord.ExceptionCode)
                        {
                        case EXCEPTION_BREAKPOINT:
                        {
                            if(!sysbreak)
                            {
                                counter--;
                                if(!counter)
                                {
                                    unsigned int eax=ReturnContext(register_ad, &Context);
                                    char text_final[256]="";
                                    wsprintf(text_final, format_string, eax);
                                    MessageBoxA(0, text_final, "Data:", MB_OK|MB_TOPMOST|MB_SETFOREGROUND);
                                    bContinue=false;
                                    if(!WriteProcessMemory(ProcessInfo.hProcess, (void*)breakpoint_place, &old, 1, 0))
                                    {
                                        MessageBoxA(0, "Could not write data!", "Fail...", MB_ICONERROR);
                                        ExitProcess(1);
                                    }
                                }
                                else
                                    SetSingleStep(hCurrentThread);
                            }
                            else
                                sysbreak=false;
                        }
                        break;
                        }
                    }
                    break;

                    case EXIT_PROCESS_DEBUG_EVENT:
                    {
                        bContinue=false;
                    }
                    break;
                    }
                }
                else
                {
                    bContinue=false;
                }
                CloseHandle(hCurrentThread);
            }
            else
            {
                bContinue=false; //OpenThread failed.
            }
        }
        else
        {
            bContinue=false; //WaitForDebugEvent timed out.
        }
        returnvalid=ContinueDebugEvent(DebugEvent.dwProcessId, DebugEvent.dwThreadId, dwContinueStatus);
    }
    while(returnvalid and bContinue);
    DebugActiveProcessStop(ProcessInfo.dwProcessId);
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);
    return true;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    char curdir[256]="",filename1[256]="";
    GetCurrentDirectoryA(256, curdir);
    wsprintf(filename1, "%s\\%s", curdir, filename);
    hInst=hInstance;
    InitCommonControls();
    DebugChild(filename1, curdir);
    return 0;
}
