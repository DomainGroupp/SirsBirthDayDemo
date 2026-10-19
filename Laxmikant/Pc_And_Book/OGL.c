// Header File
#include <windows.h>
#include <stdio.h> // For File I/O
#include <stdlib.h> // for exit()

// OpenGL Releted Header File 
#include <gl/GL.h>
#include <GL/glu.h> // change no.1

#include "OGL.h"

// Link with OpenGL Import Libraries 
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")   

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
GLint keyPress = 0;

// Texture related global variables // change no.1
GLuint Texture_Setup;
GLUquadric* quadric = NULL;

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
                            TEXT("My First RTR7 Program : Laxmikant Sanjay Tate"),
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
    void PrintGLInfo(void);
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
                } 
                break;

            case 'P':
            case 'p':
                keyPress = 1;
                break;

            case 'S':
            case 's':
                keyPress = 2;
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
    BOOL LoadGLTexture(GLuint*, TCHAR[]);    // change no.2
    
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

    // Print OpenGL Info
    PrintGLInfo(); 

    // choose screen clearing color as BLUE  
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (LoadGLTexture(&Texture_Setup, MAKEINTRESOURCE(ID_BITMAP_SETUP)) == FALSE)   // change no.3
    {
        fprintf(gpFile, "IN INITIALIZE LoadGLTexture FAILED !!!");
        return(-6);
    }

    glEnable(GL_TEXTURE_2D);    // change no.4

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void PrintGLInfo(void)
{
    // code
    fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
}

BOOL LoadGLTexture(GLuint* Texture, TCHAR imageResourceId[])    //change no.5
{
    // variable declarations
    HBITMAP hBitmap = NULL;
    BITMAP bmp;
    BOOL bResult = FALSE;

    // code
    // Load The Bitmap as Image
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    // Null Pass Kela First Parameter Manje hInstance dyayla aplyala  Swatacha without using global variable for handle
    // width ani height dyayche nahi 4, 5 parameter la karan 0 deun apan sangat aahot bitmap ahe, cursor kiva icon nahi
    // mazya image la device independant bitmap manun load kar mazya exe madhe

    if (hBitmap == NULL)
    {
        fprintf(gpFile, "Loadimage Failed..!!!");
        return(FALSE);
    }

    if (hBitmap)
    {
        bResult = TRUE;

        // get bitmap structure from the loaded bitmap image
        GetObject(hBitmap, sizeof(BITMAP), &bmp);
        // Generate openGL texture object
        glGenTextures(1, Texture); // rikama texture tayar zala
        // bind to the newly created object
        glBindTexture(GL_TEXTURE_2D, *Texture);
        // Unpack the image into memory for faster loading
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
        glBindTexture(GL_TEXTURE_2D, 0); // unbind karat aahot asa kalta 0 deun
        DeleteObject(hBitmap);

        hBitmap = NULL;
    }

    return(bResult);
    // generate openGL
}

void resize(int width, int height)  
{
    // code
    if (height <= 0)
        height = 1; // height should not be zero for infinity

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    // Perspective Project // change no.3 
    // jas disat tasa
    gluPerspective(45.0f,
        (GLfloat)width / (GLfloat)height,
        0.1f,
        100.0f);

}

void render(void) 
{
    void Computer_SetUp(void);
   
    // code
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -4.0f); 

    if (keyPress == 1)
    {
        Computer_SetUp();
    }

    if (keyPress == 2)
    {

        glBindTexture(GL_TEXTURE_2D, Texture_Setup);    // change no.6

        // Rectanlge Drawing Code
        glBegin(GL_QUADS);      // change no.7
        // Right Top
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(2.0f, 1.0f, 0.0f);

        // Left Top
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-2.0f, 1.0f, 0.0f);

        // left bottom
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-2.0f, -1.0f, 0.0f);

        // right bottom
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(2.0f, -1.0f, 0.0f);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    // Do double buffering
    SwapBuffers(ghdc);

}

void update(void) 
{
    // code
}

void Computer_SetUp(void)
{
    
    // Monitor
    // desk top
    glBegin(GL_QUADS);
        glColor3f(0.55f, 0.40f, 0.27f);                 
        glVertex3f(-0.80f, -0.80f, 0.0f);
        glVertex3f( 0.80f, -0.80f, 0.0f);
        glVertex3f( 0.80f, -0.30f, 0.0f);
        glVertex3f(-0.80f, -0.30f, 0.0f);
    glEnd();
    
    // back edge
    glBegin(GL_QUADS);
        glColor3f(0.38f, 0.26f, 0.17f);                 
        glVertex3f(-0.80f, -0.30f, 0.0f);
        glVertex3f( 0.80f, -0.30f, 0.0f);
        glVertex3f( 0.80f, -0.29f, 0.0f);
        glVertex3f(-0.80f, -0.29f, 0.0f);
    glEnd();

    // front edge
    glBegin(GL_QUADS);
        glColor3f(0.42f, 0.30f, 0.20f);                 
        glVertex3f(-0.80f, -0.86f, 0.0f);
        glVertex3f( 0.80f, -0.86f, 0.0f);
        glVertex3f( 0.80f, -0.80f, 0.0f);
        glVertex3f(-0.80f, -0.80f, 0.0f);
    glEnd();


    // bezel
    glBegin(GL_QUADS);
        glColor3f(0.12f, 0.12f, 0.14f);                 
        glVertex3f(-0.50f, -0.10f, 0.0f);
        glVertex3f( 0.50f, -0.10f, 0.0f);
        glVertex3f( 0.50f,  0.80f, 0.0f);
        glVertex3f(-0.50f,  0.80f, 0.0f);
    glEnd();

    // Monitor Screen
    glBegin(GL_QUADS);
        glColor3f(0.10f, 0.25f, 0.55f);                 // screen: dark blue bottom
        glVertex3f(-0.475f, -0.03f, 0.0f);
        glVertex3f( 0.475f, -0.03f, 0.0f);
        glColor3f(0.35f, 0.65f, 0.95f);                 // screen: light blue top
        glVertex3f( 0.475f,  0.77f, 0.0f);
        glVertex3f(-0.475f,  0.77f, 0.0f);
    glEnd();
 
    // power LED
    glBegin(GL_QUADS);
        glColor3f(0.20f, 0.90f, 0.30f);                 
        glVertex3f(0.44f, -0.075f, 0.0f);
        glVertex3f(0.455f, -0.075f, 0.0f);
        glVertex3f(0.455f, -0.060f, 0.0f);
        glVertex3f(0.44f, -0.060f, 0.0f);
    glEnd();


    // Monitor Stand
    // base
    glBegin(GL_QUADS);
        glColor3f(0.25f, 0.25f, 0.28f);                 
        glVertex3f(-0.22f, -0.38f, 0.0f);
        glVertex3f( 0.22f, -0.38f, 0.0f);
        glVertex3f( 0.16f, -0.33f, 0.0f);
        glVertex3f(-0.16f, -0.33f, 0.0f);
    glEnd();

    // neck
    glBegin(GL_QUADS);
        glColor3f(0.30f, 0.30f, 0.33f);                 
        glVertex3f(-0.05f, -0.34f, 0.0f);
        glVertex3f( 0.05f, -0.34f, 0.0f);
        glVertex3f( 0.04f, -0.06f, 0.0f);
        glVertex3f(-0.04f, -0.06f, 0.0f);
    glEnd();

    // neck highlight
    glBegin(GL_QUADS);
        glColor3f(0.45f, 0.45f, 0.48f);                 
        glVertex3f(-0.01f, -0.32f, 0.0f);
        glVertex3f( 0.01f, -0.32f, 0.0f);
        glVertex3f( 0.01f, -0.08f, 0.0f);
        glVertex3f(-0.01f, -0.08f, 0.0f);
    glEnd();      

    // stand outlines white
    glBegin(GL_LINE_LOOP);      
        glColor3f(0.10f, 0.10f, 0.12f); 
        glVertex3f(-0.05f, -0.34f, 0.0f);
        glVertex3f( 0.05f, -0.34f, 0.0f);
        glVertex3f( 0.04f, -0.06f, 0.0f);
        glVertex3f(-0.04f, -0.06f, 0.0f);
    glEnd();

    
    // KEYBOARD
    // Keyboard case
    glBegin(GL_QUADS);
        glColor3f(0.20f, 0.20f, 0.22f);                 
        glVertex3f(-0.30f, -0.73f, 0.0f);
        glVertex3f( 0.30f, -0.73f, 0.0f);
        glVertex3f( 0.30f, -0.50f, 0.0f);
        glVertex3f(-0.30f, -0.50f, 0.0f);
    glEnd();

    // Keyboard Key
    // First Row
    glPushMatrix();
    {
        glTranslatef(0.0f, -0.55f, 0.0f);
    
        // Key 1
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2875f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.038f, 0.0f);
            glVertex3f(-0.2875f, 0.038f, 0.0f);
        glEnd();    

        // Key 2
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2425f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.038f, 0.0f);
            glVertex3f(-0.2425f, 0.038f, 0.0f);
        glEnd();
        
        // Key 3
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1975f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.038f, 0.0f);
            glVertex3f(-0.1975f, 0.038f, 0.0f);
        glEnd();

        // key 4
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1525f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.038f, 0.0f);
            glVertex3f(-0.1525f, 0.038f, 0.0f);
        glEnd();

        // key 5
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1075f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.038f, 0.0f);
            glVertex3f(-0.1075f, 0.038f, 0.0f);
        glEnd();

        // key 6
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0625f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.038f, 0.0f);
            glVertex3f(-0.0625f, 0.038f, 0.0f);
        glEnd();

        // key 7
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.038f, 0.0f);
            glVertex3f(-0.0175f, 0.038f, 0.0f);
        glEnd();

        // key 8
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0275f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.038f, 0.0f);
            glVertex3f(0.0275f, 0.038f, 0.0f);
        glEnd();

        // key 9
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0725f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.038f, 0.0f);
            glVertex3f(0.0725f, 0.038f, 0.0f);
        glEnd();

        // key 10
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1175f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.038f, 0.0f);
            glVertex3f(0.1175f, 0.038f, 0.0f);
        glEnd();

        // key 11
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1625f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.038f, 0.0f);
            glVertex3f(0.1625f, 0.038f, 0.0f);
        glEnd();

        // key 12
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2075f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.038f, 0.0f);
            glVertex3f(0.2075f, 0.038f, 0.0f);
        glEnd();

        // key 13
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2525f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.038f, 0.0f);
            glVertex3f(0.2525f, 0.038f, 0.0f);
        glEnd();
    }
    glPopMatrix();

    
    // Second Row
    glPushMatrix();
    {
        glTranslatef(0.0f, -0.602f, 0.0f);
    
        // Key 1
        // Key 1
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2875f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.038f, 0.0f);
            glVertex3f(-0.2875f, 0.038f, 0.0f);
        glEnd();    

        // Key 2
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2425f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.038f, 0.0f);
            glVertex3f(-0.2425f, 0.038f, 0.0f);
        glEnd();
        
        // Key 3
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1975f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.038f, 0.0f);
            glVertex3f(-0.1975f, 0.038f, 0.0f);
        glEnd();

        // key 4
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1525f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.038f, 0.0f);
            glVertex3f(-0.1525f, 0.038f, 0.0f);
        glEnd();

        // key 5
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1075f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.038f, 0.0f);
            glVertex3f(-0.1075f, 0.038f, 0.0f);
        glEnd();

        // key 6
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0625f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.038f, 0.0f);
            glVertex3f(-0.0625f, 0.038f, 0.0f);
        glEnd();

        // key 7
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.038f, 0.0f);
            glVertex3f(-0.0175f, 0.038f, 0.0f);
        glEnd();

        // key 8
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0275f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.038f, 0.0f);
            glVertex3f(0.0275f, 0.038f, 0.0f);
        glEnd();

        // key 9
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0725f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.038f, 0.0f);
            glVertex3f(0.0725f, 0.038f, 0.0f);
        glEnd();

        // key 10
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1175f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.038f, 0.0f);
            glVertex3f(0.1175f, 0.038f, 0.0f);
        glEnd();

        // key 11
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1625f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.038f, 0.0f);
            glVertex3f(0.1625f, 0.038f, 0.0f);
        glEnd();

        // key 12
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2075f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.038f, 0.0f);
            glVertex3f(0.2075f, 0.038f, 0.0f);
        glEnd();

        // key 13
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2525f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.038f, 0.0f);
            glVertex3f(0.2525f, 0.038f, 0.0f);
        glEnd();
    }
    glPopMatrix();


    // Third Row
    glPushMatrix();
    {
        glTranslatef(0.0f, -0.654f, 0.0f);

        // Key 1
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2875f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.038f, 0.0f);
            glVertex3f(-0.2875f, 0.038f, 0.0f);
        glEnd();    

        // Key 2
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2425f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.038f, 0.0f);
            glVertex3f(-0.2425f, 0.038f, 0.0f);
        glEnd();
        
        // Key 3
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1975f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.0f, 0.0f);
            glVertex3f(-0.1625f, 0.038f, 0.0f);
            glVertex3f(-0.1975f, 0.038f, 0.0f);
        glEnd();

        // key 4
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1525f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.0f, 0.0f);
            glVertex3f(-0.1175f, 0.038f, 0.0f);
            glVertex3f(-0.1525f, 0.038f, 0.0f);
        glEnd();

        // key 5
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1075f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.0f, 0.0f);
            glVertex3f(-0.0725f, 0.038f, 0.0f);
            glVertex3f(-0.1075f, 0.038f, 0.0f);
        glEnd();

        // key 6
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0625f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.0f, 0.0f);
            glVertex3f(-0.0275f, 0.038f, 0.0f);
            glVertex3f(-0.0625f, 0.038f, 0.0f);
        glEnd();

        // key 7
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.0f, 0.0f);
            glVertex3f( 0.0175f, 0.038f, 0.0f);
            glVertex3f(-0.0175f, 0.038f, 0.0f);
        glEnd();

        // key 8
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0275f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.0f, 0.0f);
            glVertex3f(0.0625f, 0.038f, 0.0f);
            glVertex3f(0.0275f, 0.038f, 0.0f);
        glEnd();

        // key 9
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.0725f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.0f, 0.0f);
            glVertex3f(0.1075f, 0.038f, 0.0f);
            glVertex3f(0.0725f, 0.038f, 0.0f);
        glEnd();

        // key 10
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1175f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.0f, 0.0f);
            glVertex3f(0.1525f, 0.038f, 0.0f);
            glVertex3f(0.1175f, 0.038f, 0.0f);
        glEnd();

        // key 11
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.1625f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.0f, 0.0f);
            glVertex3f(0.1975f, 0.038f, 0.0f);
            glVertex3f(0.1625f, 0.038f, 0.0f);
        glEnd();

        // key 12
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2075f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.038f, 0.0f);
            glVertex3f(0.2075f, 0.038f, 0.0f);
        glEnd();

        // key 13
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2525f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.038f, 0.0f);
            glVertex3f(0.2525f, 0.038f, 0.0f);
        glEnd();
    }
    glPopMatrix();

    // Fourth Row
    glPushMatrix();
    {
        glTranslatef(0.0f, -0.702f, 0.0f);
    
        // Key 1
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2875f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.0f, 0.0f);
            glVertex3f(-0.2525f, 0.038f, 0.0f);
            glVertex3f(-0.2875f, 0.038f, 0.0f);
        glEnd();    

        // Key 2
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.2425f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.0f, 0.0f);
            glVertex3f(-0.2075f, 0.038f, 0.0f);
            glVertex3f(-0.2425f, 0.038f, 0.0f);
        glEnd();
        
        // Key 3
        glBegin(GL_QUADS);
        glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(-0.1975f, 0.0f, 0.0f);
            glVertex3f(0.1960f, 0.0f, 0.0f); 
            glVertex3f(0.1960f, 0.038f, 0.0f);
            glVertex3f(-0.1975f, 0.038f, 0.0f);
        glEnd();

        // key 12
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2075f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.0f, 0.0f);
            glVertex3f(0.2425f, 0.038f, 0.0f);
            glVertex3f(0.2075f, 0.038f, 0.0f);
        glEnd();

        // key 13
        glBegin(GL_QUADS);
            glColor3f(0.88f, 0.88f, 0.90f);
            glVertex3f(0.2525f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.0f, 0.0f);
            glVertex3f(0.2875f, 0.038f, 0.0f);
            glVertex3f(0.2525f, 0.038f, 0.0f);
        glEnd();

    }
    glPopMatrix();



    // C Book
    // front cover
    glBegin(GL_QUADS);
        glColor3f(0.15f, 0.30f, 0.60f);
        glVertex2f(0.54f, -0.66f);
        glVertex2f(0.74f, -0.66f);
        glVertex2f(0.74f, -0.26f);
        glVertex2f(0.54f, -0.26f);
    glEnd();

    // darker strip on the left
    glBegin(GL_QUADS);
        glColor3f(0.08f, 0.18f, 0.40f);
        glVertex2f(0.54f, -0.66f);
        glVertex2f(0.552f, -0.66f);
        glVertex2f(0.552f, -0.26f);
        glVertex2f(0.54f, -0.26f);
    glEnd();

    // page edges strip 
    glBegin(GL_QUADS);
        glColor3f(0.95f, 0.92f, 0.80f);
        glVertex2f(0.74f, -0.655f);
        glVertex2f(0.752f, -0.655f);
        glVertex2f(0.752f, -0.265f);
        glVertex2f(0.74f, -0.265f);
    glEnd();

    // title band
    glBegin(GL_QUADS);
        glColor3f(0.95f, 0.95f, 0.97f);
        glVertex2f(0.565f, -0.34f);
        glVertex2f(0.725f, -0.34f);
        glVertex2f(0.725f, -0.29f);
        glVertex2f(0.565f, -0.29f);
    glEnd();


    //  "C" - left bar
    glBegin(GL_QUADS);
        glColor3f(0.98f, 0.85f, 0.30f);
        glVertex2f(0.59f, -0.56f);
        glVertex2f(0.62f, -0.56f);
        glVertex2f(0.62f, -0.38f);
        glVertex2f(0.59f, -0.38f);
    glEnd();

    // letter "C" - Horizontal top bar
    glBegin(GL_QUADS);
        glColor3f(0.98f, 0.85f, 0.30f);
        glVertex2f(0.59f, -0.41f);
        glVertex2f(0.69f, -0.41f);
        glVertex2f(0.69f, -0.38f);
        glVertex2f(0.59f, -0.38f);
    glEnd();

    // letter "C" - Horizontal bottom bar
    glBegin(GL_QUADS);
        glColor3f(0.98f, 0.85f, 0.30f);
        glVertex2f(0.59f, -0.56f);
        glVertex2f(0.69f, -0.56f);
        glVertex2f(0.69f, -0.53f);
        glVertex2f(0.59f, -0.53f);
    glEnd();

    // letter "C" - top Vertical Bar
    glBegin(GL_QUADS);
        glColor3f(0.98f, 0.85f, 0.30f);
        glVertex2f(0.66f, -0.45f);
        glVertex2f(0.69f, -0.45f);
        glVertex2f(0.69f, -0.38f);
        glVertex2f(0.66f, -0.38f);
    glEnd();

    // letter "C" - bottom Vertical Bar
    glBegin(GL_QUADS);
        glColor3f(0.98f, 0.85f, 0.30f);
        glVertex2f(0.66f, -0.56f);
        glVertex2f(0.69f, -0.56f);
        glVertex2f(0.69f, -0.49f);
        glVertex2f(0.66f, -0.49f);
    glEnd();

    // text lines First
    glBegin(GL_QUADS);
        glColor3f(0.80f, 0.85f, 0.95f);
        glVertex2f(0.57f, -0.605f);
        glVertex2f(0.66f, -0.605f);
        glVertex2f(0.66f, -0.597f);
        glVertex2f(0.57f, -0.597f);
    glEnd();

    //Text line Second
    glBegin(GL_QUADS);
        glColor3f(0.80f, 0.85f, 0.95f);
        glVertex2f(0.57f, -0.625f);
        glVertex2f(0.63f, -0.625f);
        glVertex2f(0.63f, -0.617f);
        glVertex2f(0.57f, -0.617f);
    glEnd();

    // corner Triangle
    glBegin(GL_TRIANGLES);
        glColor3f(0.25f, 0.45f, 0.75f);
        glVertex2f(0.74f, -0.66f);
        glVertex2f(0.74f, -0.60f);
        glVertex2f(0.68f, -0.66f);
    glEnd();

}

void Uninitialize(void)
{
    // code
    // if existing in fullscreen first restore and then procced
    if (Texture_Setup) // change no.8
    {
        glDeleteTextures(1, &Texture_Setup);
        Texture_Setup = 0;
    }
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
