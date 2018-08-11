#pragma once

class IcoFrameGdi
{
public:

    IcoFrameGdi(PCWSTR filename, int width, int height);
    ~IcoFrameGdi();

    IcoFrameGdi(IcoFrameGdi &) = delete;
    IcoFrameGdi & operator=(IcoFrameGdi &) = delete;

    IcoFrameGdi(IcoFrameGdi &&rhs) :
        m_hicon{ rhs.m_hicon },
        m_width{ rhs.m_width },
        m_height{ rhs.m_height }
    {
        rhs.m_hicon = nullptr;
    }

    IcoFrameGdi & operator=(IcoFrameGdi &&rhs)
    {
        m_hicon = rhs.m_hicon;
        m_width = rhs.m_width;
        m_height = rhs.m_height;

        rhs.m_hicon = nullptr;
    }

    UINT GetWidth() { return m_width; }
    UINT GetHeight() { return m_height; }

    void Paint(HDC hdc, int x, int y, int cx, int cy, int borderWidth, int flags);

private:

    HICON m_hicon = nullptr;

    int m_width = 0;
    int m_height = 0;
};
