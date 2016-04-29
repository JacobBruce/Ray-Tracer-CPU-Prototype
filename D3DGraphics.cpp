#include "D3DGraphics.h"
#pragma comment(lib, "d3d9.lib")

D3DGraphics::D3DGraphics( HWND hWnd )
	:
pDirect3D( NULL ),
pDevice( NULL ),
pBackBuffer( NULL ),
pSysBuffer( NULL )
{
	HRESULT result;
	
	pDirect3D = Direct3DCreate9( D3D_SDK_VERSION );
	assert( pDirect3D != NULL );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp,sizeof( d3dpp ) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    result = pDirect3D->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,&d3dpp,&pDevice );
	assert( !FAILED( result ) );

	result = pDevice->GetBackBuffer( 0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );
	assert( !FAILED( result ) );

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	pSysBuffer = new D3DCOLOR[ WINDOW_WIDTH * WINDOW_HEIGHT ];
}

D3DGraphics::~D3DGraphics()
{
	if( pDevice )
	{
		pDevice->Release();
		pDevice = NULL;
	}
	if( pDirect3D )
	{
		pDirect3D->Release();
		pDirect3D = NULL;
	}
	if( pBackBuffer )
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}
	if( pSysBuffer )
	{
		delete pSysBuffer;
		pSysBuffer = NULL;
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
}

void D3DGraphics::BeginFrame()
{
	memset( pSysBuffer, FILL_COLOR, sizeof(D3DCOLOR) * WINDOW_WIDTH * WINDOW_HEIGHT );
}

void D3DGraphics::EndFrame()
{
	HRESULT result;

	result = pBackBuffer->LockRect( &backRect,NULL,NULL );
	assert( !FAILED( result ) );

	for( int y = 0; y < WINDOW_HEIGHT; y++ )
	{
		memcpy( &((BYTE*)backRect.pBits)[backRect.Pitch * y],&pSysBuffer[WINDOW_WIDTH * y],sizeof(D3DCOLOR)* WINDOW_WIDTH );
	}

	result = pBackBuffer->UnlockRect( );
	assert( !FAILED( result ) );

	result = pDevice->Present( NULL,NULL,NULL,NULL );
	assert( !FAILED( result ) );
}

void D3DGraphics::PutPixel( int x,int y,int r,int g,int b )
{	
	assert(x >= 0);
	assert(y >= 0);
	assert(x < WINDOW_WIDTH);
	assert(y < WINDOW_HEIGHT);
	pSysBuffer[ x + WINDOW_WIDTH * y ] = D3DCOLOR_XRGB( r,g,b );
}

void D3DGraphics::PutPixel(int x, int y, const D3DCOLOR& c)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < WINDOW_WIDTH);
	assert(y < WINDOW_HEIGHT);
	pSysBuffer[ x + WINDOW_WIDTH * y ] = c;
}

void D3DGraphics::PutPixel(unsigned int index, const D3DCOLOR& c)
{
	assert(index < WINDOW_WIDTH * WINDOW_HEIGHT);
	pSysBuffer[ index ] = c;
}

D3DCOLOR D3DGraphics::GetPixel(int x, int y)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < WINDOW_WIDTH);
	assert(y < WINDOW_HEIGHT);
	return pSysBuffer[ x + WINDOW_WIDTH * y ];
}

D3DCOLOR D3DGraphics::GetPixel(unsigned int index)
{
	assert(index < WINDOW_WIDTH * WINDOW_HEIGHT);
	return pSysBuffer[ index ];
}

void D3DGraphics::DrawLine(int x1, int y1, int x2, int y2, const D3DCOLOR& c)
{
	const int dx = x2 - x1;
	const int dy = y2 - y1;

	if( dy == 0 && dx == 0 )
	{
		PutPixel( x1,y1,c );
	}
	else if( abs( dy ) > abs( dx ) )
	{
		if( dy < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dx / (float)dy;
		const float b = x1 - m*y1;
		for( int y = y1; y <= y2; y = y + 1 )
		{
			int x = (int)(m*y + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
	else
	{
		if( dx < 0 )
		{
			int temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}
		const float m = (float)dy / (float)dx;
		const float b = y1 - m*x1;
		for( int x = x1; x <= x2; x = x + 1 )
		{
			int y = (int)(m*x + b + 0.5f);
			PutPixel( x,y,c );
		}
	}
}

void D3DGraphics::DrawCircle(int cx, int cy, int rad, int r, int g, int b)
{
	float radSqr = rad * rad;
	int x0 = round(0.707106781f * rad);

	for (int x = 0; x <= x0; x++)
	{
		int y = round(sqrt(radSqr - (x*x)));
		PutPixel(cx+x, cy+y, r, g, b);
		PutPixel(cx+y, cy+x, r, g, b);
		PutPixel(cx-x, cy+y, r, g, b);
		PutPixel(cx-y, cy+x, r, g, b);
		PutPixel(cx+x, cy-y, r, g, b);
		PutPixel(cx+y, cy-x, r, g, b);
		PutPixel(cx-x, cy-y, r, g, b);
		PutPixel(cx-y, cy-x, r, g, b);
	}
}

void D3DGraphics::DrawCircle(int cx, int cy, int rad, D3DCOLOR c)
{
	float radSqr = rad * rad;
	int x0 = round(0.707106781f * rad);

	for (int x = 0; x <= x0; x++)
	{
		int y = round(sqrt(radSqr - (x*x)));
		PutPixel(cx+x, cy+y, c);
		PutPixel(cx+y, cy+x, c);
		PutPixel(cx-x, cy+y, c);
		PutPixel(cx-y, cy+x, c);
		PutPixel(cx+x, cy-y, c);
		PutPixel(cx+y, cy-x, c);
		PutPixel(cx-x, cy-y, c);
		PutPixel(cx-y, cy-x, c);
	}
}

void D3DGraphics::DrawDisc(int cx, int cy, int rad, int r, int g, int b)
{
	for (int x = cx - rad; x < cx +rad; x++)
	{
		for (int y = cy - rad; y < cy +rad; y++)
		{
			if (sqrt((float)((x-cx)*(x-cx)+(y-cy)*(y-cy))) < rad)
			{
				PutPixel(x,y,r,g,b);
			}
		}
	}
}

void D3DGraphics::DrawDisc(int cx, int cy, int rad, D3DCOLOR c)
{
	for (int x = cx - rad; x < cx +rad; x++)
	{
		for (int y = cy - rad; y < cy +rad; y++)
		{
			if (sqrt((float)((x-cx)*(x-cx)+(y-cy)*(y-cy))) < rad)
			{
				PutPixel(x,y,c);
			}
		}
	}
}

void D3DGraphics::DrawFlatTriangle( float y0,float y1,float m0,float b0,float m1,float b1,const RGB32& c )
{
	const int yStart = max( (int)( y0 + 0.5f ), 0 );
	const int yEnd = min( (int)( y1 + 0.5f ), WINDOW_HEIGHT );
	int x, y, xStart, xEnd;

	for( y = yStart; y < yEnd; y++ )
	{
		xStart = max( int( m0 * ( float( y ) + 0.5f ) + b0 + 0.5f ), 0 );
		xEnd   = min( int( m1 * ( float( y ) + 0.5f ) + b1 + 0.5f ), WINDOW_WIDTH );

		for( x = xStart; x < xEnd; x++ )
		{
			PutPixel( x,y,c.value );
		}
	}
}

void D3DGraphics::DrawTriangle( Vec2 v0,Vec2 v1,Vec2 v2,const RGB32& c )
{
	if( v1.y < v0.y ) v0.Swap( v1 );
	if( v2.y < v1.y ) v1.Swap( v2 );
	if( v1.y < v0.y ) v0.Swap( v1 );

	if( v0.y == v1.y )
	{
		if( v1.x < v0.x ) v0.Swap( v1 );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		const float m2 = ( v1.x - v2.x ) / ( v1.y - v2.y );
		float b1 = v0.x - m1 * v0.y;
		float b2 = v1.x - m2 * v1.y;
		DrawFlatTriangle( v1.y,v2.y,m1,b1,m2,b2,c );
	}
	else if( v1.y == v2.y )
	{
		if( v2.x < v1.x ) v1.Swap( v2 );
		const float m0 = ( v0.x - v1.x ) / ( v0.y - v1.y );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		float b0 = v0.x - m0 * v0.y;
		float b1 = v0.x - m1 * v0.y;
		DrawFlatTriangle( v0.y,v1.y,m0,b0,m1,b1,c );
	}
	else
	{
		const float m0 = ( v0.x - v1.x ) / ( v0.y - v1.y );
		const float m1 = ( v0.x - v2.x ) / ( v0.y - v2.y );
		const float m2 = ( v1.x - v2.x ) / ( v1.y - v2.y );
		float b0 = v0.x - m0 * v0.y;
		float b1 = v0.x - m1 * v0.y;
		float b2 = v1.x - m2 * v1.y;

		const float qx = m1 * v1.y + b1;

		if( qx < v1.x )
		{
			DrawFlatTriangle( v0.y,v1.y,m1,b1,m0,b0,c );
			DrawFlatTriangle( v1.y,v2.y,m1,b1,m2,b2,c );
		}
		else
		{
			DrawFlatTriangle( v0.y,v1.y,m0,b0,m1,b1,c );
			DrawFlatTriangle( v1.y,v2.y,m2,b2,m1,b1,c );
		}
	}
}

void D3DGraphics::DrawRectangle(const Vec2& topLeft, const Vec2& botRight, const RGB32& c)
{
	unsigned int x, y;
	for (y=topLeft.y; y<botRight.y; y++) {
		for (x=topLeft.x; x<botRight.x; x++) {
			PutPixel(x, y, c.value);
		}
	}
}

void D3DGraphics::DrawChar( char ch,int xo,int yo,Font* font,const RGB32* c )
{
	if( ch < ' ' || ch > '~' ) return;

	const int charPos = ch - ' ';
	const int xPos = (charPos % font->nCharsPerRow);
	const int yPos = charPos / font->nCharsPerRow;
	const int xStart = (xPos * font->charWidth);
	const int yStart = (yPos * font->charHeight);
	const int xEnd = (xStart + font->charWidth);
	const int yEnd = (yStart + font->charHeight);
	double cbright = 1;
	RGB32 cc, cf;

	for (int y = yStart; y < yEnd; y++)
	{
		for (int x = xStart; x < xEnd; x++)
		{
			cc.value = font->GetPixel(x, y);
			if (cc.value != font->key)
			{
				cbright = Brightness(cc);
				cc = *c; cc.alpha = cbright;
				cf.value = GetPixel(x+xo-xStart, y+yo-yStart);
				cf = AlphaBlend(cf, cc);
				PutPixel(x+xo-xStart, y+yo-yStart, cf.value);
			}
		}
	}
}

void D3DGraphics::DrawString( const char* str,int x,int y,int horiPad,Font* font,const RGB32& c )
{
	int xx = x, yy = y;

	while (*str != '\0')
	{
		if (*str == '\n')
		{
			yy += font->charHeight;
			xx = x;
			str++;
			continue;
		}
		DrawChar(*str, xx, yy, font, &c);
		xx += font->charWidth + horiPad;
		str++;
	}
}