// GlobalLighting.cpp: ���������� ����� ����� ��� ����������.
//

#include "stdafx.h"
#include "GlobalLighting.h"

#include "SimpleTracing.h"
#include "Rasterizer.h"

#include "Sphere.h"
#include "Plane.h"
#include "Triangle.h"
#include "Square.h"
#include "Scene.h"

#include "SphereLight.h"
#include "TriangleLight.h"
#include "SquareLight.h"
#include "CompositeLightSource.h"

#include "DiffuseSpecularMaterial.h"

#include <time.h>

using namespace Engine;

IShape* scene;
ILightSource* lights;

#define W 640
#define H 640
#define CAM_Z 6
#define CAM_SIZE 0.5
#define PIXEL_SIZE 1.05
#define WORKERS 8
//#define SEED 0
#define SEED time(0)

Luminance L[W * H];

DWORD ThreadProc(LPVOID lpdwThreadParam);
CRITICAL_SECTION CriticalSection;
bool destroyed = false;
bool inited = false;
bool busy[H];
int frame[H];

#include "MemoryManager.h"

#define MAX_LOADSTRING 100

// ���������� ����������:
HINSTANCE hInst;								// ������� ���������
volatile HWND hWnd;
TCHAR szTitle[MAX_LOADSTRING];					// ����� ������ ���������
TCHAR szWindowClass[MAX_LOADSTRING];			// ��� ������ �������� ����

// ��������� ���������� �������, ���������� � ���� ������ ����:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���������� ��� �����.
	MSG msg;
	HACCEL hAccelTable;

	// ������������� ���������� �����
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GLOBALLIGHTING, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ��������� ������������� ����������:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GLOBALLIGHTING));

	// ���� ��������� ���������:
	while (GetMessage(&msg, NULL, 0, 0))
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
//  �������: MyRegisterClass()
//
//  ����������: ������������ ����� ����.
//
//  �����������:
//
//    ��� ������� � �� ������������� ���������� ������ � ������, ���� �����, ����� ������ ���
//    ��� ��������� � ��������� Win32, �� �������� ������� RegisterClassEx'
//    ������� ���� ��������� � Windows 95. ����� ���� ������� ����� ��� ����,
//    ����� ���������� �������� "������������" ������ ������ � ���������� �����
//    � ����.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLOBALLIGHTING));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0; //MAKEINTRESOURCE(IDC_GLOBALLIGHTING);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �������: InitInstance(HINSTANCE, int)
//
//   ����������: ��������� ��������� ���������� � ������� ������� ����.
//
//   �����������:
//
//        � ������ ������� ���������� ���������� ����������� � ���������� ����������, � �����
//        ��������� � ��������� �� ����� ������� ���� ���������.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // ��������� ���������� ���������� � ���������� ����������

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		50, 20, W, H + 20, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	
	GO_FLOAT kd1[] = {0.9, 0.6, 0.3};
	GO_FLOAT ks1[] = {0, 0, 0};
	int      n1[]  = {0, 0, 0};
	const IMaterial* m1 = new Materials::DuffuseSpecularMaterial(kd1, ks1, n1);

	GO_FLOAT kd2[] = {0.6, 0.1, 1};
	GO_FLOAT ks2[] = {0, 0, 0};
	int      n2[]  = {0, 0, 0};
	const IMaterial* m2 = new Materials::DuffuseSpecularMaterial(kd2, ks2, n2);
	
	GO_FLOAT kd3[] = {0, 0, 0};
	GO_FLOAT ks3[] = {1, 1, 1};
	int      n3[]  = {1000, 1000, 1000};
	const IMaterial* m3 = new Materials::DuffuseSpecularMaterial(kd3, ks3, n3);
	
	GO_FLOAT Le1[] = {20, 20, 20};

	const IShape* shapes[] = {
		//floor
		new Shapes::Square(Vector(-0.5, -0.5, 1), Vector(-0.5, -0.5, 10), Vector(0.5,  -0.5, 1), m1),
		//ceiling
		new Shapes::Square(Vector(-0.5,  0.5, 1), Vector(0.5,   0.5, 1), Vector(-0.5,  0.5, 10), m1),
		//back wall
		new Shapes::Square(Vector(-0.5, -0.5, 10), Vector(-0.5, 0.5, 10), Vector(0.5,  -0.5, 10), m3),
		//left wall
		new Shapes::Square(Vector(-0.5,  0.5, 1), Vector(-0.5, 0.5, 10), Vector(-0.5, -0.5, 1), m1),
		//right wall
		new Shapes::Square(Vector(0.5,  0.5, 1), Vector(0.5, -0.5, 1), Vector(0.5, 0.5, 10), m1),
		
		new Shapes::Sphere(Vector(0, -0.4, 8), 0.1, m1),
		new Shapes::Sphere(Vector(-0.3, 0, 3), 0.15, m2),
		new Shapes::Sphere(Vector(-0.3, -0.2, 4), 0.3, m3),
	};
	
	const ILightSource* lightSources[] = {
		new Lights::Square(Vector(-0.15, 0.45, 3.35), Vector(0.15,  0.45, 3.35), Vector(-0.15, 0.45, 3.65), Luminance(Le1)),
		new Lights::Square(Vector(-0.15, 0.45, 8.35), Vector(0.15,  0.45, 8.35), Vector(-0.15, 0.45, 8.65), Luminance(Le1)),
		//new Lights::Sphere(Vector(0, 0.5, 1.5), 0.1, Luminance(Le1)),
		//new Lights::Sphere(Vector(-0.3, -0.3, 1.5), 0.05, Luminance(Le1)),
	};
	
	scene = new Shapes::Scene(sizeof(shapes) / sizeof(IShape*), shapes);
	lights = new Lights::CompositeLightSource(sizeof(lightSources) / sizeof(ILightSource*), lightSources);
	
	srand(SEED);
	
	ZeroMemory(frame, sizeof(frame));
	ZeroMemory(busy, sizeof(busy));
	ZeroMemory(L, sizeof(L));

	InitializeCriticalSection(&CriticalSection);

	for(int i = 0; i< WORKERS; i++)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadProc, 0, 0, 0);
	}
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	inited = true;
	return TRUE;
}

//
//  �������: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����������:  ������������ ��������� � ������� ����.
//
//  WM_COMMAND	- ��������� ���� ����������
//  WM_PAINT	-��������� ������� ����
//  WM_DESTROY	 - ������ ��������� � ������ � ���������.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT r;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// ��������� ����� � ����:
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
		break;
	case WM_PAINT:
		if(GetUpdateRect(hWnd, &r, false))
		{
			hdc = BeginPaint(hWnd, &ps);

			if(inited)
			{
				for(int j = r.top; j <= r.bottom && j < H; j++)
				{
					for(int i = r.left; i <= r.right && i < W; i++)
					{
						Luminance l = L[i * H + j] * (255.0 / frame[j]);
						SetPixel(hdc, i, j, RGB(l.r() > 255 ? 255 : l.r(),
												l.g() > 255 ? 255 : l.g(),
												l.b() > 255 ? 255 : l.b()
												));
					}

					SetPixel(hdc, 10, j, RGB((frame[j] % 2) * 255, 0, ((frame[j] + 1) % 2) * 255));
				}
			}
			else
			{
				r.left = 0;
				r.top = 0;
				r.bottom = H - 1;
				r.right = W - 1;
				FillRect(hdc, &r, (HBRUSH) (GetStockObject(BLACK_BRUSH)));
			}
			EndPaint(hWnd, &ps);
		}

		break;
	case WM_DESTROY:
		destroyed = true;
		PrintMemoryTable();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���������� ��������� ��� ���� "� ���������".
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



DWORD ThreadProc(LPVOID lpdwThreadParam)
{

	IEngine* engine = new SimpleTracing();

	int j = -1;

	while(!destroyed)
	{
		EnterCriticalSection(&CriticalSection);
		
		if(j >= 0)
		{
			busy[j] = false;
			frame[j]++;

			RECT r;
			r.left   = 0;
			r.top    = j;
			r.right  = W;
			r.bottom = j + 1;
			InvalidateRect(hWnd, &r, false);
		}
		
		j = -1;

		for(int i = 0; i < H; i++)
		{
			if(!busy[i])
			{
				if(j < 0 || frame[i] < frame[j])
				{
					j = i;
				}

			}
		}

		busy[j] = true;

		LeaveCriticalSection(&CriticalSection);

		for(int i = 0; i < W && !destroyed; i++)
		{
			L[i * H + j] += ColorAtPixel(i + PIXEL_SIZE * (float)rand() / RAND_MAX - PIXEL_SIZE * 0.5, j + PIXEL_SIZE * (float)rand() / RAND_MAX - PIXEL_SIZE * 0.5, W, H, CAM_Z, CAM_SIZE, *scene, *lights, *engine);
		}
	}

	return 0;
}