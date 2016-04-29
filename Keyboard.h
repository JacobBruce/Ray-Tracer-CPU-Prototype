#pragma once
#include <Windows.h>
#include <queue>

class KeyEvent
{
public:
	enum EventType
	{
		Press,
		Release,
		Invalid
	};
private:
	EventType type;
	unsigned char code;
public:
	KeyEvent( EventType type,unsigned char code )
		:
	type( type ),
	code( code )
	{}
	bool IsPress() const
	{
		return type == Press;
	}
	bool IsRelease() const
	{
		return type == Release;
	}
	bool IsValid() const
	{
		return type != Invalid;
	}
	unsigned char GetCode() const
	{
		return code;
	}
};

class KeyboardServer;

class KeyboardClient
{
public:
	KeyboardClient( KeyboardServer& kServer );
	bool KeyIsPressed( unsigned char keycode ) const;
	bool NumlockOn() const;
	bool CapslockOn() const;
	KeyEvent ReadKey();
	KeyEvent PeekKey() const;
	bool KeyEmpty() const;
	unsigned char ReadChar();
	unsigned char PeekChar() const;
	bool CharEmpty() const;
	void FlushKeyBuffer();
	void FlushCharBuffer();
	void FlushBuffers();
private:
	KeyboardServer& server;
};

class KeyboardServer
{
	friend KeyboardClient;
public:
	KeyboardServer();
	void OnKeyPressed( unsigned char keycode );
	void OnKeyReleased( unsigned char keycode );
	void OnChar( unsigned char character );
private:
	static const int nKeys = 256;
	static const int bufferSize = 4;
	bool keystates[ nKeys ];
	std::queue<KeyEvent> keybuffer;
	std::queue<unsigned char> charbuffer;
};