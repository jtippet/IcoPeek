#pragma once

class IcoFrameWic
{
public:

    IcoFrameWic(IWICImagingFactory *factory, IWICBitmapFrameDecode *frame);
    ~IcoFrameWic();

    IcoFrameWic(IcoFrameWic &) = delete;
    IcoFrameWic & operator=(IcoFrameWic &) = delete;

    IcoFrameWic(IcoFrameWic &&rhs) :
        m_width{ rhs.m_width },
        m_height{ rhs.m_height },
        m_bmi{ rhs.m_bmi },
        m_pixelData{ rhs.m_pixelData },
        m_bitmap{ rhs.m_bitmap }
    {
        rhs.m_pixelData = nullptr;
        rhs.m_bitmap = nullptr;
    }

    IcoFrameWic &operator=(IcoFrameWic &&rhs)
    {
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_bmi = rhs.m_bmi;
        m_pixelData = rhs.m_pixelData;
        m_bitmap = rhs.m_bitmap;

        rhs.m_pixelData = nullptr;
        rhs.m_bitmap = nullptr;
    }

    UINT GetWidth() { return m_width; }
    UINT GetHeight() { return m_height; }

    void Paint(HDC hdc, int x, int y, int cx, int cy, int borderWidth);

private:

    UINT m_width = 0;
    UINT m_height = 0;

    BITMAPINFO m_bmi{};
    void *m_pixelData = nullptr;
    HBITMAP m_bitmap = nullptr;
};
