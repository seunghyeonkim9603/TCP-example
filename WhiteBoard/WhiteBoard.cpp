#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <stdio.h>

#include <assert.h>
#include <windows.h>
#include <windowsx.h>

#include "resource.h"
#include "framework.h"
#include "WhiteBoard.h"

#include "RingBuffer.h"

#define BUFFER_SIZE (256)
#define SERVER_PORT (25000)
#define WM_SOCKET (WM_USER + 1)

struct Packet
{
	__int32 fromX;
	__int32 fromY;
	__int32 toX;
	__int32 toY;
};

SOCKADDR_IN gServerAddr;
SOCKET gSocket;
RingBuffer gReceiveBuffer;
RingBuffer gSendBuffer;
bool gbIsConnected;

HINSTANCE hInst;
RECT rect;
HDC hBuffer;
HBITMAP hBmpBuffer;
HBITMAP hBmpOldBuffer;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void ProcessSocketMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	const char serverIP[] = "127.0.0.1";

	ZeroMemory(&gServerAddr, sizeof(gServerAddr));
	gServerAddr.sin_family = AF_INET;
	gServerAddr.sin_port = htons(SERVER_PORT);
	gServerAddr.sin_addr.S_un.S_addr = inet_addr(serverIP);

	gSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (gSocket == INVALID_SOCKET)
	{
		return WSAGetLastError();;
	}

	hInst = hInstance;

	WNDCLASSEXW wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WHITEBOARD));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WHITEBOARD);
		wcex.lpszClassName = L"WhiteBoard";
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	}
	RegisterClassExW(&wcex);

	HWND hWnd = CreateWindowW(L"WhiteBoard", L"WhiteBoard", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	int asyncSelectError = WSAAsyncSelect(gSocket, hWnd, WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
	if (asyncSelectError == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}

	gbIsConnected = false;

	int connectError = connect(gSocket, (SOCKADDR*)&gServerAddr, sizeof(gServerAddr));
	if (connectError == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WHITEBOARD));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	WSACleanup();
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int x;
	static int y;
	static bool bIsDrag = false;

	switch (message)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		hBuffer = CreateCompatibleDC(hdc);

		GetWindowRect(hWnd, &rect);

		hBmpBuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		hBmpOldBuffer = (HBITMAP)SelectObject(hBuffer, hBmpBuffer);

		PatBlt(hBuffer, 0, 0, rect.right, rect.bottom, WHITENESS);
		ReleaseDC(hWnd, hdc);
	}
	break;
	case WM_SOCKET:
	{
		ProcessSocketMessage(hWnd, message, wParam, lParam);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		bIsDrag = true;
	}
	break;
	case WM_LBUTTONUP:
	{
		bIsDrag = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (bIsDrag && gbIsConnected)
		{
			char bytes[sizeof(__int16) + sizeof(Packet)];
			Packet packet;
			__int16 packetSize = sizeof(packet);

			packet.fromX = x;
			packet.fromY = y;
			packet.toX = GET_X_LPARAM(lParam);
			packet.toY = GET_Y_LPARAM(lParam);

			memcpy(bytes, &packetSize, sizeof(packetSize));
			memcpy(&bytes[sizeof(packetSize)], &packet, sizeof(packet));

			if (!gSendBuffer.TryEnqueue(bytes, sizeof(bytes)))
			{
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			}
			while (!gSendBuffer.IsEmpty())
			{
				int sendRetval = send(gSocket, gSendBuffer.GetFront(), gSendBuffer.GetSize(), 0);

				if (sendRetval == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						PostMessage(hWnd, WM_CLOSE, 0, 0);
					}
					break;
				}
				gSendBuffer.MoveFront(sendRetval);
			}
		}
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		{
			BitBlt(hdc, 0, 0, rect.right, rect.bottom, hBuffer, 0, 0, SRCCOPY);
		}
		EndPaint(hWnd, &ps);
		break;
	}
	break;
	case WM_DESTROY:
		closesocket(gSocket);

		SelectObject(hBuffer, hBmpOldBuffer);
		DeleteObject(hBuffer);
		DeleteObject(hBmpBuffer);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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

void ProcessSocketMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int recvRetval;
	int sendRetval;

	if (WSAGETSELECTERROR(lParam))
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		return;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
	{
		gbIsConnected = true;
	}
	break;
	case FD_READ:
	{
		while (true)
		{
			recvRetval = recv(gSocket, gReceiveBuffer.GetRear(), gReceiveBuffer.GetUnusedSize(), 0);
			if (recvRetval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					PostMessage(hWnd, WM_CLOSE, 0, 0);
				}
				break;
			}
			gReceiveBuffer.MoveRear(recvRetval);

			while (!gReceiveBuffer.IsEmpty())
			{
				Packet packet;
				UINT16 header;

				if (!gReceiveBuffer.TryPeek((char*)&header, sizeof(header)))
				{
					break;
				}
				assert(header == sizeof(packet));
				if (!gReceiveBuffer.TryPeek((char*)&packet, header))
				{
					break;
				}

				gReceiveBuffer.MoveFront(sizeof(header) + header);

				HDC hdc = GetDC(hWnd);
				{
					MoveToEx(hBuffer, packet.fromX, packet.fromY, nullptr);
					LineTo(hBuffer, packet.toX, packet.toY);

					BitBlt(hdc, 0, 0, rect.right, rect.bottom, hBuffer, 0, 0, SRCCOPY);
				}
				ReleaseDC(hWnd, hdc);
			}
		}
	}
	break;
	case FD_WRITE:
	{
		while (!gSendBuffer.IsEmpty())
		{
			sendRetval = send(gSocket, gSendBuffer.GetFront(), gSendBuffer.GetSize(), 0);

			if (sendRetval == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					PostMessage(hWnd, WM_CLOSE, 0, 0);
				}
				break;
			}
			gSendBuffer.MoveFront(sendRetval);
		}
	}
	break;
	case FD_CLOSE:
	{
		closesocket(gSocket);
	}
	break;
	default:
		assert(false);
		break;
	}
}