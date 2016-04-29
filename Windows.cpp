#include <Windows.h>
#include <wchar.h>
#include "resource.h"
#include "MathExt.h"
#include "Game.h"
#include "Mouse.h"
#include "D3DGraphics.h"

static KeyboardServer kServ;
static MouseServer mServ;

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

		// ************ KEYBOARD MESSAGES ************ //
		case WM_KEYDOWN:
			if( !( lParam & 0x40000000 ) ) // no thank you on the autorepeat
			{
				kServ.OnKeyPressed( wParam );
			}
			break;
		case WM_KEYUP:
   			kServ.OnKeyReleased( wParam );
			break;
		case WM_CHAR:
			kServ.OnChar( wParam );
			break;
		// ************ END KEYBOARD MESSAGES ************ //

		// ************ MOUSE MESSAGES ************ //
		case WM_MOUSEMOVE:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			if( x > 0 && x < WINDOW_WIDTH && y > 0 && y < WINDOW_HEIGHT )
			{
				mServ.OnMouseMove(x, y);
				if( !mServ.IsInWindow() )
				{
					SetCapture( hWnd );
					mServ.OnMouseEnter();
				}
			}
			else
			{
				if( wParam & (MK_LBUTTON | MK_RBUTTON) )
				{
					x = max(0, x);
					x = min(WINDOW_WIDTH - 1, x);
					y = max(0, y);
					y = min(WINDOW_HEIGHT - 1, y);
					mServ.OnMouseMove(x, y);
				}
				else
				{
					ReleaseCapture();
					mServ.OnMouseLeave();
					mServ.OnLeftReleased(x, y);
					mServ.OnRightReleased(x, y);
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			mServ.OnLeftPressed(x, y);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			mServ.OnRightPressed(x, y);
			break;
		}
		case WM_LBUTTONUP:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			mServ.OnLeftReleased(x, y);
			break;
		}
		case WM_RBUTTONUP:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			mServ.OnRightReleased(x, y);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			int x = (short)LOWORD( lParam );
			int y = (short)HIWORD( lParam );
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				mServ.OnWheelUp(x, y);
			}
			else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			{
				mServ.OnWheelDown(x, y);
			}
			break;
		}
		// ************ END MOUSE MESSAGES ************ //
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

int WINAPI wWinMain( HINSTANCE hInst,HINSTANCE,LPWSTR,INT )
{
	WNDCLASSEX wc = { sizeof( WNDCLASSEX ),CS_CLASSDC,MsgProc,0,0,
                      GetModuleHandle( NULL ),NULL,NULL,NULL,NULL,
                      L"Pray Engine Window", NULL };
	//wc.hIconSm = (HICON)LoadImage(NULL,L"16x16.ico",IMAGE_ICON,16,16,LR_LOADFROMFILE|LR_SHARED|LR_LOADTRANSPARENT);
	//wc.hIcon   = (HICON)LoadImage(NULL,L"32x32.ico",IMAGE_ICON,32,32,LR_LOADFROMFILE|LR_SHARED|LR_LOADTRANSPARENT);
    wc.hIconSm = (HICON)LoadImage( hInst,MAKEINTRESOURCE( IDI_APPICON16 ),IMAGE_ICON,16,16,0 );
	wc.hIcon   = (HICON)LoadImage( hInst,MAKEINTRESOURCE( IDI_APPICON32 ),IMAGE_ICON,32,32,0 );
	wc.hCursor = LoadCursor( NULL,IDC_ARROW );
    RegisterClassEx( &wc );
	
	RECT wr;
	wr.left = WINDOW_LEFT;
	wr.right = WINDOW_WIDTH + wr.left;
	wr.top = WINDOW_TOP;
	wr.bottom = WINDOW_HEIGHT + wr.top;

	AdjustWindowRect( &wr,WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,FALSE );
    HWND hWnd = CreateWindowW( L"Pray Engine Window", L"Pray Engine",
                              WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
							  wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,
                              NULL,NULL,wc.hInstance,NULL );

    ShowWindow( hWnd,SW_SHOWDEFAULT );
    UpdateWindow( hWnd );

	Game theGame( hWnd,kServ,mServ );
	
    MSG msg;
    ZeroMemory( &msg,sizeof( msg ) );
    while( msg.message != WM_QUIT )
    {
        if( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
		{
			theGame.Go();
		}
    }

    UnregisterClass( L"Pray Engine Window",wc.hInstance );
    return 0;
}