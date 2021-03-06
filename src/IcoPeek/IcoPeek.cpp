// IcoPeek.cpp : Defines the entry point for the application.
//

#include "IcoPeek.h"
#include "IcoFrameGdi.h"
#include "IcoFrameWic.h"

#define MAX_LOADSTRING 100

HINSTANCE g_hInstance = nullptr;
HWND g_hMainWindow = nullptr;
HWND g_hLoadIcoButton = nullptr;
HWND g_hResetButton = nullptr;
HWND g_hZoomTrackbar = nullptr;
HWND g_hZoomText = nullptr;

std::vector<IcoFrameGdi> g_Frames;
std::vector<IcoFrameWic> g_FramesWic;

HFONT g_hFont = nullptr;

HPEN g_Pen = nullptr;
HBRUSH g_Background = nullptr;
int g_PenWidth = 2;

int g_DpiScale = USER_DEFAULT_SCREEN_DPI;

int g_ZoomFactor = 100;

IWICImagingFactory *g_wic = nullptr;

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

enum class ChildControls : short
{
    LoadIconButton = 100,
    ResetButton = 101,
    ZoomTrackbar = 102,
    ZoomText = 103,
};

int Dpi(int metric)
{
    return MulDiv(metric, g_DpiScale, USER_DEFAULT_SCREEN_DPI);
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES | ICC_BAR_CLASSES };
    if (!InitCommonControlsEx(&icc))
        return FALSE;

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ICOPEEK, szWindowClass, MAX_LOADSTRING);
    if (0 == MyRegisterClass(hInstance))
        return FALSE;

    hr = BufferedPaintInit();
    if (FAILED(hr))
        return FALSE;

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_wic));
    if (FAILED(hr))
        return FALSE;

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(IDC_ICOPEEK));

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        if (!TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(wcex);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICOPEEK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ICOPEEK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void GetIconArea(_Out_ RECT *rect)
{
    GetClientRect(g_hMainWindow, rect);
    rect->top += Dpi(40);
}

void InvalidateIconArea()
{
    RECT rect;
    GetIconArea(&rect);

    InvalidateRect(g_hMainWindow, &rect, FALSE);
}

void UpdateLayout()
{
    SendMessageW(g_hMainWindow, WM_SETFONT, reinterpret_cast<WPARAM>(g_hFont), FALSE);
    SendMessageW(g_hLoadIcoButton, WM_SETFONT, reinterpret_cast<WPARAM>(g_hFont), FALSE);
    SendMessageW(g_hResetButton, WM_SETFONT, reinterpret_cast<WPARAM>(g_hFont), FALSE);
    SendMessageW(g_hZoomText, WM_SETFONT, reinterpret_cast<WPARAM>(g_hFont), FALSE);

    SetWindowPos(
        g_hLoadIcoButton,
        nullptr,
        Dpi(8), Dpi(8),
        Dpi(100), Dpi(24),
        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);

    SetWindowPos(
        g_hResetButton,
        nullptr,
        Dpi(116), Dpi(8),
        Dpi(100), Dpi(24),
        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);

    SetWindowPos(
        g_hZoomTrackbar,
        nullptr,
        Dpi(232), Dpi(8),
        Dpi(300), Dpi(24),
        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);

    SetWindowPos(
        g_hZoomText,
        nullptr,
        Dpi(536), Dpi(8),
        Dpi(80), Dpi(24),
        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);
}

void SetZoom(int zoom)
{
    g_ZoomFactor = zoom;

    WCHAR text[20];
    swprintf_s(text, L"Zoom %d", zoom);
    SendMessageW(g_hZoomText, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text));

    InvalidateIconArea();
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInstance = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    g_hLoadIcoButton = CreateWindowExW(
        0,
        WC_BUTTON,
        L"Load icon...",
        BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd,
        reinterpret_cast<HMENU>(ChildControls::LoadIconButton),
        g_hInstance,
        nullptr);

    g_hResetButton = CreateWindowExW(
        0,
        WC_BUTTON,
        L"Clear",
        BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd,
        reinterpret_cast<HMENU>(ChildControls::ResetButton),
        g_hInstance,
        nullptr);

    g_hZoomTrackbar = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        L"Zoom",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_AUTOTICKS | TBS_ENABLESELRANGE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd,
        reinterpret_cast<HMENU>(ChildControls::ZoomTrackbar),
        g_hInstance,
        nullptr);

    g_hZoomText = CreateWindowExW(
        0,
        WC_STATICW,
        L"",
        WS_VISIBLE | WS_CHILD,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd,
        reinterpret_cast<HMENU>(ChildControls::ZoomText),
        g_hInstance,
        nullptr);

    SendMessageW(g_hZoomTrackbar, TBM_SETRANGE, FALSE, (LPARAM)(MAKELONG(5, 800)));
    SendMessageW(g_hZoomTrackbar, TBM_SETPOS, FALSE, 100);

    UpdateLayout();
    SetZoom(100);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

static const int iconSizes[] = { 16, 24, 32, 48, 64, 96, 128, 256 };

void SetCurrentIcon(PCWSTR path)
{
    for (auto size : iconSizes)
    {
        g_Frames.emplace_back(path, size, size);
    }

    UINT numFrames = 0;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    auto hr = g_wic->CreateDecoderFromFilename(path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (SUCCEEDED(hr))
        hr = decoder->GetFrameCount(&numFrames);

    for (UINT i = 0; i < numFrames; i++)
    {
        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(i, &frame);
        if (FAILED(hr))
            continue;

        g_FramesWic.emplace_back(g_wic, frame.Get());
    }

    InvalidateIconArea();
}

void OnLoadIcon(HWND hParentWindow)
{
    WCHAR path[MAX_PATH];
    path[0] = L'\0';

    OPENFILENAMEW ofn{ sizeof(ofn) };
    ofn.hwndOwner = hParentWindow;
    ofn.hInstance = g_hInstance;
    ofn.lpstrFilter = L"*.ICO";
    ofn.lpstrTitle = L"Select an ICO file to analyze";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = path;
    ofn.nMaxFile = ARRAYSIZE(path);

    if (!GetOpenFileNameW(&ofn))
        return;

    if (path[0] == L'\0')
        return;

    SetCurrentIcon(path);
}

void OnResetIcons()
{
    g_Frames.clear();
    g_FramesWic.clear();
    InvalidateIconArea();
}

void RecreateResources()
{
    if (g_Pen)
        DeleteObject(g_Pen);

    g_PenWidth = Dpi(1);
    g_Pen = CreatePen(PS_SOLID, g_PenWidth, RGB(128, 128, 128));

    if (g_hFont)
        DeleteObject(g_hFont);

    g_hFont = CreateFontW(
        Dpi(18), 0,
        0,
        0,
        FW_DONTCARE,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        VARIABLE_PITCH,
        L"Segoe UI");

    g_Background = CreateHatchBrush(HS_DIAGCROSS, RGB(170, 170, 170));

    UpdateLayout();
}

void OnPaint(HDC hdc, RECT const &clip)
{
    SelectObject(hdc, GetStockObject(NULL_PEN));
    SelectObject(hdc, g_Background);
    SetBkColor(hdc, RGB(200, 200, 200));
    Rectangle(hdc, 0, Dpi(40), clip.right, clip.bottom);

    auto x = Dpi(20);
    auto maxY = Dpi(42);

    SelectObject(hdc, g_Pen);
    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

    for (auto &f : g_Frames)
    {
        auto y = Dpi(42);

        if (x > clip.right || y > clip.bottom)
            continue;

        auto width = f.GetWidth() * g_ZoomFactor / 100;
        auto height = f.GetHeight() * g_ZoomFactor / 100;

        f.Paint(hdc, x, y, width, height, g_PenWidth, DI_NORMAL);
        y += height + 3 * g_PenWidth;
        f.Paint(hdc, x, y, width, height, g_PenWidth, DI_MASK);
        y += height + 3 * g_PenWidth;
        f.Paint(hdc, x, y, width, height, g_PenWidth, DI_IMAGE);
        x += width + 3 * g_PenWidth;
        y += height + 3 * g_PenWidth;

        maxY = max(y, maxY);
    }

    x = Dpi(20);

    for (auto &f : g_FramesWic)
    {
        auto y = maxY;

        if (x > clip.right || y > clip.bottom)
            continue;

        auto width = f.GetWidth() * g_ZoomFactor / 100;
        auto height = f.GetHeight() * g_ZoomFactor / 100;

        f.Paint(hdc, x, y, width, height, g_PenWidth);

        x += width + 3 * g_PenWidth;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            auto code = HIWORD(wParam);
            switch (static_cast<ChildControls>(LOWORD(wParam)))
            {
            case ChildControls::LoadIconButton:
                switch (code)
                {
                case BN_CLICKED:
                    OnLoadIcon(hWnd);
                    return TRUE;
                }
                break;
            case ChildControls::ResetButton:
                switch (code)
                {
                case BN_CLICKED:
                    OnResetIcons();
                    return TRUE;
                }
                break;
            }
        }
        break;
    case WM_HSCROLL:
        {
            if (reinterpret_cast<HWND>(lParam) == g_hZoomTrackbar && (LOWORD(wParam) == TB_ENDTRACK || LOWORD(wParam) == TB_THUMBTRACK))
            {
                SetZoom(static_cast<int>(SendMessageW(g_hZoomTrackbar, TBM_GETPOS, 0, 0)));
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdcOriginal = BeginPaint(hWnd, &ps);

            RECT clientRect;
            GetIconArea(&clientRect);

            IntersectRect(&clientRect, &clientRect, &ps.rcPaint);

            HDC hdc;
            auto buffered = BeginBufferedPaint(hdcOriginal, &clientRect, BPBF_COMPATIBLEBITMAP, nullptr, &hdc);

            if (buffered)
            {
                OnPaint(hdc, clientRect);
                EndBufferedPaint(buffered, TRUE);
            }
            else
            {
                OnPaint(hdcOriginal, clientRect);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE:
        {
            g_hMainWindow = hWnd;
            g_DpiScale = GetDpiForWindow(hWnd);
            RecreateResources();

            SetCurrentIcon(L"C:\\Users\\jtipp\\Desktop\\ico\\mobility\\infrastructure.ico");
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DPICHANGED:
        {
            g_DpiScale = LOWORD(wParam);

            RecreateResources();

            auto prcNewWindow = reinterpret_cast<RECT const*>(lParam);
            SetWindowPos(
                hWnd,
                NULL,
                prcNewWindow->left,
                prcNewWindow->top,
                prcNewWindow->right - prcNewWindow->left,
                prcNewWindow->bottom - prcNewWindow->top,
                SWP_NOZORDER | SWP_NOACTIVATE);

            InvalidateIconArea();
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
