
#include "IcoPeek.h"
#include "IcoFrameGdi.h"

IcoFrameGdi::IcoFrameGdi(PCWSTR filename, int width, int height)
{
    m_hicon = reinterpret_cast<HICON>(LoadImageW(nullptr, filename, IMAGE_ICON, width, height, LR_CREATEDIBSECTION | LR_LOADFROMFILE));
    if (!m_hicon)
        return;

    ICONINFOEXW iconInfo{ sizeof(iconInfo) };
    GetIconInfoExW(m_hicon, &iconInfo);

    BITMAP dib{};
    if (GetObjectW(iconInfo.hbmColor, sizeof(dib), &dib))
    {
        this->m_width = dib.bmWidth;
        this->m_height = dib.bmHeight;
    }
}

IcoFrameGdi::~IcoFrameGdi()
{
    if (m_hicon)
        DeleteObject(m_hicon);
}

void IcoFrameGdi::Paint(HDC hdc, int x, int y, int cx, int cy, int borderWidth, int flags)
{
    if (borderWidth > 0)
        Rectangle(hdc, x - 1, y - 1, x + cx + borderWidth, y + cy + borderWidth);

    if (cx >= 1 && cy >= 1)
        DrawIconEx(hdc, x, y, m_hicon, cx, cy, 0, nullptr, DI_NOMIRROR | flags);
}
