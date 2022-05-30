#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "result.txt"

HINSTANCE hInst;

int reg=0;
int counter=1;
unsigned int addr=0;

char format[256]="";
char filename[256]="";

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        SendMessageA(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconA(hInst, MAKEINTRESOURCEA(IDI_ICON1)));
        SetDlgItemTextA(hwndDlg, IDC_EDT_COUNTER, "1");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"EAX");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"ECX");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"EDX");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"EBX");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"ESP");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"EBP");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"ESI");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_ADDSTRING, 0, (LPARAM)"EDI");
        SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_SETCURSEL, 0, 0);
    }
    return TRUE;

    case WM_CLOSE:
    {
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    case WM_COMMAND:
    {
        switch(LOWORD(wParam))
        {
        case IDC_EDT_COUNTER:
        {
            BOOL good=false;
            counter=GetDlgItemInt(hwndDlg, IDC_EDT_COUNTER, &good, true);
        }
        return TRUE;

        case IDC_EDT_ADDR:
        {
            char temp[10]="";
            GetDlgItemTextA(hwndDlg, IDC_EDT_ADDR, temp, 10);
            sscanf(temp, "%X", &addr);
        }
        return TRUE;

        case IDC_EDT_FORMAT:
        {
            GetDlgItemTextA(hwndDlg, IDC_EDT_FORMAT, format, 256);
        }
        return TRUE;

        case IDC_EDT_FILENAME:
        {
            GetDlgItemTextA(hwndDlg, IDC_EDT_FILENAME, filename, 256);
        }
        return TRUE;

        case IDC_COMBO_REGISTER:
        {
            switch(HIWORD(wParam))
            {
            case CBN_SELCHANGE:
            {
                reg=SendDlgItemMessageA(hwndDlg, IDC_COMBO_REGISTER, CB_GETCURSEL, 0, 0);
            }
            return TRUE;
            }
        }
        return TRUE;

        case IDC_BTN_ABOUT:
        {
            MessageBoxA(hwndDlg, "Register Sniffer v0.1\n\nCreated by:\nMr. eXoDia // T.P.o.D.T 2012\nmr.exodia.tpodt@gmail.com\nhttp://www.tpodt.com\n\nHow to use this tool?\n1) Find the address where you want to retrieve a register\n2) Select that register in this program\n3) Enter the address\n4) In case you want to retrieve the second time enter 2 in counter\n5) Format text is for sprint (example: Serial : %.8X)\n6) Filename is just the name of the exe\n7) Click Save!\n\nCreated for DMichael, Enjoy!", "About", MB_ICONINFORMATION);
        }
        return TRUE;

        case IDC_BTN_MAKE:
        {
            if(!counter or !addr or !format[0] or !filename[0])
            {
                MessageBoxA(hwndDlg, "Please fill in all data!", "Error...", MB_ICONERROR);
                return TRUE;
            }
            OPENFILENAME ofstruct;
            char filename_exe[256]="";
            memset(&ofstruct, 0, sizeof(ofstruct));
            ofstruct.lStructSize=sizeof(ofstruct);
            ofstruct.hwndOwner=hwndDlg;
            ofstruct.hInstance=hInst;
            ofstruct.lpstrFilter="Executables (*.exe)\0*.exe\0\0";
            ofstruct.lpstrFile=filename_exe;
            ofstruct.nMaxFile=256;
            ofstruct.lpstrDefExt="exe";
            ofstruct.Flags=OFN_EXTENSIONDIFFERENT|OFN_HIDEREADONLY|OFN_NONETWORKBUTTON|OFN_OVERWRITEPROMPT;
            GetSaveFileName(&ofstruct);
            if(!filename_exe[0])
                return TRUE;
            HANDLE hFile=CreateFileA(filename_exe, GENERIC_ALL, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
            if(hFile==INVALID_HANDLE_VALUE)
            {
                MessageBoxA(hwndDlg, "Could not create file!", "Error...", MB_ICONERROR);
                return TRUE;
            }
            memcpy(table+0x1600, format, strlen(format));
            memcpy(table+0x1720, filename, strlen(filename));
            memcpy(table+0x1824, &counter, 4);
            memcpy(table+0x0A16, &reg, 4);
            memcpy(table+0x0A9C, &addr, 4);
            memcpy(table+0x08AA, &addr, 4);
            memcpy(table+0x0874, &addr, 4);
            DWORD written=0;
            if(!WriteFile(hFile, table, 14848, &written, 0))
            {
                CloseHandle(hFile);
                MessageBoxA(hwndDlg, "Could not write file!", "Error...", MB_ICONERROR);
                return TRUE;
            }
            CloseHandle(hFile);
            MessageBoxA(hwndDlg, "File written without errors!", "Done!", MB_ICONINFORMATION);
        }
        return TRUE;
        }
    }
    return TRUE;
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();
    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}
