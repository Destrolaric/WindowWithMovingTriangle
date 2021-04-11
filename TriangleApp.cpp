#include "TriangleApp.h"
#include <iostream>
using namespace Microsoft::WRL;

template<class Interface>
inline void SafeRelease(
        Interface **ppInterfaceToRelease
) {
    if (*ppInterfaceToRelease != NULL) {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class DPIScale {
    static float scaleX;
    static float scaleY;

public:
    static void Initialize(ID2D1Factory *pFactory) {
        FLOAT dpiX, dpiY;
        dpiX = (FLOAT) GetDpiForWindow(GetDesktopWindow());
        dpiY = dpiX;
        scaleX = dpiX / 96.0f;
        scaleY = dpiY / 96.0f;
    }

    template<typename T>
    static D2D1_POINT_2F PixelsToDips(T x, T y) {
        return D2D1::Point2F(static_cast<float>(x) / scaleX, static_cast<float>(y) / scaleY);
    }
};

float DPIScale::scaleX = 1.0f;
float DPIScale::scaleY = 1.0f;

HRESULT TriangleApp::CreateGraphicsResources() {
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(m_hwnd, size),
                &pRenderTarget);

        if (SUCCEEDED(hr)) {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0, 1.0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr)) {
                CalculateLayout();
            }
        }
    }
    return hr;
}

void TriangleApp::DiscardGraphicsResources() {
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}


void TriangleApp::OnPaint() {
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr)) {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(0x25854b));
        if (triangle.point1.x > 0) {
            ComPtr<ID2D1PathGeometry> geometry = GenTriangleGeometry(
                    D2D1::Point2F((int)triangle.point1.x,(int)triangle.point1.y),
                    D2D1::Point2F((int)triangle.point2.x,(int)triangle.point2.y),
                    D2D1::Point2F((int)triangle.point3.x,(int)triangle.point3.y));

            pRenderTarget->FillGeometry(geometry.Get(), pBrush);
        }
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void TriangleApp::Resize() {
    if (pRenderTarget != NULL) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void TriangleApp::OnLButtonDown(int pixelX, int pixelY, DWORD flags) {
    SetCapture(m_hwnd);
    triangle.point1 = ptMouse = DPIScale::PixelsToDips(pixelX,pixelY);
    triangle.point2.x = triangle.point1.x + 200;
    triangle.point2.y = triangle.point1.y;
    triangle.point3.x = triangle.point1.x + 400;
    triangle.point3.y = triangle.point1.y + 300 ;
    InvalidateRect(m_hwnd, NULL, FALSE);

}

void TriangleApp::OnMouseMove(int pixelX, int pixelY, DWORD flags) {
    if (flags & MK_LBUTTON) {
        triangle.point1 = ptMouse = DPIScale::PixelsToDips(pixelX,pixelY);

        triangle.point2.x = triangle.point1.x + 200;
        triangle.point2.y = triangle.point1.y;
        triangle.point3.x = triangle.point1.x + 400;
        triangle.point3.y = triangle.point1.y + 300 ;

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void TriangleApp::OnLButtonUp() {
    ReleaseCapture();
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    TriangleApp win;

    if (!win.Create(("TriangleApp"), WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

// Run the message loop.

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT TriangleApp::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            if (FAILED(D2D1CreateFactory(
                    D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) {
                return -1;  // Fail CreateWindowEx.
            }
            DPIScale::Initialize(pFactory);
            return 0;

        case WM_DESTROY:
            DiscardGraphicsResources();
            SafeRelease(&pFactory);
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
            OnPaint();
            return 0;

        case WM_SIZE:
            Resize();
            return 0;

        case WM_LBUTTONDOWN:
            OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD) wParam);
            return 0;

        case WM_LBUTTONUP:
            OnLButtonUp();
            return 0;

        case WM_MOUSEMOVE:
            OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD) wParam);
            return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

ComPtr<ID2D1PathGeometry> TriangleApp::GenTriangleGeometry
        (D2D1_POINT_2F pt1, D2D1_POINT_2F pt2, D2D1_POINT_2F pt3) {
    ID2D1GeometrySink *pSink = NULL;
    std::cout << pt1.x << std::endl;
    HRESULT hr = S_OK;
    ComPtr<ID2D1PathGeometry> m_pPathGeometry;
    // Create a path geometry.
    if (SUCCEEDED(hr)) {
        hr = pFactory->CreatePathGeometry
                (m_pPathGeometry.ReleaseAndGetAddressOf());

        if (SUCCEEDED(hr)) {
            // Write to the path geometry using the geometry sink.
            hr = m_pPathGeometry->Open(&pSink);

            if (SUCCEEDED(hr)) {
                pSink->BeginFigure(
                        pt1,
                        D2D1_FIGURE_BEGIN_FILLED
                );

                pSink->AddLine(pt2);


                pSink->AddLine(pt3);

                pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

                hr = pSink->Close();
            }
            SafeRelease(&pSink);
        }
    }
    return m_pPathGeometry;
}
