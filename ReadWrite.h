#pragma once
#include <string>
#include <fstream>
#include <assert.h>
#include "Resource.h"

using namespace std;

inline string ErrorCodeToStr(const int ecode) 
{
	string emsg;
	switch (ecode)
	{
	case 060:
		emsg = "Cannot locate font file:\n";
		break;
	case 130:
		emsg = "Cannot locate mesh file:\n";
		break;
	case 110:
		emsg = "Cannot locate material definition file:\n";
		break;
	case 150:
		emsg = "Cannot locate texture map file:\n";
		break;
	case 151:
		emsg = "Invalid LoD for texture map file:\n";
		break;
	case 200:
		emsg = "Cannot locate texture file:\n";
		break;
	default:
		emsg = "n/a";
		break;
	}
	return emsg;
}

inline void HandleFatalError(const int ecode, string emsg)
{
	wstring wemsg = StrToWstr(emsg);

    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)(L"The application encountered a fatal error and needs to close.\nError message: "+wemsg).c_str(),
        (LPCWSTR)(L"Fatal Error ("+IntToWstr(ecode)+L")").c_str(), MB_ICONERROR | MB_OK
    );

	exit(EXIT_FAILURE);
}

inline string readFile(const string filename) 
{
	ifstream sourceFile(filename);
    string sourceCode(istreambuf_iterator<char>(sourceFile),(istreambuf_iterator<char>()));
	return sourceCode;
}