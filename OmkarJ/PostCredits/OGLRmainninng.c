// Header File
#include <windows.h>
#include <stdio.h> // For File I/O
#include <stdlib.h> // for exit()

// OpenGL Releted Header File 
#include <gl/GL.h>

#include "OGL.h"

// Link with OpenGL Import Libraries 
#pragma comment(lib, "opengl32.lib")

// Macros
#define WIN_WIDTH   800
#define WIN_HEIGHT  600

// Global Function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
HWND ghwnd = NULL;
HDC ghdc = NULL;            
HGLRC ghrc = NULL;          
BOOL bFullscreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
FILE *gpFile = NULL;
BOOL bActiveWindow = FALSE; 
BOOL bEscapeKeyIsPressed = FALSE;

// Entry-Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // function declarations  
    int initialize(void);
    void render(void); // display == drow == render
    void update(void);
    void Uninitialize(void);

    

    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_LST");
    BOOL bDone = FALSE; 


    // Create Log file
    gpFile = fopen("Log.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log File Creation Failed.."), TEXT("Error"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "LST : Program started Successfully\n");
    }


    //code
    // WNDCLASSEX Structure Initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // HGDIOBJ
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    
    // Register Above WNDCLASS
    RegisterClassEx(&wndclass);

    // Centering
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create The Window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,  
                            szAppName,
                            TEXT("My First RTR7 Program : Vaibhav Kantrao Deshmukh"),
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, 
                            screenWidth/2 - WIN_WIDTH/2,
                            screenHeight/2 - WIN_HEIGHT/2,
                            WIN_WIDTH,
                            WIN_HEIGHT,
                            NULL,
                            NULL,
                            hInstance,
                            NULL
                        );

    // Set global window handle
    ghwnd = hwnd;

    int iResult = initialize(); 
    if (iResult != 0)
    {
        fprintf(gpFile, "WinMain() : initialized(): function is failed");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "WinMain() : initialize(): Function Succeeded");
    }

    // Show window
    ShowWindow(hwnd, iCmdShow);

    // Update The Window to point its background
    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd); 
    SetFocus(hwnd);

    // Game Loop 
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                bDone = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (bActiveWindow == TRUE)
            {
                if (bEscapeKeyIsPressed == TRUE)
                    bDone = TRUE; 
                    // Render
                    render();
                    // update
                    update();
                    
            }
        }
    }

    // Uninitialized  
    Uninitialize();

    return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // local function declarations
    void resize(int, int); 
    void toggleFullscreen(void);
    void Uninitialize(void); 
    

    // code
    switch(iMsg)
    {
        case WM_CREATE: // wm_create ha window tayar zalya zalya adhi wm_create yeto(manjech pahila yeto) ani parat yet nhi
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS:
            bActiveWindow = TRUE; 
            break;

        case WM_KILLFOCUS:
            bActiveWindow = FALSE; 
            break;

        case WM_SIZE: // 2nd number la wm_size yete ani nantr window yete
            resize(LOWORD(lParam), HIWORD(lParam)); //  LOWORD(lparam) manje Width and HIWORD(lParam) manje Height
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {
            case VK_ESCAPE: //  VK: Virtual Keycode
                bEscapeKeyIsPressed = TRUE;
                break;

            default:
                break;
            }
            break;

        case WM_CHAR:
            switch (wParam)
            {
            case 'F':
            case 'f':
                if (bFullscreen == FALSE)
                {
                    toggleFullscreen();
                    bFullscreen = TRUE;
                }
                else
                {
                    toggleFullscreen();
                    bFullscreen = FALSE;
                } // optimized code toggleFullscreen();
                   // bFullscreen = !bFullscreen;
                break;

            default:
                break;
            }
            break;

        case WM_CLOSE:
            Uninitialize(); 
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            break;
    }

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
    
}

void toggleFullscreen(void)
{
    // Variable declarations
    MONITORINFO mi;
    // code
    if (bFullscreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        memset(&mi, 0, sizeof(MONITORINFO));
        mi.cbSize = sizeof(MONITORINFO);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP,mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top , SWP_NOZORDER | SWP_FRAMECHANGED);
                
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

int initialize(void) 
{
    // function declarations
    void resize(int, int); 
    
    // variable declarations  
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    // code
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)); 

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;   // convensinal
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;

    ghdc = GetDC(ghwnd);
    if (ghdc == NULL)
    {
        return(-1);
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd); // index is 1 based not 0
    if (iPixelFormatIndex == 0)
        return(-2);

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return(-3);
    }

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        return(-4);
    }

    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return(-5);
    }

    // choose screen clearing color as BLUE
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void resize(int width, int height)  
{
    // code
    if (height <= 0)
        height = 1; // height should not be zero for infinity

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void render(void) 
{
    // code
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // ---------- SPECIAL THANKS ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.609f, 0.63f, 0.0f);
    glVertex3f(-0.618f, 0.652f, 0.0f);
    glVertex3f(-0.662f, 0.652f, 0.0f);
    glVertex3f(-0.671f, 0.63f, 0.0f);
    glVertex3f(-0.671f, 0.601f, 0.0f);
    glVertex3f(-0.662f, 0.579f, 0.0f);
    glVertex3f(-0.618f, 0.579f, 0.0f);
    glVertex3f(-0.609f, 0.558f, 0.0f);
    glVertex3f(-0.609f, 0.529f, 0.0f);
    glVertex3f(-0.618f, 0.507f, 0.0f);
    glVertex3f(-0.662f, 0.507f, 0.0f);
    glVertex3f(-0.671f, 0.529f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.573f, 0.507f, 0.0f);
    glVertex3f(-0.573f, 0.652f, 0.0f);
    glVertex3f(-0.523f, 0.652f, 0.0f);
    glVertex3f(-0.51f, 0.63f, 0.0f);
    glVertex3f(-0.51f, 0.601f, 0.0f);
    glVertex3f(-0.523f, 0.579f, 0.0f);
    glVertex3f(-0.573f, 0.579f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.412f, 0.652f, 0.0f);
    glVertex3f(-0.474f, 0.652f, 0.0f);
    glVertex3f(-0.474f, 0.507f, 0.0f);
    glVertex3f(-0.412f, 0.507f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.474f, 0.579f, 0.0f);
    glVertex3f(-0.431f, 0.579f, 0.0f);
    glEnd();

    // C
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.314f, 0.652f, 0.0f);
    glVertex3f(-0.363f, 0.652f, 0.0f);
    glVertex3f(-0.376f, 0.623f, 0.0f);
    glVertex3f(-0.376f, 0.536f, 0.0f);
    glVertex3f(-0.363f, 0.507f, 0.0f);
    glVertex3f(-0.314f, 0.507f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.246f, 0.507f, 0.0f);
    glVertex3f(-0.246f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.265f, 0.652f, 0.0f);
    glVertex3f(-0.228f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.265f, 0.507f, 0.0f);
    glVertex3f(-0.228f, 0.507f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.179f, 0.507f, 0.0f);
    glVertex3f(-0.148f, 0.652f, 0.0f);
    glVertex3f(-0.117f, 0.507f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.163f, 0.579f, 0.0f);
    glVertex3f(-0.132f, 0.579f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.08f, 0.652f, 0.0f);
    glVertex3f(-0.08f, 0.507f, 0.0f);
    glVertex3f(-0.018f, 0.507f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.117f, 0.652f, 0.0f);
    glVertex3f(0.179f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.148f, 0.652f, 0.0f);
    glVertex3f(0.148f, 0.507f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.215f, 0.507f, 0.0f);
    glVertex3f(0.215f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.277f, 0.507f, 0.0f);
    glVertex3f(0.277f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.215f, 0.579f, 0.0f);
    glVertex3f(0.277f, 0.579f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.314f, 0.507f, 0.0f);
    glVertex3f(0.345f, 0.652f, 0.0f);
    glVertex3f(0.376f, 0.507f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.329f, 0.579f, 0.0f);
    glVertex3f(0.36f, 0.579f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.412f, 0.507f, 0.0f);
    glVertex3f(0.412f, 0.652f, 0.0f);
    glVertex3f(0.474f, 0.507f, 0.0f);
    glVertex3f(0.474f, 0.652f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.51f, 0.507f, 0.0f);
    glVertex3f(0.51f, 0.652f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.573f, 0.652f, 0.0f);
    glVertex3f(0.51f, 0.579f, 0.0f);
    glVertex3f(0.573f, 0.507f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.671f, 0.63f, 0.0f);
    glVertex3f(0.662f, 0.652f, 0.0f);
    glVertex3f(0.618f, 0.652f, 0.0f);
    glVertex3f(0.609f, 0.63f, 0.0f);
    glVertex3f(0.609f, 0.601f, 0.0f);
    glVertex3f(0.618f, 0.579f, 0.0f);
    glVertex3f(0.662f, 0.579f, 0.0f);
    glVertex3f(0.671f, 0.558f, 0.0f);
    glVertex3f(0.671f, 0.529f, 0.0f);
    glVertex3f(0.662f, 0.507f, 0.0f);
    glVertex3f(0.618f, 0.507f, 0.0f);
    glVertex3f(0.609f, 0.529f, 0.0f);
    glEnd();


    // ---------- RAMA MADAM ----------
    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.296f, 0.241f, 0.0f);
    glVertex3f(-0.296f, 0.332f, 0.0f);
    glVertex3f(-0.265f, 0.332f, 0.0f);
    glVertex3f(-0.258f, 0.318f, 0.0f);
    glVertex3f(-0.258f, 0.3f, 0.0f);
    glVertex3f(-0.265f, 0.287f, 0.0f);
    glVertex3f(-0.296f, 0.287f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.277f, 0.287f, 0.0f);
    glVertex3f(-0.258f, 0.241f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.235f, 0.241f, 0.0f);
    glVertex3f(-0.215f, 0.332f, 0.0f);
    glVertex3f(-0.196f, 0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.225f, 0.287f, 0.0f);
    glVertex3f(-0.206f, 0.287f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.173f, 0.241f, 0.0f);
    glVertex3f(-0.173f, 0.332f, 0.0f);
    glVertex3f(-0.154f, 0.287f, 0.0f);
    glVertex3f(-0.134f, 0.332f, 0.0f);
    glVertex3f(-0.134f, 0.241f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.112f, 0.241f, 0.0f);
    glVertex3f(-0.092f, 0.332f, 0.0f);
    glVertex3f(-0.073f, 0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.102f, 0.287f, 0.0f);
    glVertex3f(-0.083f, 0.287f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.011f, 0.241f, 0.0f);
    glVertex3f(0.011f, 0.332f, 0.0f);
    glVertex3f(0.031f, 0.287f, 0.0f);
    glVertex3f(0.05f, 0.332f, 0.0f);
    glVertex3f(0.05f, 0.241f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.073f, 0.241f, 0.0f);
    glVertex3f(0.092f, 0.332f, 0.0f);
    glVertex3f(0.112f, 0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.083f, 0.287f, 0.0f);
    glVertex3f(0.102f, 0.287f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.134f, 0.241f, 0.0f);
    glVertex3f(0.134f, 0.332f, 0.0f);
    glVertex3f(0.158f, 0.332f, 0.0f);
    glVertex3f(0.173f, 0.305f, 0.0f);
    glVertex3f(0.173f, 0.269f, 0.0f);
    glVertex3f(0.158f, 0.241f, 0.0f);
    glVertex3f(0.134f, 0.241f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.196f, 0.241f, 0.0f);
    glVertex3f(0.215f, 0.332f, 0.0f);
    glVertex3f(0.235f, 0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.206f, 0.287f, 0.0f);
    glVertex3f(0.225f, 0.287f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.258f, 0.241f, 0.0f);
    glVertex3f(0.258f, 0.332f, 0.0f);
    glVertex3f(0.277f, 0.287f, 0.0f);
    glVertex3f(0.296f, 0.332f, 0.0f);
    glVertex3f(0.296f, 0.241f, 0.0f);
    glEnd();


    // ---------- PRADNYA MADAM ----------
    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.389f, 0.072f, 0.0f);
    glVertex3f(-0.389f, 0.163f, 0.0f);
    glVertex3f(-0.358f, 0.163f, 0.0f);
    glVertex3f(-0.35f, 0.149f, 0.0f);
    glVertex3f(-0.35f, 0.131f, 0.0f);
    glVertex3f(-0.358f, 0.118f, 0.0f);
    glVertex3f(-0.389f, 0.118f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.327f, 0.072f, 0.0f);
    glVertex3f(-0.327f, 0.163f, 0.0f);
    glVertex3f(-0.296f, 0.163f, 0.0f);
    glVertex3f(-0.288f, 0.149f, 0.0f);
    glVertex3f(-0.288f, 0.131f, 0.0f);
    glVertex3f(-0.296f, 0.118f, 0.0f);
    glVertex3f(-0.327f, 0.118f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.308f, 0.118f, 0.0f);
    glVertex3f(-0.288f, 0.072f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.266f, 0.072f, 0.0f);
    glVertex3f(-0.246f, 0.163f, 0.0f);
    glVertex3f(-0.227f, 0.072f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.256f, 0.118f, 0.0f);
    glVertex3f(-0.236f, 0.118f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.204f, 0.072f, 0.0f);
    glVertex3f(-0.204f, 0.163f, 0.0f);
    glVertex3f(-0.181f, 0.163f, 0.0f);
    glVertex3f(-0.165f, 0.136f, 0.0f);
    glVertex3f(-0.165f, 0.1f, 0.0f);
    glVertex3f(-0.181f, 0.072f, 0.0f);
    glVertex3f(-0.204f, 0.072f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.143f, 0.072f, 0.0f);
    glVertex3f(-0.143f, 0.163f, 0.0f);
    glVertex3f(-0.104f, 0.072f, 0.0f);
    glVertex3f(-0.104f, 0.163f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.081f, 0.163f, 0.0f);
    glVertex3f(-0.062f, 0.118f, 0.0f);
    glVertex3f(-0.042f, 0.163f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.062f, 0.118f, 0.0f);
    glVertex3f(-0.062f, 0.072f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.019f, 0.072f, 0.0f);
    glVertex3f(0.0f, 0.163f, 0.0f);
    glVertex3f(0.019f, 0.072f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.01f, 0.118f, 0.0f);
    glVertex3f(0.01f, 0.118f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.104f, 0.072f, 0.0f);
    glVertex3f(0.104f, 0.163f, 0.0f);
    glVertex3f(0.123f, 0.118f, 0.0f);
    glVertex3f(0.143f, 0.163f, 0.0f);
    glVertex3f(0.143f, 0.072f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.165f, 0.072f, 0.0f);
    glVertex3f(0.185f, 0.163f, 0.0f);
    glVertex3f(0.204f, 0.072f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.175f, 0.118f, 0.0f);
    glVertex3f(0.194f, 0.118f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.227f, 0.072f, 0.0f);
    glVertex3f(0.227f, 0.163f, 0.0f);
    glVertex3f(0.25f, 0.163f, 0.0f);
    glVertex3f(0.266f, 0.136f, 0.0f);
    glVertex3f(0.266f, 0.1f, 0.0f);
    glVertex3f(0.25f, 0.072f, 0.0f);
    glVertex3f(0.227f, 0.072f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.288f, 0.072f, 0.0f);
    glVertex3f(0.308f, 0.163f, 0.0f);
    glVertex3f(0.327f, 0.072f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.298f, 0.118f, 0.0f);
    glVertex3f(0.317f, 0.118f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.35f, 0.072f, 0.0f);
    glVertex3f(0.35f, 0.163f, 0.0f);
    glVertex3f(0.369f, 0.118f, 0.0f);
    glVertex3f(0.389f, 0.163f, 0.0f);
    glVertex3f(0.389f, 0.072f, 0.0f);
    glEnd();


    // ---------- RADHIKA MADAM ----------
    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.389f, -0.097f, 0.0f);
    glVertex3f(-0.389f, -0.006f, 0.0f);
    glVertex3f(-0.358f, -0.006f, 0.0f);
    glVertex3f(-0.35f, -0.02f, 0.0f);
    glVertex3f(-0.35f, -0.038f, 0.0f);
    glVertex3f(-0.358f, -0.051f, 0.0f);
    glVertex3f(-0.389f, -0.051f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.369f, -0.051f, 0.0f);
    glVertex3f(-0.35f, -0.097f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.327f, -0.097f, 0.0f);
    glVertex3f(-0.308f, -0.006f, 0.0f);
    glVertex3f(-0.288f, -0.097f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.317f, -0.051f, 0.0f);
    glVertex3f(-0.298f, -0.051f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.266f, -0.097f, 0.0f);
    glVertex3f(-0.266f, -0.006f, 0.0f);
    glVertex3f(-0.242f, -0.006f, 0.0f);
    glVertex3f(-0.227f, -0.033f, 0.0f);
    glVertex3f(-0.227f, -0.069f, 0.0f);
    glVertex3f(-0.242f, -0.097f, 0.0f);
    glVertex3f(-0.266f, -0.097f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.204f, -0.097f, 0.0f);
    glVertex3f(-0.204f, -0.006f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.165f, -0.097f, 0.0f);
    glVertex3f(-0.165f, -0.006f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.204f, -0.051f, 0.0f);
    glVertex3f(-0.165f, -0.051f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.123f, -0.097f, 0.0f);
    glVertex3f(-0.123f, -0.006f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.135f, -0.006f, 0.0f);
    glVertex3f(-0.111f, -0.006f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.135f, -0.097f, 0.0f);
    glVertex3f(-0.111f, -0.097f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.081f, -0.097f, 0.0f);
    glVertex3f(-0.081f, -0.006f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.042f, -0.006f, 0.0f);
    glVertex3f(-0.081f, -0.051f, 0.0f);
    glVertex3f(-0.042f, -0.097f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.019f, -0.097f, 0.0f);
    glVertex3f(0.0f, -0.006f, 0.0f);
    glVertex3f(0.019f, -0.097f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.01f, -0.051f, 0.0f);
    glVertex3f(0.01f, -0.051f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.104f, -0.097f, 0.0f);
    glVertex3f(0.104f, -0.006f, 0.0f);
    glVertex3f(0.123f, -0.051f, 0.0f);
    glVertex3f(0.143f, -0.006f, 0.0f);
    glVertex3f(0.143f, -0.097f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.165f, -0.097f, 0.0f);
    glVertex3f(0.185f, -0.006f, 0.0f);
    glVertex3f(0.204f, -0.097f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.175f, -0.051f, 0.0f);
    glVertex3f(0.194f, -0.051f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.227f, -0.097f, 0.0f);
    glVertex3f(0.227f, -0.006f, 0.0f);
    glVertex3f(0.25f, -0.006f, 0.0f);
    glVertex3f(0.266f, -0.033f, 0.0f);
    glVertex3f(0.266f, -0.069f, 0.0f);
    glVertex3f(0.25f, -0.097f, 0.0f);
    glVertex3f(0.227f, -0.097f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.288f, -0.097f, 0.0f);
    glVertex3f(0.308f, -0.006f, 0.0f);
    glVertex3f(0.327f, -0.097f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.298f, -0.051f, 0.0f);
    glVertex3f(0.317f, -0.051f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.35f, -0.097f, 0.0f);
    glVertex3f(0.35f, -0.006f, 0.0f);
    glVertex3f(0.369f, -0.051f, 0.0f);
    glVertex3f(0.389f, -0.006f, 0.0f);
    glVertex3f(0.389f, -0.097f, 0.0f);
    glEnd();


    // ---------- INSPIRED BY ----------
    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.246f, -0.386f, 0.0f);
    glVertex3f(-0.246f, -0.314f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.255f, -0.314f, 0.0f);
    glVertex3f(-0.237f, -0.314f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.255f, -0.386f, 0.0f);
    glVertex3f(-0.237f, -0.386f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.212f, -0.386f, 0.0f);
    glVertex3f(-0.212f, -0.314f, 0.0f);
    glVertex3f(-0.181f, -0.386f, 0.0f);
    glVertex3f(-0.181f, -0.314f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.132f, -0.325f, 0.0f);
    glVertex3f(-0.137f, -0.314f, 0.0f);
    glVertex3f(-0.159f, -0.314f, 0.0f);
    glVertex3f(-0.163f, -0.325f, 0.0f);
    glVertex3f(-0.163f, -0.339f, 0.0f);
    glVertex3f(-0.159f, -0.35f, 0.0f);
    glVertex3f(-0.137f, -0.35f, 0.0f);
    glVertex3f(-0.132f, -0.361f, 0.0f);
    glVertex3f(-0.132f, -0.375f, 0.0f);
    glVertex3f(-0.137f, -0.386f, 0.0f);
    glVertex3f(-0.159f, -0.386f, 0.0f);
    glVertex3f(-0.163f, -0.375f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.114f, -0.386f, 0.0f);
    glVertex3f(-0.114f, -0.314f, 0.0f);
    glVertex3f(-0.089f, -0.314f, 0.0f);
    glVertex3f(-0.083f, -0.325f, 0.0f);
    glVertex3f(-0.083f, -0.339f, 0.0f);
    glVertex3f(-0.089f, -0.35f, 0.0f);
    glVertex3f(-0.114f, -0.35f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.049f, -0.386f, 0.0f);
    glVertex3f(-0.049f, -0.314f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.059f, -0.314f, 0.0f);
    glVertex3f(-0.04f, -0.314f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.059f, -0.386f, 0.0f);
    glVertex3f(-0.04f, -0.386f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.016f, -0.386f, 0.0f);
    glVertex3f(-0.016f, -0.314f, 0.0f);
    glVertex3f(0.009f, -0.314f, 0.0f);
    glVertex3f(0.016f, -0.325f, 0.0f);
    glVertex3f(0.016f, -0.339f, 0.0f);
    glVertex3f(0.009f, -0.35f, 0.0f);
    glVertex3f(-0.016f, -0.35f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.0f, -0.35f, 0.0f);
    glVertex3f(0.016f, -0.386f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.065f, -0.314f, 0.0f);
    glVertex3f(0.034f, -0.314f, 0.0f);
    glVertex3f(0.034f, -0.386f, 0.0f);
    glVertex3f(0.065f, -0.386f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.034f, -0.35f, 0.0f);
    glVertex3f(0.055f, -0.35f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.083f, -0.386f, 0.0f);
    glVertex3f(0.083f, -0.314f, 0.0f);
    glVertex3f(0.102f, -0.314f, 0.0f);
    glVertex3f(0.114f, -0.336f, 0.0f);
    glVertex3f(0.114f, -0.365f, 0.0f);
    glVertex3f(0.102f, -0.386f, 0.0f);
    glVertex3f(0.083f, -0.386f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.181f, -0.386f, 0.0f);
    glVertex3f(0.181f, -0.314f, 0.0f);
    glVertex3f(0.205f, -0.314f, 0.0f);
    glVertex3f(0.212f, -0.325f, 0.0f);
    glVertex3f(0.212f, -0.339f, 0.0f);
    glVertex3f(0.205f, -0.35f, 0.0f);
    glVertex3f(0.181f, -0.35f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.205f, -0.35f, 0.0f);
    glVertex3f(0.212f, -0.361f, 0.0f);
    glVertex3f(0.212f, -0.375f, 0.0f);
    glVertex3f(0.205f, -0.386f, 0.0f);
    glVertex3f(0.181f, -0.386f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.231f, -0.314f, 0.0f);
    glVertex3f(0.246f, -0.35f, 0.0f);
    glVertex3f(0.262f, -0.314f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.246f, -0.35f, 0.0f);
    glVertex3f(0.246f, -0.386f, 0.0f);
    glEnd();


    // ---------- DR. VIJAY GOKHALE SIR ----------
    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.762f, -0.604f, 0.0f);
    glVertex3f(-0.762f, -0.495f, 0.0f);
    glVertex3f(-0.734f, -0.495f, 0.0f);
    glVertex3f(-0.715f, -0.528f, 0.0f);
    glVertex3f(-0.715f, -0.571f, 0.0f);
    glVertex3f(-0.734f, -0.604f, 0.0f);
    glVertex3f(-0.762f, -0.604f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.688f, -0.604f, 0.0f);
    glVertex3f(-0.688f, -0.495f, 0.0f);
    glVertex3f(-0.651f, -0.495f, 0.0f);
    glVertex3f(-0.641f, -0.511f, 0.0f);
    glVertex3f(-0.641f, -0.533f, 0.0f);
    glVertex3f(-0.651f, -0.549f, 0.0f);
    glVertex3f(-0.688f, -0.549f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.665f, -0.549f, 0.0f);
    glVertex3f(-0.641f, -0.604f, 0.0f);
    glEnd();

    // dot
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.595f, -0.604f, 0.0f);
    glVertex3f(-0.586f, -0.604f, 0.0f);
    glVertex3f(-0.586f, -0.587f, 0.0f);
    glVertex3f(-0.595f, -0.587f, 0.0f);
    glVertex3f(-0.595f, -0.604f, 0.0f);
    glEnd();

    // V
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.466f, -0.495f, 0.0f);
    glVertex3f(-0.443f, -0.604f, 0.0f);
    glVertex3f(-0.42f, -0.495f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.369f, -0.604f, 0.0f);
    glVertex3f(-0.369f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.383f, -0.495f, 0.0f);
    glVertex3f(-0.355f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.383f, -0.604f, 0.0f);
    glVertex3f(-0.355f, -0.604f, 0.0f);
    glEnd();

    // J
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.305f, -0.495f, 0.0f);
    glVertex3f(-0.272f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.284f, -0.495f, 0.0f);
    glVertex3f(-0.284f, -0.582f, 0.0f);
    glVertex3f(-0.293f, -0.604f, 0.0f);
    glVertex3f(-0.309f, -0.604f, 0.0f);
    glVertex3f(-0.319f, -0.582f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.245f, -0.604f, 0.0f);
    glVertex3f(-0.222f, -0.495f, 0.0f);
    glVertex3f(-0.198f, -0.604f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.233f, -0.549f, 0.0f);
    glVertex3f(-0.21f, -0.549f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.171f, -0.495f, 0.0f);
    glVertex3f(-0.148f, -0.549f, 0.0f);
    glVertex3f(-0.124f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.148f, -0.549f, 0.0f);
    glVertex3f(-0.148f, -0.604f, 0.0f);
    glEnd();

    // G
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.023f, -0.517f, 0.0f);
    glVertex3f(0.014f, -0.495f, 0.0f);
    glVertex3f(-0.014f, -0.495f, 0.0f);
    glVertex3f(-0.023f, -0.517f, 0.0f);
    glVertex3f(-0.023f, -0.582f, 0.0f);
    glVertex3f(-0.014f, -0.604f, 0.0f);
    glVertex3f(0.014f, -0.604f, 0.0f);
    glVertex3f(0.023f, -0.582f, 0.0f);
    glVertex3f(0.023f, -0.555f, 0.0f);
    glVertex3f(0.0f, -0.555f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.06f, -0.604f, 0.0f);
    glVertex3f(0.051f, -0.582f, 0.0f);
    glVertex3f(0.051f, -0.517f, 0.0f);
    glVertex3f(0.06f, -0.495f, 0.0f);
    glVertex3f(0.088f, -0.495f, 0.0f);
    glVertex3f(0.097f, -0.517f, 0.0f);
    glVertex3f(0.097f, -0.582f, 0.0f);
    glVertex3f(0.088f, -0.604f, 0.0f);
    glVertex3f(0.06f, -0.604f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.124f, -0.604f, 0.0f);
    glVertex3f(0.124f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.171f, -0.495f, 0.0f);
    glVertex3f(0.124f, -0.549f, 0.0f);
    glVertex3f(0.171f, -0.604f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.198f, -0.604f, 0.0f);
    glVertex3f(0.198f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.245f, -0.604f, 0.0f);
    glVertex3f(0.245f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.198f, -0.549f, 0.0f);
    glVertex3f(0.245f, -0.549f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.272f, -0.604f, 0.0f);
    glVertex3f(0.295f, -0.495f, 0.0f);
    glVertex3f(0.319f, -0.604f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.284f, -0.549f, 0.0f);
    glVertex3f(0.307f, -0.549f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.346f, -0.495f, 0.0f);
    glVertex3f(0.346f, -0.604f, 0.0f);
    glVertex3f(0.393f, -0.604f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.466f, -0.495f, 0.0f);
    glVertex3f(0.42f, -0.495f, 0.0f);
    glVertex3f(0.42f, -0.604f, 0.0f);
    glVertex3f(0.466f, -0.604f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.42f, -0.549f, 0.0f);
    glVertex3f(0.452f, -0.549f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.614f, -0.511f, 0.0f);
    glVertex3f(0.607f, -0.495f, 0.0f);
    glVertex3f(0.574f, -0.495f, 0.0f);
    glVertex3f(0.567f, -0.511f, 0.0f);
    glVertex3f(0.567f, -0.533f, 0.0f);
    glVertex3f(0.574f, -0.549f, 0.0f);
    glVertex3f(0.607f, -0.549f, 0.0f);
    glVertex3f(0.614f, -0.566f, 0.0f);
    glVertex3f(0.614f, -0.587f, 0.0f);
    glVertex3f(0.607f, -0.604f, 0.0f);
    glVertex3f(0.574f, -0.604f, 0.0f);
    glVertex3f(0.567f, -0.587f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.665f, -0.604f, 0.0f);
    glVertex3f(0.665f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.651f, -0.495f, 0.0f);
    glVertex3f(0.679f, -0.495f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.651f, -0.604f, 0.0f);
    glVertex3f(0.679f, -0.604f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.715f, -0.604f, 0.0f);
    glVertex3f(0.715f, -0.495f, 0.0f);
    glVertex3f(0.753f, -0.495f, 0.0f);
    glVertex3f(0.762f, -0.511f, 0.0f);
    glVertex3f(0.762f, -0.533f, 0.0f);
    glVertex3f(0.753f, -0.549f, 0.0f);
    glVertex3f(0.715f, -0.549f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.739f, -0.549f, 0.0f);
    glVertex3f(0.762f, -0.604f, 0.0f);
    glEnd();

    // Do double buffering
    SwapBuffers(ghdc);

}

void update(void) 
{
    // code
}

void Uninitialize(void)
{
    // code
    // if existing in fullscreen first restore and then procced
    if (bFullscreen == TRUE)
    {
        toggleFullscreen();
        bFullscreen = FALSE;
    }

    // First check the current context and if it is then unmake it
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);

    }

    // now destroy the current context
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // relese the device context
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // Destroy Window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Close Log File 
    if (gpFile)
    {
        fprintf(gpFile, "LST : Program Terminated Successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
