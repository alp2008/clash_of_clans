#include "TXLib.h"
#include "button.cpp"
#include "picture.cpp"
#include <fstream>
#include <stdio.h>
#include <dirent.h>

using namespace std;

int get_w(string adress)
{
    FILE *f1 = fopen(adress.c_str(), "rb");
    unsigned char headerinfo[54];
    fread(headerinfo, sizeof(unsigned char), 54, f1);
    int w = *(int *)&headerinfo[18];

    return w;
}

int get_h(string adress)
{
    FILE *f1 = fopen(adress.c_str(), "rb");
    unsigned char headerinfo[54];
    fread(headerinfo, sizeof(unsigned char), 54, f1);
    int h = *(int *)&headerinfo[22];

    return h;
}

int readFromDir(string adress, Picture menuPic[], int count_pic)
{
    DIR *dir;
    struct dirent *ent;
    int X = 20;
    int Y = 100;
    if ((dir = opendir (adress.c_str())) != NULL)
    {
      while ((ent = readdir (dir)) != NULL)
      {
        if((string)ent->d_name != "." && (string)ent->d_name != "..")
        {
            if(Y<=800 && X == 20)
            {
                X = 20;
            }
            else if (Y>800)
            {
                X = 150;
                Y = 100;
            }
            menuPic[count_pic].x = X;
            menuPic[count_pic].y = Y;
            menuPic[count_pic].adress = adress + (string)ent->d_name;
            count_pic++;
            Y += 150;
        }
      }
      closedir (dir);
    }

    return count_pic;
}

string runFileDialog(bool isSave)
{
    string FileName = "";

    OPENFILENAME ofn;
    TCHAR szFile[260] = {0};

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = txWindow();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = ("Text\0*.TXT\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box.
    if(isSave)
    {
        if (GetSaveFileName(&ofn)==TRUE)
        {
           FileName = ofn.lpstrFile;
           FileName = FileName + ".txt";

        }
    }
    else
    {
        if (GetOpenFileName(&ofn)==TRUE)
        {
           FileName = ofn.lpstrFile;
        }
    }
    return FileName;
}

inline int GetFilePointer(HANDLE FileHandle)
{
    return SetFilePointer(FileHandle, 0, 0, FILE_CURRENT);
}

bool SaveBMPFile(char *filename, HBITMAP bitmap, HDC bitmapDC, int width, int height)
{
    bool Success=0;
    HBITMAP OffscrBmp=NULL;
    HDC OffscrDC=NULL;
    LPBITMAPINFO lpbi=NULL;
    LPVOID lpvBits=NULL;
    HANDLE BmpFile=INVALID_HANDLE_VALUE;
    BITMAPFILEHEADER bmfh;
    if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
        return 0;
    if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
        return 0;
    HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);
    BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL)
        return 0;
    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    SelectObject(OffscrDC, OldBmp);
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
        return 0;
    if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
        return 0;
    if ((BmpFile = CreateFile(filename,
                        GENERIC_WRITE,
                        0, NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL)) == INVALID_HANDLE_VALUE)
        return 0;
    DWORD Written;
    bmfh.bfType = 19778;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;
    if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL))
        return 0;
    if (Written < sizeof(BITMAPINFOHEADER))
        return 0;
    int PalEntries;
    if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
        PalEntries = 3;
    else PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
                      (int)(1 << lpbi->bmiHeader.biBitCount) : 0;
    if(lpbi->bmiHeader.biClrUsed)
    PalEntries = lpbi->bmiHeader.biClrUsed;
    if(PalEntries){
    if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL))
        return 0;
        if (Written < PalEntries * sizeof(RGBQUAD))
            return 0;
    }
    bmfh.bfOffBits = GetFilePointer(BmpFile);
    if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL))
        return 0;
    if (Written < lpbi->bmiHeader.biSizeImage)
        return 0;
    bmfh.bfSize = GetFilePointer(BmpFile);
    SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        return 0;
    if (Written < sizeof(bmfh))
        return 0;

    CloseHandle (BmpFile);

    delete [] (char*)lpvBits;
    delete [] lpbi;

    DeleteDC (OffscrDC);
    DeleteObject (OffscrBmp);


    return 1;
}

bool ScreenCapture(int x, int y, int width, int height, char *filename, HWND hwnd)
{
    HDC hDC = GetDC(hwnd);
    HDC hDc = CreateCompatibleDC(hDC);

    HBITMAP hBmp = CreateCompatibleBitmap(hDC, width, height);

    HGDIOBJ old= SelectObject(hDc, hBmp);
    BitBlt(hDc, 0, 0, width, height, hDC, x, y, SRCCOPY);

    bool ret = SaveBMPFile(filename, hBmp, hDc, width, height);

    SelectObject(hDc, old);

    DeleteObject(hBmp);

    DeleteDC (hDc);
    ReleaseDC (hwnd, hDC);

    return ret;
}

HDC Fon1 = txLoadImage("Pictures/Fon.bmp");
const int count_btn = 12;
const int btn_prtsc = count_btn-6;
const int btn_save = count_btn-5;
const int btn_load = count_btn-4;
const int btn_help = count_btn-3;
const int btn_exit = count_btn-2;
const int btn_menu = count_btn-1;
const int page_menu = 0;
const int page_redactor = 1;
const int page_help = 2;

int main()
{
    txCreateWindow (1250, 800);
    txDisableAutoPause();
    //������������� ������
    //int count_btn=8;
    int count_pic=0;
    char str[100];

    int page = page_menu;

    Button btn[count_btn];
    btn[0] = {50, 30, "build", "build"};
    btn[1] = {250, 30, "defense", "defense"};
    btn[2] = {450, 30, "resurces", "resurces"};
    btn[3] = {650, 30, "other", "other"};
    btn[4] = {850, 30, "trap", "trap"};
    btn[5] = {1050, 30, "heroes", "heroes"};
    btn[6] = {1000,450, "������ ������", ""};
    btn[7] = {1000,500, "���������", ""};
    btn[8] = {1000,550, "���������", ""};
    btn[9] = {1000,600, "�������", ""};
    btn[10] = {1000,650, "�����", ""};
    btn[11] = {530,350, "������", ""};

    Picture menuPic[100];

    count_pic = readFromDir("pictures/build/", menuPic, count_pic);
    count_pic = readFromDir("pictures/defense/", menuPic, count_pic);
    count_pic = readFromDir("pictures/resurces/", menuPic, count_pic);
    count_pic = readFromDir("pictures/other/", menuPic, count_pic);
    count_pic = readFromDir("pictures/trap/", menuPic, count_pic);
    count_pic = readFromDir("pictures/heroes/", menuPic, count_pic);

    for(int i=0; i<count_pic; i++)
    {
        menuPic[i].pic = txLoadImage(menuPic[i].adress.c_str());

        menuPic[i].w = get_w(menuPic[i].adress);
        menuPic[i].h = get_h(menuPic[i].adress);

        menuPic[i].w_scr = menuPic[i].w/1.5;
        menuPic[i].h_scr = menuPic[i].h/1.5;

        menuPic[i].visible = false;

        string str = menuPic[i].adress;
        int pos1 = str.find("/");
        int pos2 = str.find("/", pos1+1);
        menuPic[i].category = str.substr(pos1+1, pos2-pos1-1);
    }

    Picture centrPic[100];

    Fon1 = txLoadImage ("pictures/Fon.bmp");

    int vybor = -1;
    bool mouse_click = false;
    int nCentrPic = 0;
    int npic = 0;

    while(!btn[btn_exit].Click())
    {
        txBegin();
        txClear();
        txTransparentBlt (txDC(), 0, 0, 1250, 940, Fon1);

         if(page == page_menu)
        {
            btn[btn_menu].Draw();
            btn[btn_help].Draw();
            if(btn[btn_menu].Click())
            {
               page = page_redactor;
               txSleep(100);
            }
            if(btn[btn_help].Click())
            {
               page = page_help;
               btn[btn_help].name = "������";
               txSleep(100);
            }
        }


        if(page == page_redactor)
        {
            txSetColor(TX_BLACK, 5);
            txSetFillColor(TX_NULL);
            txRectangle(250, 100, 950, 700);

            txSetColor(TX_BLACK);
            txSetFillColor(TX_YELLOW);

            for(int i=0; i<count_btn-1; i++)
            {
                btn[i].Draw();
            }

            for(int i=0; i<count_pic; i++)
            {
                menuPic[i].Draw();
            }

            for(int i=0; i<nCentrPic; i++)
            {
                centrPic[i].Draw();
            }

           for(int ib=0; ib<count_btn; ib++)
            {
                if(btn[ib].Click())
                {
                    for(int ip=0; ip<count_pic; ip++)
                    {
                        menuPic[ip].visible = false;
                        if(menuPic[ip].category == btn[ib].category)
                        {
                            menuPic[ip].visible = true;
                        }
                    }
                }
            }

            for(int npic=0; npic<count_pic; npic++)
            {
                if(menuPic[npic].Click() && menuPic[npic].visible)
                {
                    while(txMouseButtons() == 1)
                    {
                        txSleep(10);
                    }
                    centrPic[nCentrPic]  = {500,
                                            100,
                                            menuPic[npic].adress,
                                            menuPic[npic].pic,
                                            menuPic[npic].w,
                                            menuPic[npic].h,
                                            menuPic[npic].w,
                                            menuPic[npic].h,
                                            menuPic[npic].visible,
                                            menuPic[npic].category};

                    nCentrPic ++;
                }
            }


            for(int i=0; i<nCentrPic; i++)
            {
                if(centrPic[i].Click() && centrPic[i].visible)
                {
                    vybor = i;
                    mouse_click = false;
                }
            }

            if(vybor>=0 && GetAsyncKeyState (VK_DELETE))
            {
                centrPic[vybor] = centrPic[nCentrPic-1];
                nCentrPic--;
                vybor = -1;
                mouse_click = true;
            }


            /*char str[10];
            sprintf(str, "������ ���� ���  = %d", vybor);
            txTextOut(50, 650, str);*/


            if(vybor>=0)
            {
                if (GetAsyncKeyState (VK_LEFT))
                {
                    centrPic[vybor].x -= 5;
                }

                if (GetAsyncKeyState (VK_RIGHT))
                {
                    centrPic[vybor].x += 5;
                }

                if (GetAsyncKeyState (VK_UP))
                    {
                    centrPic[vybor].y -= 5;
                }

                if (GetAsyncKeyState (VK_DOWN))
                {
                    centrPic[vybor].y += 5;
                }

                if (GetAsyncKeyState (VK_OEM_PLUS) || GetAsyncKeyState(VK_ADD))
                {
                    centrPic[vybor].w_scr = centrPic[vybor].w_scr * 1.1;
                    centrPic[vybor].h_scr = centrPic[vybor].h_scr * 1.1;
                }

                if (GetAsyncKeyState (VK_OEM_MINUS) || GetAsyncKeyState(VK_SUBTRACT))
                {
                    centrPic[vybor].w_scr = centrPic[vybor].w_scr * 0.9;
                    centrPic[vybor].h_scr = centrPic[vybor].h_scr * 0.9;
                }
            }

            if(vybor>=0)
            {
                if(txMouseButtons() == 1 && !mouse_click)
                {
                    centrPic[vybor].x = txMouseX() - centrPic[vybor].w_scr/2;
                    centrPic[vybor].y = txMouseY() - centrPic[vybor].h_scr/2;
                }
                else
                {
                    if(txMouseButtons() !=1)
                    {
                        mouse_click = true;
                    }
                }
            }

            //����������
            if(btn[btn_save].Click())
            {
                string FileName = runFileDialog(true);

                ofstream fileout;
                fileout.open(FileName);
                if (fileout.is_open())
                {
                    for(int i=0; i<nCentrPic; i++)
                    {
                        if(centrPic[i].visible)
                        {
                            fileout << centrPic[i].x << endl;
                            fileout << centrPic[i].y << endl;
                            fileout << centrPic[i].adress << endl;
                            fileout << centrPic[i].w_scr << endl;
                            fileout << centrPic[i].h_scr << endl;
                        }
                    }
                }
                fileout.close();
            }

            if(btn[btn_load].Click())
            {
                string FileName = runFileDialog(false);

                for (int i=0; i<nCentrPic; i++)
                {
                    centrPic[i].visible = false;
                    nCentrPic = 0;
                }

                char buff[60];
                ifstream filein(FileName);
                while (filein.good())
                {
                    filein.getline(buff, 60);
                    int x = atoi(buff);
                    filein.getline(buff, 60);
                    int y = atoi(buff);
                    filein.getline(buff, 60);
                    string adress = buff;
                    filein.getline(buff, 60);
                    int w_scr = atoi(buff);
                    filein.getline(buff, 60);
                    int h_scr = atoi(buff);


                    for(int i=0; i<count_pic; i++)
                    {
                        if(menuPic[i].adress == adress)
                        {
                            centrPic[nCentrPic]  = {x, y,
                                                    menuPic[i].adress,
                                                    menuPic[i].pic,
                                                    w_scr,
                                                    h_scr,
                                                    menuPic[i].w,
                                                    menuPic[i].h,
                                                    true,
                                                    menuPic[i].category};

                            nCentrPic ++;
                        }
                    }
                }
                filein.close();
            }


                    /*char str[100];
                    sprintf(str, "nCentrPic  = %d", nCentrPic);
                    txTextOut(50, 650, str);*/

                if(btn[btn_help].Click())
                {
                   page = page_help;
                   btn[btn_help].name = "���������";
                   txSleep(100);
                }

                if(btn[btn_prtsc].Click())
                {
                    ScreenCapture(250, 100, 700, 600, "result.bmp", txWindow());
                    txMessageBox("��������� � result.bmp");
                }

        }

        else if(page == page_help)
        {
            txSelectFont("Times New Roman", 42);
            txDrawText(0, 200, 1200, 600,
                        "��������� '��������'.\n\n"
                        "������� ���������, ������, ���������.\n"
                        "���������� �� ������ �����.\n"
                        "�������� ������������� ����������� � ������\n"
                        "��������� '+', ��������� '-'\n"
                        "��������� ����� ��������� � txt-������� � ���������\n"
                        "����� �� ��������� - 'Escape'.\n");
            btn[btn_help].Draw();
            if(btn[btn_help].Click())
            {
               page = page_redactor;
               btn[btn_help].name = "�������";
               txSleep(100);
            }
        }

        txEnd();
        txSleep(10);
        }

    for(int i=0; i<count_pic; i++)
    {
        txDeleteDC (menuPic[i].pic);
    }

    for(int i=0; i<nCentrPic; i++)
    {
        txDeleteDC (centrPic[i].pic);
    }

    txDeleteDC(Fon1);


return 0;
}
