// client.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "client.h"
#include <WinSock2.h>
#include <iostream>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "msimg32.lib")

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 800
#define WM_ASYNC	WM_USER+2
#define CHATLOGNUM 5
#define BOARDMAX 19

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

bool sizeChangeFlag = 0;
HBITMAP hBackImage, hBack_blackStone, hBack_whiteStone, hBack_win, hBack_lose;
BITMAP bitBack, bitBack_blackStone, bitBack_whiteStone, bitBack_win, bitBack_lose;
HBITMAP hTransparentImage;
BITMAP bitTransparent;
RECT rectView;
HBITMAP hDoubleBufferImage;

struct gamePlay
{
	unsigned short header;
	int x;
	int y;
	//bool playerColor;
} playData;

unsigned short boardState[BOARDMAX][BOARDMAX];
int xGrid, yGrid, xI, yI;
//LONG xPtList[BOARDMAX];
//LONG yPtList[BOARDMAX];

WSADATA wsadata;
SOCKET s;
char buffer[100];
SOCKADDR_IN addr = { 0 };
TCHAR msg[200];
TCHAR msg2[200][100];
TCHAR playMsg[8];
TCHAR str[100];
int count, chatLogNum = 0;
//int msgLen;
bool youWin = false;
bool isPlaying = false;
unsigned short myColor = 0;

POINT clkPt;
char toA[10];
int gameState = 2;

std::vector<gamePlay> playLog_black;
std::vector<gamePlay> playLog_white;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

bool InitConnect(HWND);
bool Async_read();
bool LayStone(POINT);

void CreateBitmap();
void DrawBitmapDoubleBuffering(HWND, HDC);
void DeleteBitmap();

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
	LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

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

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
		CW_USEDEFAULT, 0, WINDOWWIDTH, WINDOWHEIGHT, nullptr, nullptr, hInstance, nullptr);

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
	/*static WSADATA wsadata;
	static SOCKET s;
	static char buffer[100];
	static SOCKADDR_IN addr = { 0 };
	static TCHAR msg[200];
	static TCHAR msg2[200][100];
	static TCHAR playMsg[8];
	static TCHAR str[100];
	static int count, chatLogNum = 0;
	int msgLen;

	static POINT clkPt;
	char toA[10];
	static int gameState = 1;*/

	switch (message)
	{
	case WM_CREATE:
		CreateBitmap();
		GetClientRect(hWnd, &rectView);
		xGrid = floor((double)(bitBack.bmWidth - 25) / BOARDMAX + 0.5);
		yGrid = floor((double)(bitBack.bmHeight - 25) / BOARDMAX + 0.5);
		/*for (int i = 0; i < BOARDMAX; i++)
		{
		xPtList[i] = xGrid * i;
		yPtList[i] = yGrid * i;
		}*/
		if (!InitConnect(hWnd)) return 0;
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
		/*int i = chatLogNum <= CHATLOGNUM ? 0 : chatLogNum - CHATLOGNUM;
		int chatLog = chatLogNum <= CHATLOGNUM ? chatLogNum : i + CHATLOGNUM;
		int j = 0;
		for (i; i < chatLog; i++)
		if (_tcscmp(msg, _T("")))
		TextOut(hdc, 0, 0 + 50 * j++, msg2[i], (int)_tcslen(msg2[i]));
		TCHAR tmpStr[6] = L"ют╥б : ";
		TextOut(hdc, 0, 500, tmpStr, (int)_tcslen(tmpStr));
		TextOut(hdc, 50, 500, str, (int)_tcslen(str));*/
		DrawBitmapDoubleBuffering(hWnd, hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_ASYNC:
		switch (lParam)
		{
		case FD_READ:
			/*msgLen = recv(s, buffer, 100, 0);
			buffer[msgLen] = NULL;
			#ifdef _UNICODE
			msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
			MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), msg, msgLen);
			msg[msgLen] = NULL;
			for (int i = 0; i <= msgLen; i++)
			msg2[chatLogNum][i] = msg[i];
			chatLogNum++;
			#else
			strcpy_s(msg, buffer);
			#endif*/
			if (!Async_read())
				gameState = 3;
			InvalidateRgn(hWnd, NULL, TRUE);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if (gameState != 2)	return 0;
		GetCursorPos(&clkPt);
		ScreenToClient(hWnd, &clkPt);
		if (!LayStone(clkPt))
			return 0;

		//#ifdef _UNICODE
		//		msgLen = WideCharToMultiByte(CP_ACP, 0, str, -1,
		//			NULL, 0, NULL, NULL);
		//		WideCharToMultiByte(CP_ACP, 0, str, -1, buffer,
		//			msgLen, NULL, NULL);
		//#else
		//		strcpy_s(buffer, str)
		//			msgLen = strlen(buffer);
		//#endif
		//		send(s, (LPSTR)buffer, msgLen + 1, 0);
		break;
		/*case WM_CHAR:
		if (wParam == VK_RETURN)
		if (s == INVALID_SOCKET)
		return 0;
		else
		{
		#ifdef _UNICODE
		msgLen = WideCharToMultiByte(CP_ACP, 0, str, -1,
		NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, str, -1, buffer,
		msgLen, NULL, NULL);
		#else
		strcpy_s(buffer, str)
		msgLen = strlen(buffer);
		#endif
		send(s, (LPSTR)buffer, msgLen + 1, 0);
		count = 0;
		return 0;
		}
		str[count++] = wParam;
		str[count] = NULL;
		InvalidateRgn(hWnd, NULL, TRUE);
		break;*/
		//return 0;
	case WM_DESTROY:
		DeleteBitmap();
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

bool InitConnect(HWND hWnd)
{
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	s = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = 20;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	WSAAsyncSelect(s, hWnd, WM_ASYNC, FD_READ);
	if (connect(s, (LPSOCKADDR)&addr, sizeof(addr)) ==
		SOCKET_ERROR)
		return false;

	return true;
}

bool Async_read()
{
	int msgLen = recv(s, buffer, 100, 0);
	buffer[msgLen] = NULL;

	if (buffer[0] == 1)
	{
		msgLen = MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), NULL, NULL);
		MultiByteToWideChar(CP_ACP, 0, buffer, strlen(buffer), playMsg, msgLen);
		playMsg[msgLen] = NULL;

		playData.x = (int)buffer[1] * 10 + (int)buffer[2];
		playData.y = (int)buffer[3] * 10 + (int)buffer[4];
		if (!buffer[5])
			boardState[playData.x][playData.y] = 1;
		else
			boardState[playData.x][playData.y] = 2;

		if (!buffer[5])
			playLog_black.push_back(playData);
		else
			playLog_white.push_back(playData);

		if (!isPlaying)
		{
			myColor = buffer[5] + 1;
			isPlaying = true;
		}
	}

	else if (buffer[0] == 2)
	{
		//if (myColor == buffer[1] + 1)
		if (buffer[1])
		{
			youWin = true;
			return false;
		}
		//else if (myColor != buffer[1] + 1)
		else if(!buffer[1])
		{
			youWin = false;
			return false;
		}
	}

	return true;
}

bool LayStone(POINT clkPt)
{
	if (clkPt.x < 0 || clkPt.x > bitBack.bmWidth || clkPt.y < 0 || clkPt.y > bitBack.bmHeight)
		return false;
	for (xI = 0; xI < BOARDMAX; xI++)
	{
		if (xI * xGrid + xGrid / 3 > clkPt.x - 25)
			break;
		else if (xI * xGrid + 2 * xGrid / 3 > clkPt.x - 25)
			return false;
	}
	for (yI = 0; yI < BOARDMAX; yI++)
	{
		if (yI * yGrid + yGrid / 3 > clkPt.y - 25)
			break;
		else if (yI * yGrid + 2 * yGrid / 3 > clkPt.y - 25)
			return false;
	}
	if (boardState[xI][yI])
		return false;

	playData.header = 1;
	playData.x = xI;
	playData.y = yI;
	buffer[0] = playData.header;
	memset(buffer + 1, playData.x / 10, 1);
	memset(buffer + 2, playData.x % 10, 1);
	memset(buffer + 3, playData.y / 10, 1);
	memset(buffer + 4, playData.y % 10, 1);
	send(s, (LPSTR)buffer, 5, 0);

	return true;
}


void CreateBitmap()
{
	{
		hBackImage = (HBITMAP)LoadImage(NULL, TEXT("images/board.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBackImage, sizeof(BITMAP), &bitBack);
	}
	{
		hBack_blackStone = (HBITMAP)LoadImage(NULL, TEXT("images/black.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBack_blackStone, sizeof(BITMAP), &bitBack_blackStone);
	}
	{
		hBack_whiteStone = (HBITMAP)LoadImage(NULL, TEXT("images/white.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBack_whiteStone, sizeof(BITMAP), &bitBack_whiteStone);
	}
	{
		hBack_win = (HBITMAP)LoadImage(NULL, TEXT("images/YOUWIN.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBack_win, sizeof(BITMAP), &bitBack_win);
	}
	{
		hBack_lose = (HBITMAP)LoadImage(NULL, TEXT("images/YOULOSE.bmp"),
			IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		GetObject(hBack_lose, sizeof(BITMAP), &bitBack_lose);
	}
}

void DrawBitmapDoubleBuffering(HWND hWnd, HDC hdc)
{
	HDC hMemDC;
	HBITMAP hOldBitmap;
	int bx, by;

	HDC hMemDC2;
	HBITMAP hOldBitmap2;

	hMemDC = CreateCompatibleDC(hdc);
	if (hDoubleBufferImage == NULL || sizeChangeFlag)
	{
		hDoubleBufferImage = CreateCompatibleBitmap(hdc, rectView.right, rectView.bottom);
		sizeChangeFlag = 0;
	}
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hDoubleBufferImage);

	{
		hMemDC2 = CreateCompatibleDC(hMemDC);
		hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBackImage);
		bx = bitBack.bmWidth;
		by = bitBack.bmHeight;

		BitBlt(hMemDC, 0, 0, bx, by, hMemDC2, 0, 0, SRCCOPY);
		SelectObject(hMemDC2, hOldBitmap2);
		DeleteDC(hMemDC2);
	}
	if (playLog_black.size() > 0)
	{
		hMemDC2 = CreateCompatibleDC(hMemDC);
		hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBack_blackStone);
		bx = bitBack_blackStone.bmWidth;
		by = bitBack_blackStone.bmHeight;

		HBRUSH myBrush = (HBRUSH)CreateSolidBrush(RGB(255, 0, 255));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC2, myBrush);

		for (unsigned int i = 0; i < playLog_black.size(); i++)
			TransparentBlt(hMemDC, xGrid * playLog_black[i].x + 11, yGrid * playLog_black[i].y + 11,
				bx, by, hMemDC2, 0, 0, bx, by, RGB(255, 0, 255));

		SelectObject(hMemDC2, oldBrush);
		DeleteObject(myBrush);

		SelectObject(hMemDC2, hOldBitmap2);
		DeleteDC(hMemDC2);
	}
	if (playLog_white.size() > 0)
	{
		hMemDC2 = CreateCompatibleDC(hMemDC);
		hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBack_whiteStone);
		bx = bitBack_whiteStone.bmWidth;
		by = bitBack_whiteStone.bmHeight;

		HBRUSH myBrush = (HBRUSH)CreateSolidBrush(RGB(255, 0, 255));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC2, myBrush);

		for (unsigned int i = 0; i < playLog_white.size(); i++)
			TransparentBlt(hMemDC, xGrid * playLog_white[i].x + 11, yGrid * playLog_white[i].y + 11,
				bx, by, hMemDC2, 0, 0, bx, by, RGB(255, 0, 255));

		SelectObject(hMemDC2, oldBrush);
		DeleteObject(myBrush);

		SelectObject(hMemDC2, hOldBitmap2);
		DeleteDC(hMemDC2);
	}
	if (gameState == 3)
	{
		if (youWin)
		{
			hMemDC2 = CreateCompatibleDC(hMemDC);
			hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBack_win);
			bx = bitBack_win.bmWidth;
			by = bitBack_win.bmHeight;

			BitBlt(hMemDC, rectView.right / 8, rectView.bottom / 4, bx, by, hMemDC2, 0, 0, SRCCOPY);
			SelectObject(hMemDC2, hOldBitmap2);
			DeleteDC(hMemDC2);
		}
		else if(!youWin)
		{
			hMemDC2 = CreateCompatibleDC(hMemDC);
			hOldBitmap2 = (HBITMAP)SelectObject(hMemDC2, hBack_lose);
			bx = bitBack_lose.bmWidth;
			by = bitBack_lose.bmHeight;

			BitBlt(hMemDC, rectView.right / 8, rectView.bottom / 4, bx, by, hMemDC2, 0, 0, SRCCOPY);
			SelectObject(hMemDC2, hOldBitmap2);
			DeleteDC(hMemDC2);
		}
	}

	BitBlt(hdc, 0, 0, rectView.right, rectView.bottom, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

void DeleteBitmap()
{
	DeleteObject(hBackImage);
	DeleteObject(hBack_blackStone);
	DeleteObject(hBack_whiteStone);
	DeleteObject(hBack_win);
	DeleteObject(hBack_lose);
}