#pragma once
#include <queue>

class MouseServer;

class MouseEvent
{
public:
	enum Type
	{
		LPress,
		LRelease,
		RPress,
		RRelease,
		WheelUp,
		WheelDown,
		Move,
		Invalid
	};
private:
	const Type type;
	const int x;
	const int y;
public:
	MouseEvent( Type type,int x,int y )
		:
		type( type ),
		x( x ),
		y( y )
	{}
	bool IsValid() const
	{
		return type != Invalid;
	}
	Type GetType() const
	{
		return type;
	}
	int GetX() const
	{
		return x;
	}
	int GetY() const
	{
		return y;
	}
};

class MouseClient
{
public:
	MouseClient( MouseServer& server );
	int GetMouseX() const;
	int GetMouseY() const;
	void SetMousePos(int mX, int mY) const;
	bool LeftIsPressed() const;
	bool RightIsPressed() const;
	bool IsInWindow() const;
	MouseEvent ReadMouse();
	bool MouseEmpty() const;
private:
	MouseServer& server;
};

class MouseServer
{
	friend MouseClient;
public:
	MouseServer();
	void OnMouseMove( int x,int y );
	void OnMouseLeave();
	void OnMouseEnter();
	void OnLeftPressed( int x,int y );
	void OnLeftReleased( int x,int y );
	void OnRightPressed( int x,int y );
	void OnRightReleased( int x,int y );
	void OnWheelUp( int x,int y );
	void OnWheelDown( int x,int y );
	bool IsInWindow() const;
private:
	int x;
	int y;
	bool leftIsPressed;
	bool rightIsPressed;
	bool isInWindow;
	static const int bufferSize = 4;
	std::queue< MouseEvent > buffer;
};