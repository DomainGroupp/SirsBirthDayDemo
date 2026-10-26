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
    glVertex3f(-0.51f, 0.8f, 0.0f);
    glVertex3f(-0.517f, 0.818f, 0.0f);
    glVertex3f(-0.554f, 0.818f, 0.0f);
    glVertex3f(-0.562f, 0.8f, 0.0f);
    glVertex3f(-0.562f, 0.776f, 0.0f);
    glVertex3f(-0.554f, 0.758f, 0.0f);
    glVertex3f(-0.517f, 0.758f, 0.0f);
    glVertex3f(-0.51f, 0.739f, 0.0f);
    glVertex3f(-0.51f, 0.715f, 0.0f);
    glVertex3f(-0.517f, 0.697f, 0.0f);
    glVertex3f(-0.554f, 0.697f, 0.0f);
    glVertex3f(-0.562f, 0.715f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.479f, 0.697f, 0.0f);
    glVertex3f(-0.479f, 0.818f, 0.0f);
    glVertex3f(-0.438f, 0.818f, 0.0f);
    glVertex3f(-0.427f, 0.8f, 0.0f);
    glVertex3f(-0.427f, 0.776f, 0.0f);
    glVertex3f(-0.438f, 0.758f, 0.0f);
    glVertex3f(-0.479f, 0.758f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.345f, 0.818f, 0.0f);
    glVertex3f(-0.397f, 0.818f, 0.0f);
    glVertex3f(-0.397f, 0.697f, 0.0f);
    glVertex3f(-0.345f, 0.697f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.397f, 0.758f, 0.0f);
    glVertex3f(-0.36f, 0.758f, 0.0f);
    glEnd();

    // C
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.262f, 0.818f, 0.0f);
    glVertex3f(-0.304f, 0.818f, 0.0f);
    glVertex3f(-0.314f, 0.794f, 0.0f);
    glVertex3f(-0.314f, 0.721f, 0.0f);
    glVertex3f(-0.304f, 0.697f, 0.0f);
    glVertex3f(-0.262f, 0.697f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.206f, 0.697f, 0.0f);
    glVertex3f(-0.206f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.222f, 0.818f, 0.0f);
    glVertex3f(-0.19f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.222f, 0.697f, 0.0f);
    glVertex3f(-0.19f, 0.697f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.15f, 0.697f, 0.0f);
    glVertex3f(-0.124f, 0.818f, 0.0f);
    glVertex3f(-0.098f, 0.697f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.137f, 0.758f, 0.0f);
    glVertex3f(-0.111f, 0.758f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.067f, 0.818f, 0.0f);
    glVertex3f(-0.067f, 0.697f, 0.0f);
    glVertex3f(-0.015f, 0.697f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.098f, 0.818f, 0.0f);
    glVertex3f(0.15f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.124f, 0.818f, 0.0f);
    glVertex3f(0.124f, 0.697f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.18f, 0.697f, 0.0f);
    glVertex3f(0.18f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.232f, 0.697f, 0.0f);
    glVertex3f(0.232f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.18f, 0.758f, 0.0f);
    glVertex3f(0.232f, 0.758f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.262f, 0.697f, 0.0f);
    glVertex3f(0.288f, 0.818f, 0.0f);
    glVertex3f(0.314f, 0.697f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.275f, 0.758f, 0.0f);
    glVertex3f(0.301f, 0.758f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.345f, 0.697f, 0.0f);
    glVertex3f(0.345f, 0.818f, 0.0f);
    glVertex3f(0.397f, 0.697f, 0.0f);
    glVertex3f(0.397f, 0.818f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.427f, 0.697f, 0.0f);
    glVertex3f(0.427f, 0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.479f, 0.818f, 0.0f);
    glVertex3f(0.427f, 0.758f, 0.0f);
    glVertex3f(0.479f, 0.697f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.562f, 0.8f, 0.0f);
    glVertex3f(0.554f, 0.818f, 0.0f);
    glVertex3f(0.517f, 0.818f, 0.0f);
    glVertex3f(0.51f, 0.8f, 0.0f);
    glVertex3f(0.51f, 0.776f, 0.0f);
    glVertex3f(0.517f, 0.758f, 0.0f);
    glVertex3f(0.554f, 0.758f, 0.0f);
    glVertex3f(0.562f, 0.739f, 0.0f);
    glVertex3f(0.562f, 0.715f, 0.0f);
    glVertex3f(0.554f, 0.697f, 0.0f);
    glVertex3f(0.517f, 0.697f, 0.0f);
    glVertex3f(0.51f, 0.715f, 0.0f);
    glEnd();


    // ---------- SHRAYU BAGUL ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.276f, 0.603f, 0.0f);
    glVertex3f(-0.281f, 0.615f, 0.0f);
    glVertex3f(-0.305f, 0.615f, 0.0f);
    glVertex3f(-0.31f, 0.603f, 0.0f);
    glVertex3f(-0.31f, 0.588f, 0.0f);
    glVertex3f(-0.305f, 0.576f, 0.0f);
    glVertex3f(-0.281f, 0.576f, 0.0f);
    glVertex3f(-0.276f, 0.564f, 0.0f);
    glVertex3f(-0.276f, 0.548f, 0.0f);
    glVertex3f(-0.281f, 0.537f, 0.0f);
    glVertex3f(-0.305f, 0.537f, 0.0f);
    glVertex3f(-0.31f, 0.548f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.257f, 0.537f, 0.0f);
    glVertex3f(-0.257f, 0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.223f, 0.537f, 0.0f);
    glVertex3f(-0.223f, 0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.257f, 0.576f, 0.0f);
    glVertex3f(-0.223f, 0.576f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.203f, 0.537f, 0.0f);
    glVertex3f(-0.203f, 0.615f, 0.0f);
    glVertex3f(-0.177f, 0.615f, 0.0f);
    glVertex3f(-0.17f, 0.603f, 0.0f);
    glVertex3f(-0.17f, 0.588f, 0.0f);
    glVertex3f(-0.177f, 0.576f, 0.0f);
    glVertex3f(-0.203f, 0.576f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.187f, 0.576f, 0.0f);
    glVertex3f(-0.17f, 0.537f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.15f, 0.537f, 0.0f);
    glVertex3f(-0.133f, 0.615f, 0.0f);
    glVertex3f(-0.116f, 0.537f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.142f, 0.576f, 0.0f);
    glVertex3f(-0.125f, 0.576f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.097f, 0.615f, 0.0f);
    glVertex3f(-0.08f, 0.576f, 0.0f);
    glVertex3f(-0.063f, 0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.08f, 0.576f, 0.0f);
    glVertex3f(-0.08f, 0.537f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.043f, 0.615f, 0.0f);
    glVertex3f(-0.043f, 0.552f, 0.0f);
    glVertex3f(-0.037f, 0.537f, 0.0f);
    glVertex3f(-0.017f, 0.537f, 0.0f);
    glVertex3f(-0.01f, 0.552f, 0.0f);
    glVertex3f(-0.01f, 0.615f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.063f, 0.537f, 0.0f);
    glVertex3f(0.063f, 0.615f, 0.0f);
    glVertex3f(0.088f, 0.615f, 0.0f);
    glVertex3f(0.097f, 0.603f, 0.0f);
    glVertex3f(0.097f, 0.588f, 0.0f);
    glVertex3f(0.088f, 0.576f, 0.0f);
    glVertex3f(0.063f, 0.576f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.088f, 0.576f, 0.0f);
    glVertex3f(0.097f, 0.564f, 0.0f);
    glVertex3f(0.097f, 0.548f, 0.0f);
    glVertex3f(0.088f, 0.537f, 0.0f);
    glVertex3f(0.063f, 0.537f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.116f, 0.537f, 0.0f);
    glVertex3f(0.133f, 0.615f, 0.0f);
    glVertex3f(0.15f, 0.537f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.125f, 0.576f, 0.0f);
    glVertex3f(0.142f, 0.576f, 0.0f);
    glEnd();

    // G
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.203f, 0.599f, 0.0f);
    glVertex3f(0.197f, 0.615f, 0.0f);
    glVertex3f(0.177f, 0.615f, 0.0f);
    glVertex3f(0.17f, 0.599f, 0.0f);
    glVertex3f(0.17f, 0.552f, 0.0f);
    glVertex3f(0.177f, 0.537f, 0.0f);
    glVertex3f(0.197f, 0.537f, 0.0f);
    glVertex3f(0.203f, 0.552f, 0.0f);
    glVertex3f(0.203f, 0.572f, 0.0f);
    glVertex3f(0.187f, 0.572f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.223f, 0.615f, 0.0f);
    glVertex3f(0.223f, 0.552f, 0.0f);
    glVertex3f(0.23f, 0.537f, 0.0f);
    glVertex3f(0.25f, 0.537f, 0.0f);
    glVertex3f(0.257f, 0.552f, 0.0f);
    glVertex3f(0.257f, 0.615f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.276f, 0.615f, 0.0f);
    glVertex3f(0.276f, 0.537f, 0.0f);
    glVertex3f(0.31f, 0.537f, 0.0f);
    glEnd();


    // ---------- GROUP LEADER ----------
    // G
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.226f, 0.41f, 0.0f);
    glVertex3f(-0.232f, 0.422f, 0.0f);
    glVertex3f(-0.248f, 0.422f, 0.0f);
    glVertex3f(-0.254f, 0.41f, 0.0f);
    glVertex3f(-0.254f, 0.371f, 0.0f);
    glVertex3f(-0.248f, 0.358f, 0.0f);
    glVertex3f(-0.232f, 0.358f, 0.0f);
    glVertex3f(-0.226f, 0.371f, 0.0f);
    glVertex3f(-0.226f, 0.387f, 0.0f);
    glVertex3f(-0.24f, 0.387f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.21f, 0.358f, 0.0f);
    glVertex3f(-0.21f, 0.422f, 0.0f);
    glVertex3f(-0.188f, 0.422f, 0.0f);
    glVertex3f(-0.183f, 0.413f, 0.0f);
    glVertex3f(-0.183f, 0.4f, 0.0f);
    glVertex3f(-0.188f, 0.39f, 0.0f);
    glVertex3f(-0.21f, 0.39f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.196f, 0.39f, 0.0f);
    glVertex3f(-0.183f, 0.358f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.161f, 0.358f, 0.0f);
    glVertex3f(-0.166f, 0.371f, 0.0f);
    glVertex3f(-0.166f, 0.41f, 0.0f);
    glVertex3f(-0.161f, 0.422f, 0.0f);
    glVertex3f(-0.144f, 0.422f, 0.0f);
    glVertex3f(-0.139f, 0.41f, 0.0f);
    glVertex3f(-0.139f, 0.371f, 0.0f);
    glVertex3f(-0.144f, 0.358f, 0.0f);
    glVertex3f(-0.161f, 0.358f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.123f, 0.422f, 0.0f);
    glVertex3f(-0.123f, 0.371f, 0.0f);
    glVertex3f(-0.117f, 0.358f, 0.0f);
    glVertex3f(-0.101f, 0.358f, 0.0f);
    glVertex3f(-0.095f, 0.371f, 0.0f);
    glVertex3f(-0.095f, 0.422f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.079f, 0.358f, 0.0f);
    glVertex3f(-0.079f, 0.422f, 0.0f);
    glVertex3f(-0.057f, 0.422f, 0.0f);
    glVertex3f(-0.052f, 0.413f, 0.0f);
    glVertex3f(-0.052f, 0.4f, 0.0f);
    glVertex3f(-0.057f, 0.39f, 0.0f);
    glVertex3f(-0.079f, 0.39f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.008f, 0.422f, 0.0f);
    glVertex3f(0.008f, 0.358f, 0.0f);
    glVertex3f(0.036f, 0.358f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.079f, 0.422f, 0.0f);
    glVertex3f(0.052f, 0.422f, 0.0f);
    glVertex3f(0.052f, 0.358f, 0.0f);
    glVertex3f(0.079f, 0.358f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.052f, 0.39f, 0.0f);
    glVertex3f(0.071f, 0.39f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.095f, 0.358f, 0.0f);
    glVertex3f(0.109f, 0.422f, 0.0f);
    glVertex3f(0.123f, 0.358f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.102f, 0.39f, 0.0f);
    glVertex3f(0.116f, 0.39f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.139f, 0.358f, 0.0f);
    glVertex3f(0.139f, 0.422f, 0.0f);
    glVertex3f(0.155f, 0.422f, 0.0f);
    glVertex3f(0.166f, 0.403f, 0.0f);
    glVertex3f(0.166f, 0.378f, 0.0f);
    glVertex3f(0.155f, 0.358f, 0.0f);
    glVertex3f(0.139f, 0.358f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.21f, 0.422f, 0.0f);
    glVertex3f(0.183f, 0.422f, 0.0f);
    glVertex3f(0.183f, 0.358f, 0.0f);
    glVertex3f(0.21f, 0.358f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.183f, 0.39f, 0.0f);
    glVertex3f(0.202f, 0.39f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.226f, 0.358f, 0.0f);
    glVertex3f(0.226f, 0.422f, 0.0f);
    glVertex3f(0.248f, 0.422f, 0.0f);
    glVertex3f(0.254f, 0.413f, 0.0f);
    glVertex3f(0.254f, 0.4f, 0.0f);
    glVertex3f(0.248f, 0.39f, 0.0f);
    glVertex3f(0.226f, 0.39f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.24f, 0.39f, 0.0f);
    glVertex3f(0.254f, 0.358f, 0.0f);
    glEnd();


    // ---------- OMKAR SURESH BHALERAO ----------
    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.543f, 0.234f, 0.0f);
    glVertex3f(-0.55f, 0.249f, 0.0f);
    glVertex3f(-0.55f, 0.296f, 0.0f);
    glVertex3f(-0.543f, 0.312f, 0.0f);
    glVertex3f(-0.523f, 0.312f, 0.0f);
    glVertex3f(-0.516f, 0.296f, 0.0f);
    glVertex3f(-0.516f, 0.249f, 0.0f);
    glVertex3f(-0.523f, 0.234f, 0.0f);
    glVertex3f(-0.543f, 0.234f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.497f, 0.234f, 0.0f);
    glVertex3f(-0.497f, 0.312f, 0.0f);
    glVertex3f(-0.48f, 0.273f, 0.0f);
    glVertex3f(-0.463f, 0.312f, 0.0f);
    glVertex3f(-0.463f, 0.234f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.443f, 0.234f, 0.0f);
    glVertex3f(-0.443f, 0.312f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.41f, 0.312f, 0.0f);
    glVertex3f(-0.443f, 0.273f, 0.0f);
    glVertex3f(-0.41f, 0.234f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.39f, 0.234f, 0.0f);
    glVertex3f(-0.373f, 0.312f, 0.0f);
    glVertex3f(-0.356f, 0.234f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.382f, 0.273f, 0.0f);
    glVertex3f(-0.365f, 0.273f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.337f, 0.234f, 0.0f);
    glVertex3f(-0.337f, 0.312f, 0.0f);
    glVertex3f(-0.31f, 0.312f, 0.0f);
    glVertex3f(-0.303f, 0.3f, 0.0f);
    glVertex3f(-0.303f, 0.284f, 0.0f);
    glVertex3f(-0.31f, 0.273f, 0.0f);
    glVertex3f(-0.337f, 0.273f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.32f, 0.273f, 0.0f);
    glVertex3f(-0.303f, 0.234f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.196f, 0.3f, 0.0f);
    glVertex3f(-0.201f, 0.312f, 0.0f);
    glVertex3f(-0.225f, 0.312f, 0.0f);
    glVertex3f(-0.23f, 0.3f, 0.0f);
    glVertex3f(-0.23f, 0.284f, 0.0f);
    glVertex3f(-0.225f, 0.273f, 0.0f);
    glVertex3f(-0.201f, 0.273f, 0.0f);
    glVertex3f(-0.196f, 0.261f, 0.0f);
    glVertex3f(-0.196f, 0.245f, 0.0f);
    glVertex3f(-0.201f, 0.234f, 0.0f);
    glVertex3f(-0.225f, 0.234f, 0.0f);
    glVertex3f(-0.23f, 0.245f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.177f, 0.312f, 0.0f);
    glVertex3f(-0.177f, 0.249f, 0.0f);
    glVertex3f(-0.17f, 0.234f, 0.0f);
    glVertex3f(-0.15f, 0.234f, 0.0f);
    glVertex3f(-0.143f, 0.249f, 0.0f);
    glVertex3f(-0.143f, 0.312f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.123f, 0.234f, 0.0f);
    glVertex3f(-0.123f, 0.312f, 0.0f);
    glVertex3f(-0.097f, 0.312f, 0.0f);
    glVertex3f(-0.09f, 0.3f, 0.0f);
    glVertex3f(-0.09f, 0.284f, 0.0f);
    glVertex3f(-0.097f, 0.273f, 0.0f);
    glVertex3f(-0.123f, 0.273f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.107f, 0.273f, 0.0f);
    glVertex3f(-0.09f, 0.234f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.036f, 0.312f, 0.0f);
    glVertex3f(-0.07f, 0.312f, 0.0f);
    glVertex3f(-0.07f, 0.234f, 0.0f);
    glVertex3f(-0.036f, 0.234f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.07f, 0.273f, 0.0f);
    glVertex3f(-0.047f, 0.273f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.017f, 0.3f, 0.0f);
    glVertex3f(0.012f, 0.312f, 0.0f);
    glVertex3f(-0.012f, 0.312f, 0.0f);
    glVertex3f(-0.017f, 0.3f, 0.0f);
    glVertex3f(-0.017f, 0.284f, 0.0f);
    glVertex3f(-0.012f, 0.273f, 0.0f);
    glVertex3f(0.012f, 0.273f, 0.0f);
    glVertex3f(0.017f, 0.261f, 0.0f);
    glVertex3f(0.017f, 0.245f, 0.0f);
    glVertex3f(0.012f, 0.234f, 0.0f);
    glVertex3f(-0.012f, 0.234f, 0.0f);
    glVertex3f(-0.017f, 0.245f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.036f, 0.234f, 0.0f);
    glVertex3f(0.036f, 0.312f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.07f, 0.234f, 0.0f);
    glVertex3f(0.07f, 0.312f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.036f, 0.273f, 0.0f);
    glVertex3f(0.07f, 0.273f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.143f, 0.234f, 0.0f);
    glVertex3f(0.143f, 0.312f, 0.0f);
    glVertex3f(0.168f, 0.312f, 0.0f);
    glVertex3f(0.177f, 0.3f, 0.0f);
    glVertex3f(0.177f, 0.284f, 0.0f);
    glVertex3f(0.168f, 0.273f, 0.0f);
    glVertex3f(0.143f, 0.273f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.168f, 0.273f, 0.0f);
    glVertex3f(0.177f, 0.261f, 0.0f);
    glVertex3f(0.177f, 0.245f, 0.0f);
    glVertex3f(0.168f, 0.234f, 0.0f);
    glVertex3f(0.143f, 0.234f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.196f, 0.234f, 0.0f);
    glVertex3f(0.196f, 0.312f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.23f, 0.234f, 0.0f);
    glVertex3f(0.23f, 0.312f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.196f, 0.273f, 0.0f);
    glVertex3f(0.23f, 0.273f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.25f, 0.234f, 0.0f);
    glVertex3f(0.267f, 0.312f, 0.0f);
    glVertex3f(0.283f, 0.234f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.258f, 0.273f, 0.0f);
    glVertex3f(0.275f, 0.273f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.303f, 0.312f, 0.0f);
    glVertex3f(0.303f, 0.234f, 0.0f);
    glVertex3f(0.337f, 0.234f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.39f, 0.312f, 0.0f);
    glVertex3f(0.356f, 0.312f, 0.0f);
    glVertex3f(0.356f, 0.234f, 0.0f);
    glVertex3f(0.39f, 0.234f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.356f, 0.273f, 0.0f);
    glVertex3f(0.38f, 0.273f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.41f, 0.234f, 0.0f);
    glVertex3f(0.41f, 0.312f, 0.0f);
    glVertex3f(0.437f, 0.312f, 0.0f);
    glVertex3f(0.443f, 0.3f, 0.0f);
    glVertex3f(0.443f, 0.284f, 0.0f);
    glVertex3f(0.437f, 0.273f, 0.0f);
    glVertex3f(0.41f, 0.273f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.427f, 0.273f, 0.0f);
    glVertex3f(0.443f, 0.234f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.463f, 0.234f, 0.0f);
    glVertex3f(0.48f, 0.312f, 0.0f);
    glVertex3f(0.497f, 0.234f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.471f, 0.273f, 0.0f);
    glVertex3f(0.488f, 0.273f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.523f, 0.234f, 0.0f);
    glVertex3f(0.516f, 0.249f, 0.0f);
    glVertex3f(0.516f, 0.296f, 0.0f);
    glVertex3f(0.523f, 0.312f, 0.0f);
    glVertex3f(0.543f, 0.312f, 0.0f);
    glVertex3f(0.55f, 0.296f, 0.0f);
    glVertex3f(0.55f, 0.249f, 0.0f);
    glVertex3f(0.543f, 0.234f, 0.0f);
    glVertex3f(0.523f, 0.234f, 0.0f);
    glEnd();


    // ---------- GROUP MEMBERS ----------
    // G
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.248f, 0.107f, 0.0f);
    glVertex3f(-0.253f, 0.119f, 0.0f);
    glVertex3f(-0.27f, 0.119f, 0.0f);
    glVertex3f(-0.276f, 0.107f, 0.0f);
    glVertex3f(-0.276f, 0.068f, 0.0f);
    glVertex3f(-0.27f, 0.055f, 0.0f);
    glVertex3f(-0.253f, 0.055f, 0.0f);
    glVertex3f(-0.248f, 0.068f, 0.0f);
    glVertex3f(-0.248f, 0.084f, 0.0f);
    glVertex3f(-0.262f, 0.084f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.232f, 0.055f, 0.0f);
    glVertex3f(-0.232f, 0.119f, 0.0f);
    glVertex3f(-0.21f, 0.119f, 0.0f);
    glVertex3f(-0.204f, 0.11f, 0.0f);
    glVertex3f(-0.204f, 0.097f, 0.0f);
    glVertex3f(-0.21f, 0.087f, 0.0f);
    glVertex3f(-0.232f, 0.087f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.218f, 0.087f, 0.0f);
    glVertex3f(-0.204f, 0.055f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.183f, 0.055f, 0.0f);
    glVertex3f(-0.188f, 0.068f, 0.0f);
    glVertex3f(-0.188f, 0.107f, 0.0f);
    glVertex3f(-0.183f, 0.119f, 0.0f);
    glVertex3f(-0.166f, 0.119f, 0.0f);
    glVertex3f(-0.161f, 0.107f, 0.0f);
    glVertex3f(-0.161f, 0.068f, 0.0f);
    glVertex3f(-0.166f, 0.055f, 0.0f);
    glVertex3f(-0.183f, 0.055f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.145f, 0.119f, 0.0f);
    glVertex3f(-0.145f, 0.068f, 0.0f);
    glVertex3f(-0.139f, 0.055f, 0.0f);
    glVertex3f(-0.123f, 0.055f, 0.0f);
    glVertex3f(-0.117f, 0.068f, 0.0f);
    glVertex3f(-0.117f, 0.119f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.101f, 0.055f, 0.0f);
    glVertex3f(-0.101f, 0.119f, 0.0f);
    glVertex3f(-0.079f, 0.119f, 0.0f);
    glVertex3f(-0.073f, 0.11f, 0.0f);
    glVertex3f(-0.073f, 0.097f, 0.0f);
    glVertex3f(-0.079f, 0.087f, 0.0f);
    glVertex3f(-0.101f, 0.087f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(-0.014f, 0.055f, 0.0f);
    glVertex3f(-0.014f, 0.119f, 0.0f);
    glVertex3f(0.0f, 0.087f, 0.0f);
    glVertex3f(0.014f, 0.119f, 0.0f);
    glVertex3f(0.014f, 0.055f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.057f, 0.119f, 0.0f);
    glVertex3f(0.03f, 0.119f, 0.0f);
    glVertex3f(0.03f, 0.055f, 0.0f);
    glVertex3f(0.057f, 0.055f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.03f, 0.087f, 0.0f);
    glVertex3f(0.049f, 0.087f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.073f, 0.055f, 0.0f);
    glVertex3f(0.073f, 0.119f, 0.0f);
    glVertex3f(0.087f, 0.087f, 0.0f);
    glVertex3f(0.101f, 0.119f, 0.0f);
    glVertex3f(0.101f, 0.055f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.117f, 0.055f, 0.0f);
    glVertex3f(0.117f, 0.119f, 0.0f);
    glVertex3f(0.138f, 0.119f, 0.0f);
    glVertex3f(0.145f, 0.11f, 0.0f);
    glVertex3f(0.145f, 0.097f, 0.0f);
    glVertex3f(0.138f, 0.087f, 0.0f);
    glVertex3f(0.117f, 0.087f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.138f, 0.087f, 0.0f);
    glVertex3f(0.145f, 0.078f, 0.0f);
    glVertex3f(0.145f, 0.065f, 0.0f);
    glVertex3f(0.138f, 0.055f, 0.0f);
    glVertex3f(0.117f, 0.055f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.188f, 0.119f, 0.0f);
    glVertex3f(0.161f, 0.119f, 0.0f);
    glVertex3f(0.161f, 0.055f, 0.0f);
    glVertex3f(0.188f, 0.055f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.161f, 0.087f, 0.0f);
    glVertex3f(0.18f, 0.087f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.204f, 0.055f, 0.0f);
    glVertex3f(0.204f, 0.119f, 0.0f);
    glVertex3f(0.226f, 0.119f, 0.0f);
    glVertex3f(0.232f, 0.11f, 0.0f);
    glVertex3f(0.232f, 0.097f, 0.0f);
    glVertex3f(0.226f, 0.087f, 0.0f);
    glVertex3f(0.204f, 0.087f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.218f, 0.087f, 0.0f);
    glVertex3f(0.232f, 0.055f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 0.84f, 0.0f);
    glVertex3f(0.276f, 0.11f, 0.0f);
    glVertex3f(0.271f, 0.119f, 0.0f);
    glVertex3f(0.252f, 0.119f, 0.0f);
    glVertex3f(0.248f, 0.11f, 0.0f);
    glVertex3f(0.248f, 0.097f, 0.0f);
    glVertex3f(0.252f, 0.087f, 0.0f);
    glVertex3f(0.271f, 0.087f, 0.0f);
    glVertex3f(0.276f, 0.078f, 0.0f);
    glVertex3f(0.276f, 0.065f, 0.0f);
    glVertex3f(0.271f, 0.055f, 0.0f);
    glVertex3f(0.252f, 0.055f, 0.0f);
    glVertex3f(0.248f, 0.065f, 0.0f);
    glEnd();


    // ---------- OMKAR SATISH JOSHI ----------
    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.463f, -0.07f, 0.0f);
    glVertex3f(-0.47f, -0.054f, 0.0f);
    glVertex3f(-0.47f, -0.007f, 0.0f);
    glVertex3f(-0.463f, 0.009f, 0.0f);
    glVertex3f(-0.443f, 0.009f, 0.0f);
    glVertex3f(-0.436f, -0.007f, 0.0f);
    glVertex3f(-0.436f, -0.054f, 0.0f);
    glVertex3f(-0.443f, -0.07f, 0.0f);
    glVertex3f(-0.463f, -0.07f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.417f, -0.07f, 0.0f);
    glVertex3f(-0.417f, 0.009f, 0.0f);
    glVertex3f(-0.4f, -0.03f, 0.0f);
    glVertex3f(-0.383f, 0.009f, 0.0f);
    glVertex3f(-0.383f, -0.07f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.363f, -0.07f, 0.0f);
    glVertex3f(-0.363f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.33f, 0.009f, 0.0f);
    glVertex3f(-0.363f, -0.03f, 0.0f);
    glVertex3f(-0.33f, -0.07f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.31f, -0.07f, 0.0f);
    glVertex3f(-0.293f, 0.009f, 0.0f);
    glVertex3f(-0.276f, -0.07f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.302f, -0.03f, 0.0f);
    glVertex3f(-0.285f, -0.03f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.257f, -0.07f, 0.0f);
    glVertex3f(-0.257f, 0.009f, 0.0f);
    glVertex3f(-0.23f, 0.009f, 0.0f);
    glVertex3f(-0.223f, -0.003f, 0.0f);
    glVertex3f(-0.223f, -0.019f, 0.0f);
    glVertex3f(-0.23f, -0.03f, 0.0f);
    glVertex3f(-0.257f, -0.03f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.24f, -0.03f, 0.0f);
    glVertex3f(-0.223f, -0.07f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.116f, -0.003f, 0.0f);
    glVertex3f(-0.122f, 0.009f, 0.0f);
    glVertex3f(-0.145f, 0.009f, 0.0f);
    glVertex3f(-0.15f, -0.003f, 0.0f);
    glVertex3f(-0.15f, -0.019f, 0.0f);
    glVertex3f(-0.145f, -0.03f, 0.0f);
    glVertex3f(-0.122f, -0.03f, 0.0f);
    glVertex3f(-0.116f, -0.042f, 0.0f);
    glVertex3f(-0.116f, -0.058f, 0.0f);
    glVertex3f(-0.122f, -0.07f, 0.0f);
    glVertex3f(-0.145f, -0.07f, 0.0f);
    glVertex3f(-0.15f, -0.058f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.097f, -0.07f, 0.0f);
    glVertex3f(-0.08f, 0.009f, 0.0f);
    glVertex3f(-0.063f, -0.07f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.088f, -0.03f, 0.0f);
    glVertex3f(-0.072f, -0.03f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.043f, 0.009f, 0.0f);
    glVertex3f(-0.01f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.027f, 0.009f, 0.0f);
    glVertex3f(-0.027f, -0.07f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.027f, -0.07f, 0.0f);
    glVertex3f(0.027f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.017f, 0.009f, 0.0f);
    glVertex3f(0.037f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.017f, -0.07f, 0.0f);
    glVertex3f(0.037f, -0.07f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.097f, -0.003f, 0.0f);
    glVertex3f(0.092f, 0.009f, 0.0f);
    glVertex3f(0.068f, 0.009f, 0.0f);
    glVertex3f(0.063f, -0.003f, 0.0f);
    glVertex3f(0.063f, -0.019f, 0.0f);
    glVertex3f(0.068f, -0.03f, 0.0f);
    glVertex3f(0.092f, -0.03f, 0.0f);
    glVertex3f(0.097f, -0.042f, 0.0f);
    glVertex3f(0.097f, -0.058f, 0.0f);
    glVertex3f(0.092f, -0.07f, 0.0f);
    glVertex3f(0.068f, -0.07f, 0.0f);
    glVertex3f(0.063f, -0.058f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.116f, -0.07f, 0.0f);
    glVertex3f(0.116f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.15f, -0.07f, 0.0f);
    glVertex3f(0.15f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.116f, -0.03f, 0.0f);
    glVertex3f(0.15f, -0.03f, 0.0f);
    glEnd();

    // J
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.233f, 0.009f, 0.0f);
    glVertex3f(0.257f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.248f, 0.009f, 0.0f);
    glVertex3f(0.248f, -0.054f, 0.0f);
    glVertex3f(0.242f, -0.07f, 0.0f);
    glVertex3f(0.23f, -0.07f, 0.0f);
    glVertex3f(0.223f, -0.054f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.283f, -0.07f, 0.0f);
    glVertex3f(0.276f, -0.054f, 0.0f);
    glVertex3f(0.276f, -0.007f, 0.0f);
    glVertex3f(0.283f, 0.009f, 0.0f);
    glVertex3f(0.303f, 0.009f, 0.0f);
    glVertex3f(0.31f, -0.007f, 0.0f);
    glVertex3f(0.31f, -0.054f, 0.0f);
    glVertex3f(0.303f, -0.07f, 0.0f);
    glVertex3f(0.283f, -0.07f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.363f, -0.003f, 0.0f);
    glVertex3f(0.358f, 0.009f, 0.0f);
    glVertex3f(0.335f, 0.009f, 0.0f);
    glVertex3f(0.33f, -0.003f, 0.0f);
    glVertex3f(0.33f, -0.019f, 0.0f);
    glVertex3f(0.335f, -0.03f, 0.0f);
    glVertex3f(0.358f, -0.03f, 0.0f);
    glVertex3f(0.363f, -0.042f, 0.0f);
    glVertex3f(0.363f, -0.058f, 0.0f);
    glVertex3f(0.358f, -0.07f, 0.0f);
    glVertex3f(0.335f, -0.07f, 0.0f);
    glVertex3f(0.33f, -0.058f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.07f, 0.0f);
    glVertex3f(0.383f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.417f, -0.07f, 0.0f);
    glVertex3f(0.417f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.03f, 0.0f);
    glVertex3f(0.417f, -0.03f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.453f, -0.07f, 0.0f);
    glVertex3f(0.453f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.443f, 0.009f, 0.0f);
    glVertex3f(0.463f, 0.009f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.443f, -0.07f, 0.0f);
    glVertex3f(0.463f, -0.07f, 0.0f);
    glEnd();


    // ---------- SUKANYA VASANT SONAWANE ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.57f, -0.128f, 0.0f);
    glVertex3f(-0.575f, -0.116f, 0.0f);
    glVertex3f(-0.598f, -0.116f, 0.0f);
    glVertex3f(-0.603f, -0.128f, 0.0f);
    glVertex3f(-0.603f, -0.143f, 0.0f);
    glVertex3f(-0.598f, -0.155f, 0.0f);
    glVertex3f(-0.575f, -0.155f, 0.0f);
    glVertex3f(-0.57f, -0.167f, 0.0f);
    glVertex3f(-0.57f, -0.183f, 0.0f);
    glVertex3f(-0.575f, -0.194f, 0.0f);
    glVertex3f(-0.598f, -0.194f, 0.0f);
    glVertex3f(-0.603f, -0.183f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.55f, -0.116f, 0.0f);
    glVertex3f(-0.55f, -0.179f, 0.0f);
    glVertex3f(-0.543f, -0.194f, 0.0f);
    glVertex3f(-0.523f, -0.194f, 0.0f);
    glVertex3f(-0.516f, -0.179f, 0.0f);
    glVertex3f(-0.516f, -0.116f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.497f, -0.194f, 0.0f);
    glVertex3f(-0.497f, -0.116f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.463f, -0.116f, 0.0f);
    glVertex3f(-0.497f, -0.155f, 0.0f);
    glVertex3f(-0.463f, -0.194f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.443f, -0.194f, 0.0f);
    glVertex3f(-0.427f, -0.116f, 0.0f);
    glVertex3f(-0.41f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.435f, -0.155f, 0.0f);
    glVertex3f(-0.418f, -0.155f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.39f, -0.194f, 0.0f);
    glVertex3f(-0.39f, -0.116f, 0.0f);
    glVertex3f(-0.356f, -0.194f, 0.0f);
    glVertex3f(-0.356f, -0.116f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.337f, -0.116f, 0.0f);
    glVertex3f(-0.32f, -0.155f, 0.0f);
    glVertex3f(-0.303f, -0.116f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.32f, -0.155f, 0.0f);
    glVertex3f(-0.32f, -0.194f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.283f, -0.194f, 0.0f);
    glVertex3f(-0.267f, -0.116f, 0.0f);
    glVertex3f(-0.25f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.275f, -0.155f, 0.0f);
    glVertex3f(-0.258f, -0.155f, 0.0f);
    glEnd();

    // V
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.177f, -0.116f, 0.0f);
    glVertex3f(-0.16f, -0.194f, 0.0f);
    glVertex3f(-0.143f, -0.116f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.123f, -0.194f, 0.0f);
    glVertex3f(-0.107f, -0.116f, 0.0f);
    glVertex3f(-0.09f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.115f, -0.155f, 0.0f);
    glVertex3f(-0.098f, -0.155f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.036f, -0.128f, 0.0f);
    glVertex3f(-0.042f, -0.116f, 0.0f);
    glVertex3f(-0.065f, -0.116f, 0.0f);
    glVertex3f(-0.07f, -0.128f, 0.0f);
    glVertex3f(-0.07f, -0.143f, 0.0f);
    glVertex3f(-0.065f, -0.155f, 0.0f);
    glVertex3f(-0.042f, -0.155f, 0.0f);
    glVertex3f(-0.036f, -0.167f, 0.0f);
    glVertex3f(-0.036f, -0.183f, 0.0f);
    glVertex3f(-0.042f, -0.194f, 0.0f);
    glVertex3f(-0.065f, -0.194f, 0.0f);
    glVertex3f(-0.07f, -0.183f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.017f, -0.194f, 0.0f);
    glVertex3f(0.0f, -0.116f, 0.0f);
    glVertex3f(0.017f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.008f, -0.155f, 0.0f);
    glVertex3f(0.008f, -0.155f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.036f, -0.194f, 0.0f);
    glVertex3f(0.036f, -0.116f, 0.0f);
    glVertex3f(0.07f, -0.194f, 0.0f);
    glVertex3f(0.07f, -0.116f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.09f, -0.116f, 0.0f);
    glVertex3f(0.123f, -0.116f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.107f, -0.116f, 0.0f);
    glVertex3f(0.107f, -0.194f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.23f, -0.128f, 0.0f);
    glVertex3f(0.225f, -0.116f, 0.0f);
    glVertex3f(0.201f, -0.116f, 0.0f);
    glVertex3f(0.196f, -0.128f, 0.0f);
    glVertex3f(0.196f, -0.143f, 0.0f);
    glVertex3f(0.201f, -0.155f, 0.0f);
    glVertex3f(0.225f, -0.155f, 0.0f);
    glVertex3f(0.23f, -0.167f, 0.0f);
    glVertex3f(0.23f, -0.183f, 0.0f);
    glVertex3f(0.225f, -0.194f, 0.0f);
    glVertex3f(0.201f, -0.194f, 0.0f);
    glVertex3f(0.196f, -0.183f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.256f, -0.194f, 0.0f);
    glVertex3f(0.25f, -0.179f, 0.0f);
    glVertex3f(0.25f, -0.132f, 0.0f);
    glVertex3f(0.256f, -0.116f, 0.0f);
    glVertex3f(0.277f, -0.116f, 0.0f);
    glVertex3f(0.283f, -0.132f, 0.0f);
    glVertex3f(0.283f, -0.179f, 0.0f);
    glVertex3f(0.277f, -0.194f, 0.0f);
    glVertex3f(0.256f, -0.194f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.303f, -0.194f, 0.0f);
    glVertex3f(0.303f, -0.116f, 0.0f);
    glVertex3f(0.337f, -0.194f, 0.0f);
    glVertex3f(0.337f, -0.116f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.356f, -0.194f, 0.0f);
    glVertex3f(0.373f, -0.116f, 0.0f);
    glVertex3f(0.39f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.365f, -0.155f, 0.0f);
    glVertex3f(0.382f, -0.155f, 0.0f);
    glEnd();

    // W
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.41f, -0.116f, 0.0f);
    glVertex3f(0.418f, -0.194f, 0.0f);
    glVertex3f(0.427f, -0.147f, 0.0f);
    glVertex3f(0.435f, -0.194f, 0.0f);
    glVertex3f(0.443f, -0.116f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.463f, -0.194f, 0.0f);
    glVertex3f(0.48f, -0.116f, 0.0f);
    glVertex3f(0.497f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.471f, -0.155f, 0.0f);
    glVertex3f(0.488f, -0.155f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.516f, -0.194f, 0.0f);
    glVertex3f(0.516f, -0.116f, 0.0f);
    glVertex3f(0.55f, -0.194f, 0.0f);
    glVertex3f(0.55f, -0.116f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.603f, -0.116f, 0.0f);
    glVertex3f(0.57f, -0.116f, 0.0f);
    glVertex3f(0.57f, -0.194f, 0.0f);
    glVertex3f(0.603f, -0.194f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.57f, -0.155f, 0.0f);
    glVertex3f(0.593f, -0.155f, 0.0f);
    glEnd();


    // ---------- LAXMIKANT SANJAY TATE ----------
    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.55f, -0.241f, 0.0f);
    glVertex3f(-0.55f, -0.319f, 0.0f);
    glVertex3f(-0.516f, -0.319f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.497f, -0.319f, 0.0f);
    glVertex3f(-0.48f, -0.241f, 0.0f);
    glVertex3f(-0.463f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.488f, -0.28f, 0.0f);
    glVertex3f(-0.471f, -0.28f, 0.0f);
    glEnd();

    // X
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.443f, -0.241f, 0.0f);
    glVertex3f(-0.41f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.443f, -0.319f, 0.0f);
    glVertex3f(-0.41f, -0.241f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.39f, -0.319f, 0.0f);
    glVertex3f(-0.39f, -0.241f, 0.0f);
    glVertex3f(-0.373f, -0.28f, 0.0f);
    glVertex3f(-0.356f, -0.241f, 0.0f);
    glVertex3f(-0.356f, -0.319f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.32f, -0.319f, 0.0f);
    glVertex3f(-0.32f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.33f, -0.241f, 0.0f);
    glVertex3f(-0.31f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.33f, -0.319f, 0.0f);
    glVertex3f(-0.31f, -0.319f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.283f, -0.319f, 0.0f);
    glVertex3f(-0.283f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.25f, -0.241f, 0.0f);
    glVertex3f(-0.283f, -0.28f, 0.0f);
    glVertex3f(-0.25f, -0.319f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.23f, -0.319f, 0.0f);
    glVertex3f(-0.213f, -0.241f, 0.0f);
    glVertex3f(-0.196f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.222f, -0.28f, 0.0f);
    glVertex3f(-0.205f, -0.28f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.177f, -0.319f, 0.0f);
    glVertex3f(-0.177f, -0.241f, 0.0f);
    glVertex3f(-0.143f, -0.319f, 0.0f);
    glVertex3f(-0.143f, -0.241f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.123f, -0.241f, 0.0f);
    glVertex3f(-0.09f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.107f, -0.241f, 0.0f);
    glVertex3f(-0.107f, -0.319f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.017f, -0.252f, 0.0f);
    glVertex3f(0.012f, -0.241f, 0.0f);
    glVertex3f(-0.012f, -0.241f, 0.0f);
    glVertex3f(-0.017f, -0.252f, 0.0f);
    glVertex3f(-0.017f, -0.268f, 0.0f);
    glVertex3f(-0.012f, -0.28f, 0.0f);
    glVertex3f(0.012f, -0.28f, 0.0f);
    glVertex3f(0.017f, -0.292f, 0.0f);
    glVertex3f(0.017f, -0.307f, 0.0f);
    glVertex3f(0.012f, -0.319f, 0.0f);
    glVertex3f(-0.012f, -0.319f, 0.0f);
    glVertex3f(-0.017f, -0.307f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.036f, -0.319f, 0.0f);
    glVertex3f(0.053f, -0.241f, 0.0f);
    glVertex3f(0.07f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.045f, -0.28f, 0.0f);
    glVertex3f(0.062f, -0.28f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.09f, -0.319f, 0.0f);
    glVertex3f(0.09f, -0.241f, 0.0f);
    glVertex3f(0.123f, -0.319f, 0.0f);
    glVertex3f(0.123f, -0.241f, 0.0f);
    glEnd();

    // J
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.153f, -0.241f, 0.0f);
    glVertex3f(0.177f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.168f, -0.241f, 0.0f);
    glVertex3f(0.168f, -0.303f, 0.0f);
    glVertex3f(0.162f, -0.319f, 0.0f);
    glVertex3f(0.15f, -0.319f, 0.0f);
    glVertex3f(0.143f, -0.303f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.196f, -0.319f, 0.0f);
    glVertex3f(0.213f, -0.241f, 0.0f);
    glVertex3f(0.23f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.205f, -0.28f, 0.0f);
    glVertex3f(0.222f, -0.28f, 0.0f);
    glEnd();

    // Y
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.25f, -0.241f, 0.0f);
    glVertex3f(0.267f, -0.28f, 0.0f);
    glVertex3f(0.283f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.267f, -0.28f, 0.0f);
    glVertex3f(0.267f, -0.319f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.356f, -0.241f, 0.0f);
    glVertex3f(0.39f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.373f, -0.241f, 0.0f);
    glVertex3f(0.373f, -0.319f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.41f, -0.319f, 0.0f);
    glVertex3f(0.427f, -0.241f, 0.0f);
    glVertex3f(0.443f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.418f, -0.28f, 0.0f);
    glVertex3f(0.435f, -0.28f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.463f, -0.241f, 0.0f);
    glVertex3f(0.497f, -0.241f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.48f, -0.241f, 0.0f);
    glVertex3f(0.48f, -0.319f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.55f, -0.241f, 0.0f);
    glVertex3f(0.516f, -0.241f, 0.0f);
    glVertex3f(0.516f, -0.319f, 0.0f);
    glVertex3f(0.55f, -0.319f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.516f, -0.28f, 0.0f);
    glVertex3f(0.54f, -0.28f, 0.0f);
    glEnd();


    // ---------- SHUBHANGI KAILASH PARDESHI ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.65f, -0.377f, 0.0f);
    glVertex3f(-0.655f, -0.365f, 0.0f);
    glVertex3f(-0.678f, -0.365f, 0.0f);
    glVertex3f(-0.683f, -0.377f, 0.0f);
    glVertex3f(-0.683f, -0.393f, 0.0f);
    glVertex3f(-0.678f, -0.405f, 0.0f);
    glVertex3f(-0.655f, -0.405f, 0.0f);
    glVertex3f(-0.65f, -0.416f, 0.0f);
    glVertex3f(-0.65f, -0.432f, 0.0f);
    glVertex3f(-0.655f, -0.444f, 0.0f);
    glVertex3f(-0.678f, -0.444f, 0.0f);
    glVertex3f(-0.683f, -0.432f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.63f, -0.444f, 0.0f);
    glVertex3f(-0.63f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.596f, -0.444f, 0.0f);
    glVertex3f(-0.596f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.63f, -0.405f, 0.0f);
    glVertex3f(-0.596f, -0.405f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.577f, -0.365f, 0.0f);
    glVertex3f(-0.577f, -0.428f, 0.0f);
    glVertex3f(-0.57f, -0.444f, 0.0f);
    glVertex3f(-0.55f, -0.444f, 0.0f);
    glVertex3f(-0.543f, -0.428f, 0.0f);
    glVertex3f(-0.543f, -0.365f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.523f, -0.444f, 0.0f);
    glVertex3f(-0.523f, -0.365f, 0.0f);
    glVertex3f(-0.498f, -0.365f, 0.0f);
    glVertex3f(-0.49f, -0.377f, 0.0f);
    glVertex3f(-0.49f, -0.393f, 0.0f);
    glVertex3f(-0.498f, -0.405f, 0.0f);
    glVertex3f(-0.523f, -0.405f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.498f, -0.405f, 0.0f);
    glVertex3f(-0.49f, -0.416f, 0.0f);
    glVertex3f(-0.49f, -0.432f, 0.0f);
    glVertex3f(-0.498f, -0.444f, 0.0f);
    glVertex3f(-0.523f, -0.444f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.47f, -0.444f, 0.0f);
    glVertex3f(-0.47f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.436f, -0.444f, 0.0f);
    glVertex3f(-0.436f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.47f, -0.405f, 0.0f);
    glVertex3f(-0.436f, -0.405f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.417f, -0.444f, 0.0f);
    glVertex3f(-0.4f, -0.365f, 0.0f);
    glVertex3f(-0.383f, -0.444f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.408f, -0.405f, 0.0f);
    glVertex3f(-0.391f, -0.405f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.363f, -0.444f, 0.0f);
    glVertex3f(-0.363f, -0.365f, 0.0f);
    glVertex3f(-0.33f, -0.444f, 0.0f);
    glVertex3f(-0.33f, -0.365f, 0.0f);
    glEnd();

    // G
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.276f, -0.381f, 0.0f);
    glVertex3f(-0.283f, -0.365f, 0.0f);
    glVertex3f(-0.303f, -0.365f, 0.0f);
    glVertex3f(-0.31f, -0.381f, 0.0f);
    glVertex3f(-0.31f, -0.428f, 0.0f);
    glVertex3f(-0.303f, -0.444f, 0.0f);
    glVertex3f(-0.283f, -0.444f, 0.0f);
    glVertex3f(-0.276f, -0.428f, 0.0f);
    glVertex3f(-0.276f, -0.409f, 0.0f);
    glVertex3f(-0.293f, -0.409f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.24f, -0.444f, 0.0f);
    glVertex3f(-0.24f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.25f, -0.365f, 0.0f);
    glVertex3f(-0.23f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.25f, -0.444f, 0.0f);
    glVertex3f(-0.23f, -0.444f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.15f, -0.444f, 0.0f);
    glVertex3f(-0.15f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.116f, -0.365f, 0.0f);
    glVertex3f(-0.15f, -0.405f, 0.0f);
    glVertex3f(-0.116f, -0.444f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.097f, -0.444f, 0.0f);
    glVertex3f(-0.08f, -0.365f, 0.0f);
    glVertex3f(-0.063f, -0.444f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.088f, -0.405f, 0.0f);
    glVertex3f(-0.072f, -0.405f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.027f, -0.444f, 0.0f);
    glVertex3f(-0.027f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.037f, -0.365f, 0.0f);
    glVertex3f(-0.017f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.037f, -0.444f, 0.0f);
    glVertex3f(-0.017f, -0.444f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.01f, -0.365f, 0.0f);
    glVertex3f(0.01f, -0.444f, 0.0f);
    glVertex3f(0.043f, -0.444f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.063f, -0.444f, 0.0f);
    glVertex3f(0.08f, -0.365f, 0.0f);
    glVertex3f(0.097f, -0.444f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.072f, -0.405f, 0.0f);
    glVertex3f(0.088f, -0.405f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.15f, -0.377f, 0.0f);
    glVertex3f(0.145f, -0.365f, 0.0f);
    glVertex3f(0.122f, -0.365f, 0.0f);
    glVertex3f(0.116f, -0.377f, 0.0f);
    glVertex3f(0.116f, -0.393f, 0.0f);
    glVertex3f(0.122f, -0.405f, 0.0f);
    glVertex3f(0.145f, -0.405f, 0.0f);
    glVertex3f(0.15f, -0.416f, 0.0f);
    glVertex3f(0.15f, -0.432f, 0.0f);
    glVertex3f(0.145f, -0.444f, 0.0f);
    glVertex3f(0.122f, -0.444f, 0.0f);
    glVertex3f(0.116f, -0.432f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.17f, -0.444f, 0.0f);
    glVertex3f(0.17f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.203f, -0.444f, 0.0f);
    glVertex3f(0.203f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.17f, -0.405f, 0.0f);
    glVertex3f(0.203f, -0.405f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.276f, -0.444f, 0.0f);
    glVertex3f(0.276f, -0.365f, 0.0f);
    glVertex3f(0.303f, -0.365f, 0.0f);
    glVertex3f(0.31f, -0.377f, 0.0f);
    glVertex3f(0.31f, -0.393f, 0.0f);
    glVertex3f(0.303f, -0.405f, 0.0f);
    glVertex3f(0.276f, -0.405f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.33f, -0.444f, 0.0f);
    glVertex3f(0.347f, -0.365f, 0.0f);
    glVertex3f(0.363f, -0.444f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.338f, -0.405f, 0.0f);
    glVertex3f(0.355f, -0.405f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.444f, 0.0f);
    glVertex3f(0.383f, -0.365f, 0.0f);
    glVertex3f(0.41f, -0.365f, 0.0f);
    glVertex3f(0.417f, -0.377f, 0.0f);
    glVertex3f(0.417f, -0.393f, 0.0f);
    glVertex3f(0.41f, -0.405f, 0.0f);
    glVertex3f(0.383f, -0.405f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.4f, -0.405f, 0.0f);
    glVertex3f(0.417f, -0.444f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.436f, -0.444f, 0.0f);
    glVertex3f(0.436f, -0.365f, 0.0f);
    glVertex3f(0.457f, -0.365f, 0.0f);
    glVertex3f(0.47f, -0.389f, 0.0f);
    glVertex3f(0.47f, -0.42f, 0.0f);
    glVertex3f(0.457f, -0.444f, 0.0f);
    glVertex3f(0.436f, -0.444f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.523f, -0.365f, 0.0f);
    glVertex3f(0.49f, -0.365f, 0.0f);
    glVertex3f(0.49f, -0.444f, 0.0f);
    glVertex3f(0.523f, -0.444f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.49f, -0.405f, 0.0f);
    glVertex3f(0.513f, -0.405f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.577f, -0.377f, 0.0f);
    glVertex3f(0.572f, -0.365f, 0.0f);
    glVertex3f(0.548f, -0.365f, 0.0f);
    glVertex3f(0.543f, -0.377f, 0.0f);
    glVertex3f(0.543f, -0.393f, 0.0f);
    glVertex3f(0.548f, -0.405f, 0.0f);
    glVertex3f(0.572f, -0.405f, 0.0f);
    glVertex3f(0.577f, -0.416f, 0.0f);
    glVertex3f(0.577f, -0.432f, 0.0f);
    glVertex3f(0.572f, -0.444f, 0.0f);
    glVertex3f(0.548f, -0.444f, 0.0f);
    glVertex3f(0.543f, -0.432f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.596f, -0.444f, 0.0f);
    glVertex3f(0.596f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.63f, -0.444f, 0.0f);
    glVertex3f(0.63f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.596f, -0.405f, 0.0f);
    glVertex3f(0.63f, -0.405f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.666f, -0.444f, 0.0f);
    glVertex3f(0.666f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.656f, -0.365f, 0.0f);
    glVertex3f(0.677f, -0.365f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.656f, -0.444f, 0.0f);
    glVertex3f(0.677f, -0.444f, 0.0f);
    glEnd();


    // ---------- SANKALP NILESH CHORDIA ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.543f, -0.502f, 0.0f);
    glVertex3f(-0.548f, -0.49f, 0.0f);
    glVertex3f(-0.572f, -0.49f, 0.0f);
    glVertex3f(-0.577f, -0.502f, 0.0f);
    glVertex3f(-0.577f, -0.518f, 0.0f);
    glVertex3f(-0.572f, -0.529f, 0.0f);
    glVertex3f(-0.548f, -0.529f, 0.0f);
    glVertex3f(-0.543f, -0.541f, 0.0f);
    glVertex3f(-0.543f, -0.557f, 0.0f);
    glVertex3f(-0.548f, -0.569f, 0.0f);
    glVertex3f(-0.572f, -0.569f, 0.0f);
    glVertex3f(-0.577f, -0.557f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.523f, -0.569f, 0.0f);
    glVertex3f(-0.507f, -0.49f, 0.0f);
    glVertex3f(-0.49f, -0.569f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.515f, -0.529f, 0.0f);
    glVertex3f(-0.498f, -0.529f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.47f, -0.569f, 0.0f);
    glVertex3f(-0.47f, -0.49f, 0.0f);
    glVertex3f(-0.436f, -0.569f, 0.0f);
    glVertex3f(-0.436f, -0.49f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.417f, -0.569f, 0.0f);
    glVertex3f(-0.417f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.383f, -0.49f, 0.0f);
    glVertex3f(-0.417f, -0.529f, 0.0f);
    glVertex3f(-0.383f, -0.569f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.363f, -0.569f, 0.0f);
    glVertex3f(-0.347f, -0.49f, 0.0f);
    glVertex3f(-0.33f, -0.569f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.355f, -0.529f, 0.0f);
    glVertex3f(-0.338f, -0.529f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.31f, -0.49f, 0.0f);
    glVertex3f(-0.31f, -0.569f, 0.0f);
    glVertex3f(-0.276f, -0.569f, 0.0f);
    glEnd();

    // P
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.257f, -0.569f, 0.0f);
    glVertex3f(-0.257f, -0.49f, 0.0f);
    glVertex3f(-0.23f, -0.49f, 0.0f);
    glVertex3f(-0.223f, -0.502f, 0.0f);
    glVertex3f(-0.223f, -0.518f, 0.0f);
    glVertex3f(-0.23f, -0.529f, 0.0f);
    glVertex3f(-0.257f, -0.529f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.15f, -0.569f, 0.0f);
    glVertex3f(-0.15f, -0.49f, 0.0f);
    glVertex3f(-0.116f, -0.569f, 0.0f);
    glVertex3f(-0.116f, -0.49f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.08f, -0.569f, 0.0f);
    glVertex3f(-0.08f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.09f, -0.49f, 0.0f);
    glVertex3f(-0.07f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.09f, -0.569f, 0.0f);
    glVertex3f(-0.07f, -0.569f, 0.0f);
    glEnd();

    // L
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.043f, -0.49f, 0.0f);
    glVertex3f(-0.043f, -0.569f, 0.0f);
    glVertex3f(-0.01f, -0.569f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.043f, -0.49f, 0.0f);
    glVertex3f(0.01f, -0.49f, 0.0f);
    glVertex3f(0.01f, -0.569f, 0.0f);
    glVertex3f(0.043f, -0.569f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.01f, -0.529f, 0.0f);
    glVertex3f(0.033f, -0.529f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.097f, -0.502f, 0.0f);
    glVertex3f(0.092f, -0.49f, 0.0f);
    glVertex3f(0.068f, -0.49f, 0.0f);
    glVertex3f(0.063f, -0.502f, 0.0f);
    glVertex3f(0.063f, -0.518f, 0.0f);
    glVertex3f(0.068f, -0.529f, 0.0f);
    glVertex3f(0.092f, -0.529f, 0.0f);
    glVertex3f(0.097f, -0.541f, 0.0f);
    glVertex3f(0.097f, -0.557f, 0.0f);
    glVertex3f(0.092f, -0.569f, 0.0f);
    glVertex3f(0.068f, -0.569f, 0.0f);
    glVertex3f(0.063f, -0.557f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.116f, -0.569f, 0.0f);
    glVertex3f(0.116f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.15f, -0.569f, 0.0f);
    glVertex3f(0.15f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.116f, -0.529f, 0.0f);
    glVertex3f(0.15f, -0.529f, 0.0f);
    glEnd();

    // C
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.257f, -0.49f, 0.0f);
    glVertex3f(0.23f, -0.49f, 0.0f);
    glVertex3f(0.223f, -0.506f, 0.0f);
    glVertex3f(0.223f, -0.553f, 0.0f);
    glVertex3f(0.23f, -0.569f, 0.0f);
    glVertex3f(0.257f, -0.569f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.276f, -0.569f, 0.0f);
    glVertex3f(0.276f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.31f, -0.569f, 0.0f);
    glVertex3f(0.31f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.276f, -0.529f, 0.0f);
    glVertex3f(0.31f, -0.529f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.336f, -0.569f, 0.0f);
    glVertex3f(0.33f, -0.553f, 0.0f);
    glVertex3f(0.33f, -0.506f, 0.0f);
    glVertex3f(0.336f, -0.49f, 0.0f);
    glVertex3f(0.357f, -0.49f, 0.0f);
    glVertex3f(0.363f, -0.506f, 0.0f);
    glVertex3f(0.363f, -0.553f, 0.0f);
    glVertex3f(0.357f, -0.569f, 0.0f);
    glVertex3f(0.336f, -0.569f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.569f, 0.0f);
    glVertex3f(0.383f, -0.49f, 0.0f);
    glVertex3f(0.41f, -0.49f, 0.0f);
    glVertex3f(0.417f, -0.502f, 0.0f);
    glVertex3f(0.417f, -0.518f, 0.0f);
    glVertex3f(0.41f, -0.529f, 0.0f);
    glVertex3f(0.383f, -0.529f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.4f, -0.529f, 0.0f);
    glVertex3f(0.417f, -0.569f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.436f, -0.569f, 0.0f);
    glVertex3f(0.436f, -0.49f, 0.0f);
    glVertex3f(0.457f, -0.49f, 0.0f);
    glVertex3f(0.47f, -0.514f, 0.0f);
    glVertex3f(0.47f, -0.545f, 0.0f);
    glVertex3f(0.457f, -0.569f, 0.0f);
    glVertex3f(0.436f, -0.569f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.507f, -0.569f, 0.0f);
    glVertex3f(0.507f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.496f, -0.49f, 0.0f);
    glVertex3f(0.517f, -0.49f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.496f, -0.569f, 0.0f);
    glVertex3f(0.517f, -0.569f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.543f, -0.569f, 0.0f);
    glVertex3f(0.56f, -0.49f, 0.0f);
    glVertex3f(0.577f, -0.569f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.551f, -0.529f, 0.0f);
    glVertex3f(0.568f, -0.529f, 0.0f);
    glEnd();


    // ---------- SIDDHESH SHARAD HANDE ----------
    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.516f, -0.627f, 0.0f);
    glVertex3f(-0.521f, -0.615f, 0.0f);
    glVertex3f(-0.545f, -0.615f, 0.0f);
    glVertex3f(-0.55f, -0.627f, 0.0f);
    glVertex3f(-0.55f, -0.642f, 0.0f);
    glVertex3f(-0.545f, -0.654f, 0.0f);
    glVertex3f(-0.521f, -0.654f, 0.0f);
    glVertex3f(-0.516f, -0.666f, 0.0f);
    glVertex3f(-0.516f, -0.682f, 0.0f);
    glVertex3f(-0.521f, -0.693f, 0.0f);
    glVertex3f(-0.545f, -0.693f, 0.0f);
    glVertex3f(-0.55f, -0.682f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.48f, -0.693f, 0.0f);
    glVertex3f(-0.48f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.49f, -0.615f, 0.0f);
    glVertex3f(-0.47f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.49f, -0.693f, 0.0f);
    glVertex3f(-0.47f, -0.693f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.443f, -0.693f, 0.0f);
    glVertex3f(-0.443f, -0.615f, 0.0f);
    glVertex3f(-0.423f, -0.615f, 0.0f);
    glVertex3f(-0.41f, -0.639f, 0.0f);
    glVertex3f(-0.41f, -0.67f, 0.0f);
    glVertex3f(-0.423f, -0.693f, 0.0f);
    glVertex3f(-0.443f, -0.693f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.39f, -0.693f, 0.0f);
    glVertex3f(-0.39f, -0.615f, 0.0f);
    glVertex3f(-0.37f, -0.615f, 0.0f);
    glVertex3f(-0.356f, -0.639f, 0.0f);
    glVertex3f(-0.356f, -0.67f, 0.0f);
    glVertex3f(-0.37f, -0.693f, 0.0f);
    glVertex3f(-0.39f, -0.693f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.337f, -0.693f, 0.0f);
    glVertex3f(-0.337f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.303f, -0.693f, 0.0f);
    glVertex3f(-0.303f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.337f, -0.654f, 0.0f);
    glVertex3f(-0.303f, -0.654f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.25f, -0.615f, 0.0f);
    glVertex3f(-0.283f, -0.615f, 0.0f);
    glVertex3f(-0.283f, -0.693f, 0.0f);
    glVertex3f(-0.25f, -0.693f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.283f, -0.654f, 0.0f);
    glVertex3f(-0.26f, -0.654f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.196f, -0.627f, 0.0f);
    glVertex3f(-0.201f, -0.615f, 0.0f);
    glVertex3f(-0.225f, -0.615f, 0.0f);
    glVertex3f(-0.23f, -0.627f, 0.0f);
    glVertex3f(-0.23f, -0.642f, 0.0f);
    glVertex3f(-0.225f, -0.654f, 0.0f);
    glVertex3f(-0.201f, -0.654f, 0.0f);
    glVertex3f(-0.196f, -0.666f, 0.0f);
    glVertex3f(-0.196f, -0.682f, 0.0f);
    glVertex3f(-0.201f, -0.693f, 0.0f);
    glVertex3f(-0.225f, -0.693f, 0.0f);
    glVertex3f(-0.23f, -0.682f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.177f, -0.693f, 0.0f);
    glVertex3f(-0.177f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.143f, -0.693f, 0.0f);
    glVertex3f(-0.143f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.177f, -0.654f, 0.0f);
    glVertex3f(-0.143f, -0.654f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.036f, -0.627f, 0.0f);
    glVertex3f(-0.042f, -0.615f, 0.0f);
    glVertex3f(-0.065f, -0.615f, 0.0f);
    glVertex3f(-0.07f, -0.627f, 0.0f);
    glVertex3f(-0.07f, -0.642f, 0.0f);
    glVertex3f(-0.065f, -0.654f, 0.0f);
    glVertex3f(-0.042f, -0.654f, 0.0f);
    glVertex3f(-0.036f, -0.666f, 0.0f);
    glVertex3f(-0.036f, -0.682f, 0.0f);
    glVertex3f(-0.042f, -0.693f, 0.0f);
    glVertex3f(-0.065f, -0.693f, 0.0f);
    glVertex3f(-0.07f, -0.682f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.017f, -0.693f, 0.0f);
    glVertex3f(-0.017f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.017f, -0.693f, 0.0f);
    glVertex3f(0.017f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.017f, -0.654f, 0.0f);
    glVertex3f(0.017f, -0.654f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.036f, -0.693f, 0.0f);
    glVertex3f(0.053f, -0.615f, 0.0f);
    glVertex3f(0.07f, -0.693f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.045f, -0.654f, 0.0f);
    glVertex3f(0.062f, -0.654f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.09f, -0.693f, 0.0f);
    glVertex3f(0.09f, -0.615f, 0.0f);
    glVertex3f(0.117f, -0.615f, 0.0f);
    glVertex3f(0.123f, -0.627f, 0.0f);
    glVertex3f(0.123f, -0.642f, 0.0f);
    glVertex3f(0.117f, -0.654f, 0.0f);
    glVertex3f(0.09f, -0.654f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.107f, -0.654f, 0.0f);
    glVertex3f(0.123f, -0.693f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.143f, -0.693f, 0.0f);
    glVertex3f(0.16f, -0.615f, 0.0f);
    glVertex3f(0.177f, -0.693f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.152f, -0.654f, 0.0f);
    glVertex3f(0.168f, -0.654f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.196f, -0.693f, 0.0f);
    glVertex3f(0.196f, -0.615f, 0.0f);
    glVertex3f(0.217f, -0.615f, 0.0f);
    glVertex3f(0.23f, -0.639f, 0.0f);
    glVertex3f(0.23f, -0.67f, 0.0f);
    glVertex3f(0.217f, -0.693f, 0.0f);
    glVertex3f(0.196f, -0.693f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.303f, -0.693f, 0.0f);
    glVertex3f(0.303f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.337f, -0.693f, 0.0f);
    glVertex3f(0.337f, -0.615f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.303f, -0.654f, 0.0f);
    glVertex3f(0.337f, -0.654f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.356f, -0.693f, 0.0f);
    glVertex3f(0.373f, -0.615f, 0.0f);
    glVertex3f(0.39f, -0.693f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.365f, -0.654f, 0.0f);
    glVertex3f(0.382f, -0.654f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.41f, -0.693f, 0.0f);
    glVertex3f(0.41f, -0.615f, 0.0f);
    glVertex3f(0.443f, -0.693f, 0.0f);
    glVertex3f(0.443f, -0.615f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.463f, -0.693f, 0.0f);
    glVertex3f(0.463f, -0.615f, 0.0f);
    glVertex3f(0.483f, -0.615f, 0.0f);
    glVertex3f(0.497f, -0.639f, 0.0f);
    glVertex3f(0.497f, -0.67f, 0.0f);
    glVertex3f(0.483f, -0.693f, 0.0f);
    glVertex3f(0.463f, -0.693f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.55f, -0.615f, 0.0f);
    glVertex3f(0.516f, -0.615f, 0.0f);
    glVertex3f(0.516f, -0.693f, 0.0f);
    glVertex3f(0.55f, -0.693f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.516f, -0.654f, 0.0f);
    glVertex3f(0.54f, -0.654f, 0.0f);
    glEnd();


    // ---------- VAIBHAV KANTRAO DESHMUKH ----------
    // V
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.63f, -0.74f, 0.0f);
    glVertex3f(-0.613f, -0.818f, 0.0f);
    glVertex3f(-0.596f, -0.74f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.577f, -0.818f, 0.0f);
    glVertex3f(-0.56f, -0.74f, 0.0f);
    glVertex3f(-0.543f, -0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.568f, -0.779f, 0.0f);
    glVertex3f(-0.551f, -0.779f, 0.0f);
    glEnd();

    // I
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.507f, -0.818f, 0.0f);
    glVertex3f(-0.507f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.517f, -0.74f, 0.0f);
    glVertex3f(-0.496f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.517f, -0.818f, 0.0f);
    glVertex3f(-0.496f, -0.818f, 0.0f);
    glEnd();

    // B
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.47f, -0.818f, 0.0f);
    glVertex3f(-0.47f, -0.74f, 0.0f);
    glVertex3f(-0.445f, -0.74f, 0.0f);
    glVertex3f(-0.436f, -0.752f, 0.0f);
    glVertex3f(-0.436f, -0.767f, 0.0f);
    glVertex3f(-0.445f, -0.779f, 0.0f);
    glVertex3f(-0.47f, -0.779f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.445f, -0.779f, 0.0f);
    glVertex3f(-0.436f, -0.791f, 0.0f);
    glVertex3f(-0.436f, -0.806f, 0.0f);
    glVertex3f(-0.445f, -0.818f, 0.0f);
    glVertex3f(-0.47f, -0.818f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.417f, -0.818f, 0.0f);
    glVertex3f(-0.417f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.383f, -0.818f, 0.0f);
    glVertex3f(-0.383f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.417f, -0.779f, 0.0f);
    glVertex3f(-0.383f, -0.779f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.363f, -0.818f, 0.0f);
    glVertex3f(-0.347f, -0.74f, 0.0f);
    glVertex3f(-0.33f, -0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.355f, -0.779f, 0.0f);
    glVertex3f(-0.338f, -0.779f, 0.0f);
    glEnd();

    // V
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.31f, -0.74f, 0.0f);
    glVertex3f(-0.293f, -0.818f, 0.0f);
    glVertex3f(-0.276f, -0.74f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.203f, -0.818f, 0.0f);
    glVertex3f(-0.203f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.17f, -0.74f, 0.0f);
    glVertex3f(-0.203f, -0.779f, 0.0f);
    glVertex3f(-0.17f, -0.818f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.15f, -0.818f, 0.0f);
    glVertex3f(-0.133f, -0.74f, 0.0f);
    glVertex3f(-0.116f, -0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.142f, -0.779f, 0.0f);
    glVertex3f(-0.125f, -0.779f, 0.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.097f, -0.818f, 0.0f);
    glVertex3f(-0.097f, -0.74f, 0.0f);
    glVertex3f(-0.063f, -0.818f, 0.0f);
    glVertex3f(-0.063f, -0.74f, 0.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.043f, -0.74f, 0.0f);
    glVertex3f(-0.01f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-0.027f, -0.74f, 0.0f);
    glVertex3f(-0.027f, -0.818f, 0.0f);
    glEnd();

    // R
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.01f, -0.818f, 0.0f);
    glVertex3f(0.01f, -0.74f, 0.0f);
    glVertex3f(0.037f, -0.74f, 0.0f);
    glVertex3f(0.043f, -0.752f, 0.0f);
    glVertex3f(0.043f, -0.767f, 0.0f);
    glVertex3f(0.037f, -0.779f, 0.0f);
    glVertex3f(0.01f, -0.779f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.027f, -0.779f, 0.0f);
    glVertex3f(0.043f, -0.818f, 0.0f);
    glEnd();

    // A
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.063f, -0.818f, 0.0f);
    glVertex3f(0.08f, -0.74f, 0.0f);
    glVertex3f(0.097f, -0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.072f, -0.779f, 0.0f);
    glVertex3f(0.088f, -0.779f, 0.0f);
    glEnd();

    // O
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.123f, -0.818f, 0.0f);
    glVertex3f(0.116f, -0.802f, 0.0f);
    glVertex3f(0.116f, -0.755f, 0.0f);
    glVertex3f(0.123f, -0.74f, 0.0f);
    glVertex3f(0.143f, -0.74f, 0.0f);
    glVertex3f(0.15f, -0.755f, 0.0f);
    glVertex3f(0.15f, -0.802f, 0.0f);
    glVertex3f(0.143f, -0.818f, 0.0f);
    glVertex3f(0.123f, -0.818f, 0.0f);
    glEnd();

    // D
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.223f, -0.818f, 0.0f);
    glVertex3f(0.223f, -0.74f, 0.0f);
    glVertex3f(0.243f, -0.74f, 0.0f);
    glVertex3f(0.257f, -0.763f, 0.0f);
    glVertex3f(0.257f, -0.795f, 0.0f);
    glVertex3f(0.243f, -0.818f, 0.0f);
    glVertex3f(0.223f, -0.818f, 0.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.31f, -0.74f, 0.0f);
    glVertex3f(0.276f, -0.74f, 0.0f);
    glVertex3f(0.276f, -0.818f, 0.0f);
    glVertex3f(0.31f, -0.818f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.276f, -0.779f, 0.0f);
    glVertex3f(0.3f, -0.779f, 0.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.363f, -0.752f, 0.0f);
    glVertex3f(0.358f, -0.74f, 0.0f);
    glVertex3f(0.335f, -0.74f, 0.0f);
    glVertex3f(0.33f, -0.752f, 0.0f);
    glVertex3f(0.33f, -0.767f, 0.0f);
    glVertex3f(0.335f, -0.779f, 0.0f);
    glVertex3f(0.358f, -0.779f, 0.0f);
    glVertex3f(0.363f, -0.791f, 0.0f);
    glVertex3f(0.363f, -0.806f, 0.0f);
    glVertex3f(0.358f, -0.818f, 0.0f);
    glVertex3f(0.335f, -0.818f, 0.0f);
    glVertex3f(0.33f, -0.806f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.818f, 0.0f);
    glVertex3f(0.383f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.417f, -0.818f, 0.0f);
    glVertex3f(0.417f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.383f, -0.779f, 0.0f);
    glVertex3f(0.417f, -0.779f, 0.0f);
    glEnd();

    // M
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.436f, -0.818f, 0.0f);
    glVertex3f(0.436f, -0.74f, 0.0f);
    glVertex3f(0.453f, -0.779f, 0.0f);
    glVertex3f(0.47f, -0.74f, 0.0f);
    glVertex3f(0.47f, -0.818f, 0.0f);
    glEnd();

    // U
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.49f, -0.74f, 0.0f);
    glVertex3f(0.49f, -0.802f, 0.0f);
    glVertex3f(0.496f, -0.818f, 0.0f);
    glVertex3f(0.517f, -0.818f, 0.0f);
    glVertex3f(0.523f, -0.802f, 0.0f);
    glVertex3f(0.523f, -0.74f, 0.0f);
    glEnd();

    // K
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.543f, -0.818f, 0.0f);
    glVertex3f(0.543f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.577f, -0.74f, 0.0f);
    glVertex3f(0.543f, -0.779f, 0.0f);
    glVertex3f(0.577f, -0.818f, 0.0f);
    glEnd();

    // H
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.596f, -0.818f, 0.0f);
    glVertex3f(0.596f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.63f, -0.818f, 0.0f);
    glVertex3f(0.63f, -0.74f, 0.0f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.596f, -0.779f, 0.0f);
    glVertex3f(0.63f, -0.779f, 0.0f);
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
