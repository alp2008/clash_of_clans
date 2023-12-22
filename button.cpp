#include "TXLib.h"
struct Button
{
    int x;
    int y;
    const char* name;
    string category;

    void Draw()
    {
        txSetColor(TX_TRANSPARENT);
        txSetFillColor(TX_GREY);
        Win32::RoundRect (txDC(), x+5, y+5, x+155, y+35, 30, 30);
        txSetColor(TX_BLACK, 3);
        txSetFillColor(TX_WHITE);
        if (Click()) txSetFillColor(RGB(250, 180, 180));
        Win32::RoundRect (txDC(), x, y, x+150, y+30, 30, 30);
        txSetColor(TX_BLACK);
        txSelectFont("Arial", 24);
        txDrawText(x, y, x+150, y+30, name);
    }

    bool Click()
    {
        return(txMouseButtons() == 1 &&
        txMouseX()>= x && txMouseX()<=x+150 &&
        txMouseY()>= y+5 && txMouseY()<=y+35);
    }
};
