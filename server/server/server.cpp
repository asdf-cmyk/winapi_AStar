// server.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "server.h"
#include <WinSock2.h>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

#define WM_ASYNC	WM_USER+2
#define CHATLOGLEN 8

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//void checkWin();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static WSADATA wsadata;
	static SOCKET s, cs;
	static char buffer[100];
	static TCHAR msg[200];
	static TCHAR msg2[200][100];
	static SOCKADDR_IN addr = { 0 }, c_addr;
	static TCHAR str[100];
	static int count, count2 = 0;
	int size, msgLen;

	static SOCKET room[2];
	static int roomCnt = 0;
	static int selRet, i, i2;
	static fd_set read, readTmp;
	static TIMEVAL time;
	static SOCKET toClient;

	//std::vector<char[200]>

	switch (message)
	{
	case WM_CREATE:
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		addr.sin_family = AF_INET;
		addr.sin_port = 20;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");

		s = socket(AF_INET, SOCK_STREAM, 0);
		bind(s, (LPSOCKADDR)&addr, sizeof(addr));
		WSAAsyncSelect(s, hWnd, WM_ASYNC, FD_ACCEPT);
		if (listen(s, 5) == -1) return 0;
		FD_ZERO(&read);
		FD_SET(s, &read);

		break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_ACCEPT:
			readTmp = read;
			time.tv_sec = 1;
			time.tv_usec = 0;
			selRet = select(NULL, &readTmp, NULL, NULL, &time);
			if (selRet == SOCKET_ERROR) break;
			if (selRet == 0) break;

			//for (i = 0; i < read.fd_count; i++)
			//{
			//	if (FD_ISSET(read.fd_array[i], &readTmp))
			//	{
			//		if (s == read.fd_array[i])
			//		{
						size = sizeof(c_addr);
						cs = accept(s, (LPSOCKADDR)&c_addr, &size);
						FD_SET(cs, &read);
						WSAAsyncSelect(cs, hWnd, WM_ASYNC, FD_READ);
			//		}
			//	}
			//}
			//i--;

			if (roomCnt < 3)
				room[roomCnt++] = cs;
			break;
		case FD_READ:
			readTmp = read;
			time.tv_sec = 1;
			time.tv_usec = 0;
			selRet = select(NULL, &readTmp, NULL, NULL, &time);
			if (selRet == SOCKET_ERROR) break;
			if (selRet == 0) break;

			for (i = 0; i < read.fd_count; i++)
				if (FD_ISSET(read.fd_array[i], &readTmp))
					break;


			msgLen = recv(read.fd_array[i], buffer, 100, 0);
			if (msgLen == 0)
			{
				FD_CLR(read.fd_array[i], &read);
				closesocket(readTmp.fd_array[i]);
			}
			buffer[msgLen] = NULL;
#ifdef _UNICODE
			msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
			msg[msgLen] = NULL;
			if (read.fd_array[i] == room[0])
				msg[0] = '1';
			else if (read.fd_array[i] == room[1])
				msg[0] = '2';
			for (int i = 0; i <= msgLen; i++)
				msg2[count2][i] = msg[i];
			count2++;
#else
			strcpy_s(msg, buffer);
#endif
			InvalidateRgn(hWnd, NULL, TRUE);
			break;
		default:
			break;
		}
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		/*if (_tcscmp(msg, _T("")))
		TextOut(hdc, 0, 0, msg, (int)_tcslen(msg));*/
		int i = count2 <= CHATLOGLEN ? 0 : count2 - CHATLOGLEN;
		int chatLog = count2 <= CHATLOGLEN ? count2 : i + CHATLOGLEN;
		int j = 0;
		for (i; i < chatLog; i++)
			if (_tcscmp(msg, _T("")))
				TextOut(hdc, 0, 0 + 50 * j++, msg2[i], (int)_tcslen(msg2[i]));
		TCHAR tmpStr[6] = L"ют╥б : ";
		TextOut(hdc, 0, 500, tmpStr, (int)_tcslen(tmpStr));
		TextOut(hdc, 50, 500, str, (int)_tcslen(str));
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CHAR:
		if (wParam == VK_RETURN)
		{
			if (read.fd_array[i] == INVALID_SOCKET)
				return 0;
			else
			{
#ifdef _UNICODE
				msgLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL,
					0, NULL, NULL);
				WideCharToMultiByte(CP_ACP, 0, str, -1, buffer, msgLen,
					NULL, NULL);
#else
				strcpy_s(buffer, str);
#endif
				for (i2 = 0; i2 < read.fd_count; i2++)
					send(read.fd_array[i2], (LPSTR)buffer, strlen(buffer) + 1, 0);
				count = 0;
				return 0;
			}
		}
		str[count++] = wParam;
		str[count] = NULL;
		InvalidateRgn(hWnd, NULL, TRUE);
		return 0;
	case WM_DESTROY:
		closesocket(s);
		WSACleanup();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
