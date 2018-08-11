
#include "IcoPeek.h"
#include "IcoFrameWic.h"

#define IFC(e) do { hr = (e); if (FAILED(hr)) return; } while(0)

#define PREMULTIPLY(pixel, channel) pixel.channel = (unsigned int)pixel.channel * pixel.rgbReserved / 0xff;

IcoFrameWic::IcoFrameWic(IWICImagingFactory *factory, IWICBitmapFrameDecode *frame)
{
    UNREFERENCED_PARAMETER(factory);

    HRESULT hr;

    IFC(frame->GetSize(&m_width, &m_height));

    Microsoft::WRL::ComPtr<IWICBitmapSource> rgba;
    IFC(WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &rgba));

    m_bmi.bmiHeader.biSize = sizeof(m_bmi);
    m_bmi.bmiHeader.biWidth = m_width;
    m_bmi.bmiHeader.biHeight = -(LONG)m_height;
    m_bmi.bmiHeader.biPlanes = 1;
    m_bmi.bmiHeader.biBitCount = 32;
    m_bmi.bmiHeader.biCompression = BI_RGB;

    void *pixelData = nullptr;

    auto hdc = GetDC(nullptr);
    if (!hdc)
        IFC(E_OUTOFMEMORY);

    m_bitmap = CreateDIBSection(hdc, &m_bmi, DIB_RGB_COLORS, &pixelData, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!m_bitmap)
        IFC(E_OUTOFMEMORY);

    auto cbStride = m_width * 4;
    auto cbBytes = cbStride * m_height;
    IFC(rgba->CopyPixels(nullptr, cbStride, cbBytes, static_cast<BYTE*>(pixelData)));

    m_pixelData = pixelData;
}

IcoFrameWic::~IcoFrameWic()
{
    if (m_bitmap)
        DeleteObject(m_bitmap);
}

void IcoFrameWic::Paint(HDC hdc, int x, int y, int cx, int cy, int borderWidth)
{
    if (borderWidth > 0)
        Rectangle(hdc, x - 1, y - 1, x + cx + borderWidth, y + cy + borderWidth);

    if (m_pixelData)
    {
        auto srcDc = CreateCompatibleDC(hdc);
        if (srcDc)
        {
            BLENDFUNCTION blend{};
            blend.BlendOp = AC_SRC_OVER;
            blend.BlendFlags = 0;
            blend.SourceConstantAlpha = 255;
            blend.AlphaFormat = AC_SRC_ALPHA;

            SelectObject(srcDc, m_bitmap);
            AlphaBlend(hdc, x, y, cx, cy, srcDc, 0, 0, m_width, m_height, blend);
            DeleteDC(srcDc);
        }
    }
}
