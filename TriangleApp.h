#pragma once

#include <windows.h>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "basewin.h"





class TriangleApp : public BaseWindow<TriangleApp>
{
    ID2D1Factory            *pFactory;
    ID2D1HwndRenderTarget   *pRenderTarget;
    ID2D1SolidColorBrush    *pBrush;
    D2D1_POINT_2F           ptMouse;
    D2D1_TRIANGLE           triangle;

    void    CalculateLayout() { }
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();
    void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
    void    OnLButtonUp();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);

public:

    TriangleApp() : pFactory(NULL),pRenderTarget(NULL), pBrush(NULL),
                   ptMouse(D2D1::Point2F())
    {
    }
    LPCSTR  ClassName() const override { return ("Circle Window Class"); }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    Microsoft::WRL::ComPtr<ID2D1PathGeometry>
    GenTriangleGeometry(D2D1_POINT_2F pt1, D2D1_POINT_2F pt2, D2D1_POINT_2F pt3);

};