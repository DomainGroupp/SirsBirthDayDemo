
#include<Windows.h>
#include<stdio.h>   
#include<stdlib.h>  
#include "OGL.h"
#include "math.h"

#include<gl/GL.h> 
#include<gl/GLU.h> 

#pragma comment(lib, "opengl32.lib")  
#pragma comment(lib, "GLU32.LIB")


//Macros

#define WINWIDTH 800
#define WINHEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;


HWND ghwnd = NULL;

HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL bFullScreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

FILE* gpFile = NULL;


int length = 1;
float pointx;

GLUquadric *quadric = NULL;

GLfloat pushSaturntofront = -15.0f;
GLfloat pushNametoBack    = 18.0f;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function Declaration
    int initialize(void);
    void Render(void);
    void Update(void);
    void uninitialize(void);



    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_OSJ");
    BOOL bDone = FALSE;


    //code

    //create log file

    gpFile = fopen("Log.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Lg file creation Failed!!"), TEXT("ERROR"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "OMJ: Program Strated Sucessfully!!!!\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // 2. Register above wndclass
    RegisterClassEx(&wndclass);

    //Centering
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeigth = GetSystemMetrics(SM_CYSCREEN);



    // 3. Create the window 
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,

        szAppName,
        TEXT("My First RTR Program : Omkar Satish Joshi"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        ((screenWidth / 2) - (WINWIDTH / 2)),
        ((screenHeigth / 2) - (WINHEIGHT / 2)),
        WINWIDTH,
        WINHEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // set global window handle
    ghwnd = hwnd;

    int iResult = initialize();
    if (iResult != 0)
    {
        fprintf(gpFile, "OMJ: WinMain: Initialize function failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "OMJ: WinMain: Initialize function Succsseded\nn");
    }

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // Game Loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
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
                Render();
                //Update
                Update();

            }
        }
    }

    //uninitialize
    uninitialize();

    return((int)msg.wParam);

}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    // code

    // Function Declarations
    void ToggleFullscreen(void);
    void Resize(int, int);
    void uninitialize(void);

    switch (iMsg) {
    case WM_CREATE:

        memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
        wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;

    case WM_SETFOCUS:
        bActiveWindow = TRUE;
        break;

    case WM_KILLFOCUS:
        bActiveWindow = FALSE;
        break;

    case WM_SIZE:

        Resize(LOWORD(lParam), HIWORD(lParam));

        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
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
            if (bFullScreen == FALSE)
            {
                ToggleFullscreen();
                bFullScreen = TRUE;
            }
            else
            {
                ToggleFullscreen();
                bFullScreen = FALSE;
            }
            break;
        case 'a':
            pushSaturntofront = pushSaturntofront + 0.002f;
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:

        uninitialize();
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:
        break;
    }
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen()
{
    // variable declarations
    MONITORINFO mi;
    // code

    if (bFullScreen == FALSE)// st 2
    {
        // step 3
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                //step 7
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED
                );
            }

        }
        ShowCursor(TRUE);
    }

    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
        );


        ShowCursor(TRUE);


    }
}

int initialize()
{
    // Function Declarations
    void  Resize(int, int);

    // variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    //code
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);
    if (ghdc == NULL)
    {
        return(-1);
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        return(-2);
    }

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() Failed\n");
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
    // Enable Depth
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    quadric = gluNewQuadric();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Warmup Resize 
    Resize(WINWIDTH, WINHEIGHT);


    return(0);

}

void Resize(int width, int height)
{
    //code
    if (height <= 0)
            height = 1;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, (GLsizei)width, (GLsizei)height);


        gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}


void DrawCircle(float radius)
{
     
        float Xi, Yi, xValue, i;
        float widthofband = radius +0.2f;
        xValue = 0.0f;
        i = xValue + radius;
        while( i >= (xValue-radius))
        {
            Xi = i;
            
            Yi = sqrtf((radius*radius)-(i*i));
            float widthYi = sqrtf((widthofband*widthofband)-(i*i));
            float xiNext = Xi - 0.001f;
            float YiNext = sqrtf((radius*radius)-((i-0.001f)*(i-0.001f)));
            
            
            glBegin(GL_LINE_STRIP);
            	
                glVertex3f(Xi,Yi,-0.0f);
               
               //glVertex3f(Xi,widthYi,-0.0f);
                glVertex3f(xiNext,YiNext,-0.0f);
            glEnd();

            glBegin(GL_LINES);
            	
            	glVertex3f(Xi,-Yi,-0.0f);
                
           // glVertex3f(Xi ,-widthYi,-0.0f);

                glVertex3f(xiNext,-YiNext,-0.0f);
            glEnd();

            i = i -  0.001f;
        }
}


void Saturn()
{  
    float circleradius = 0.9;
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -4.0f);
    glRotatef(90.f, 1.0f, 0.0f,0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glColor3f(0.92f, 0.84f, 0.67f);
    glColor3f(0.85f, 0.87f, 0.89f );
    gluSphere(quadric, 0.75f, 30, 30);

    glPushMatrix();
    glRotatef(-60.0f,0.0f,0.0f,1.0f);
    glRotatef(40.0f,1.0f,0.0f,0.0f);
   
    for(int i = 0; i < 18; i ++)
    {
        //glColor3f(0.78f, 0.66f, 0.47f);
        glColor3f(0.59f, 0.61f, 0.64f);
        glLineWidth(3.0f);
        if(i%3 == 0)
        {
            //glColor3f(0.65f, 0.58f, 0.50f);
            glColor3f(0.33f, 0.36f, 0.39f);
            glLineWidth(5.0f);
        }
        DrawCircle(circleradius);
        circleradius = circleradius + 0.02;
        glLineWidth(3.0f);
    }
    glPopMatrix();

    glPopMatrix();

    
}

void Presents()
{
       //glColor3f(0.1f, 0.63f, 0.98f);
       glColor3f(1.0f, 0.82f, 0.40f);  

    // P
    glLineWidth(6.0f);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.85f, -0.20f, -3.0f);
    glVertex3f(-0.85f, 0.20f, -3.0f);
    glVertex3f(-0.70f, 0.20f, -3.0f);
    glVertex3f(-0.65f, 0.15f, -3.0f);
    glVertex3f(-0.65f, 0.05f, -3.0f);
    glVertex3f(-0.70f, 0.00f, -3.0f);
    glVertex3f(-0.85f, 0.00f, -3.0f);
    glEnd();

   

    // R
    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.60f, -0.20f, -3.0f);
    glVertex3f(-0.60f, 0.20f, -3.0f);
    glVertex3f(-0.45f, 0.20f, -3.0f);
    glVertex3f(-0.40f, 0.15f, -3.0f);
    glVertex3f(-0.40f, 0.05f, -3.0f);
    glVertex3f(-0.45f, 0.00f, -3.0f);
    glVertex3f(-0.60f, 0.00f, -3.0f);
    glEnd();

    // R diagonal leg
    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.50f, 0.00f, -3.0f);
    glVertex3f(-0.40f, -0.20f, -3.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.33f, -0.20f, -3.0f);
    glVertex3f(-0.33f, 0.20f, -3.0f);
    glVertex3f(-0.16f, 0.20f, -3.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.33f, 0.00f, -3.0f);
    glVertex3f(-0.18f, 0.00f, -3.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(-0.32f, -0.20f, -3.0f);
    glVertex3f(-0.16f, -0.20f, -3.0f);
    glEnd();


    // S
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.08f, 0.20f, -3.0f);
    glVertex3f(-0.07f, 0.20f, -3.0f);
    glVertex3f(-0.12f, 0.15f, -3.0f);
    glVertex3f(-0.12f, 0.08f, -3.0f);
    glVertex3f(-0.07f, 0.02f, -3.0f);
    glVertex3f(0.03f, 0.02f, -3.0f);
    glVertex3f(0.08f, -0.04f, -3.0f);
    glVertex3f(0.08f, -0.15f, -3.0f);
    glVertex3f(0.03f, -0.20f, -3.0f);
    glVertex3f(-0.12f, -0.20f, -3.0f);
    glEnd();

    // E
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.18f, -0.20f, -3.0f);
    glVertex3f(0.18f, 0.20f, -3.0f);
    glVertex3f(0.35f, 0.20f, -3.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.18f, 0.00f, -3.0f);
    glVertex3f(0.33f, 0.00f, -3.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.18f, -0.20f, -3.0f);
    glVertex3f(0.35f, -0.20f, -3.0f);
    glEnd();

    // N
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.43f, -0.20f, -3.0f);
    glVertex3f(0.43f, 0.20f, -3.0f);
    glVertex3f(0.60f, -0.20f, -3.0f);
    glVertex3f(0.60f, 0.20f, -3.0f);
    glEnd();

    // T
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.66f, 0.20f, -3.0f);
    glVertex3f(0.84f, 0.20f, -3.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.75f, 0.20f, -3.0f);
    glVertex3f(0.75f, -0.20f, -3.0f);
    glEnd();

    // S
    glBegin(GL_LINE_STRIP);
    glVertex3f(1.05f, 0.20f, -3.0f);
    glVertex3f(0.90f, 0.20f, -3.0f);
    glVertex3f(0.85f, 0.15f, -3.0f);
    glVertex3f(0.85f, 0.08f, -3.0f);
    glVertex3f(0.90f, 0.02f, -3.0f);
    glVertex3f(1.00f, 0.02f, -3.0f);
    glVertex3f(1.05f, -0.04f, -3.0f);
    glVertex3f(1.05f, -0.15f, -3.0f);
    glVertex3f(1.00f, -0.20f, -3.0f);
    glVertex3f(0.85f, -0.20f, -3.0f);
    glEnd();

}


void GrpName(void)
{
    glColor3f(1.0f, 0.82f, 0.40f);  
   // glColor3f(0.1f, 0.63f, 0.98f);
    // D
    glBegin(GL_QUADS);

    // Left vertical stroke
    glVertex3f(-0.600f, -0.080f, -3.0f);
    glVertex3f(-0.570f, -0.080f, -3.0f);
    glVertex3f(-0.570f, 0.220f, -3.0f);
    glVertex3f(-0.600f, 0.220f, -3.0f);

    //Top stroke
    glVertex3f(-0.600f, 0.190f, -3.0f);
    glVertex3f(-0.470f, 0.190f, -3.0f);
    glVertex3f(-0.470f, 0.220f, -3.0f);
    glVertex3f(-0.600f, 0.220f, -3.0f);

    // Upper-right slant
    glVertex3f(-0.470f, 0.220f, -3.0f);
    glVertex3f(-0.470f, 0.190f, -3.0f);
    glVertex3f(-0.440f, 0.150f, -3.0f);
    glVertex3f(-0.410f, 0.150f, -3.0f);

    // Right vertical stroke
    glVertex3f(-0.440f, -0.010f, -3.0f);
    glVertex3f(-0.410f, -0.010f, -3.0f);
    glVertex3f(-0.410f, 0.150f, -3.0f);
    glVertex3f(-0.440f, 0.150f, -3.0f);

    // Lower-right slant
    glVertex3f(-0.410f, -0.010f, -3.0f);
    glVertex3f(-0.440f, -0.010f, -3.0f);
    glVertex3f(-0.470f, -0.050f, -3.0f);
    glVertex3f(-0.470f, -0.080f, -3.0f);

    // Bottom stroke
    glVertex3f(-0.600f, -0.080f, -3.0f);
    glVertex3f(-0.470f, -0.080f, -3.0f);
    glVertex3f(-0.470f, -0.050f, -3.0f);
    glVertex3f(-0.600f, -0.050f, -3.0f);

    glEnd();


    // O
    glBegin(GL_QUADS);
    // Left vertical stroke
    glVertex3f(-0.340f, -0.080f, -3.0f);
    glVertex3f(-0.310f, -0.080f, -3.0f);
    glVertex3f(-0.310f, 0.220f, -3.0f);
    glVertex3f(-0.340f, 0.220f, -3.0f);

    // Top stroke
    glVertex3f(-0.310f, 0.190f, -3.0f);
    glVertex3f(-0.160f, 0.190f, -3.0f);
    glVertex3f(-0.160f, 0.220f, -3.0f);
    glVertex3f(-0.310f, 0.220f, -3.0f);

    // Right vertical stroke - 15 diff
    glVertex3f(-0.190f, -0.080f, -3.0f);
    glVertex3f(-0.160f, -0.080f, -3.0f);
    glVertex3f(-0.160f, 0.220f, -3.0f);
    glVertex3f(-0.190f, 0.220f, -3.0f);

    //// Lower-right slant
    //glVertex3f(-0.120f, -0.050f, -3.0f);
    //glVertex3f(-0.090f, -0.020f, -3.0f);
    //glVertex3f(-0.090f, -0.050f, -3.0f);
    //glVertex3f(-0.120f, -0.080f, -3.0f);

    // Bottom stroke
    glVertex3f(-0.310f, -0.080f, -3.0f);
    glVertex3f(-0.160f, -0.080f, -3.0f);
    glVertex3f(-0.160f, -0.050f, -3.0f);
    glVertex3f(-0.310f, -0.050f, -3.0f);

    glEnd();


    // M
    glBegin(GL_QUADS);

    // Left
    glVertex3f(-0.080f, -0.080f, -3.0f);
    glVertex3f(-0.050f, -0.080f, -3.0f);
    glVertex3f(-0.050f, 0.220f, -3.0f);
    glVertex3f(-0.080f, 0.220f, -3.0f);

    // Middle diagonals
    glVertex3f(-0.080f, 0.220f, -3.0f);
    glVertex3f(-0.050f, 0.220f, -3.0f);
    glVertex3f(0.060f, 0.030f, -3.0f);
    glVertex3f(0.030f, 0.030f, -3.0f);

    glVertex3f(0.030f, 0.030f, -3.0f);
    glVertex3f(0.060f, 0.030f, -3.0f);
    glVertex3f(0.140f, 0.220f, -3.0f);
    glVertex3f(0.110f, 0.220f, -3.0f);

    // Right
    glVertex3f(0.110f, -0.080f, -3.0f);
    glVertex3f(0.140f, -0.080f, -3.0f);
    glVertex3f(0.140f, 0.220f, -3.0f);
    glVertex3f(0.110f, 0.220f, -3.0f);

    glEnd();


    // A
    glBegin(GL_QUADS);

    // Left diagonal
    glVertex3f(0.190f, -0.080f, -3.0f);
    glVertex3f(0.220f, -0.080f, -3.0f);
    glVertex3f(0.310f, 0.220f, -3.0f);
    glVertex3f(0.280f, 0.220f, -3.0f);

    // Right diagonal
    glVertex3f(0.280f, 0.220f, -3.0f);
    glVertex3f(0.310f, 0.220f, -3.0f);
    glVertex3f(0.400f, -0.080f, -3.0f);
    glVertex3f(0.370f, -0.080f, -3.0f);

    // Crossbar
    glVertex3f(0.240f, 0.030f, -3.0f);
    glVertex3f(0.350f, 0.030f, -3.0f);
    glVertex3f(0.350f, 0.060f, -3.0f);
    glVertex3f(0.240f, 0.060f, -3.0f);

    glEnd();


    // I
    glBegin(GL_QUADS);

    // Top
    glVertex3f(0.460f, 0.190f, -3.0f);
    glVertex3f(0.600f, 0.190f, -3.0f);
    glVertex3f(0.600f, 0.220f, -3.0f);
    glVertex3f(0.460f, 0.220f, -3.0f);

    // Vertical
    glVertex3f(0.515f, -0.050f, -3.0f);
    glVertex3f(0.545f, -0.050f, -3.0f);
    glVertex3f(0.545f, 0.190f, -3.0f);
    glVertex3f(0.515f, 0.190f, -3.0f);

    // Bottom
    glVertex3f(0.460f, -0.080f, -3.0f);
    glVertex3f(0.600f, -0.080f, -3.0f);
    glVertex3f(0.600f, -0.050f, -3.0f);
    glVertex3f(0.460f, -0.050f, -3.0f);

    glEnd();


    // N
    glBegin(GL_QUADS);

    // Left
    glVertex3f(0.670f, -0.080f, -3.0f);
    glVertex3f(0.700f, -0.080f, -3.0f);
    glVertex3f(0.700f, 0.220f, -3.0f);
    glVertex3f(0.670f, 0.220f, -3.0f);

    // Diagonal
    glVertex3f(0.670f, 0.220f, -3.0f);
    glVertex3f(0.700f, 0.220f, -3.0f);
    glVertex3f(0.870f, -0.080f, -3.0f);
    glVertex3f(0.840f, -0.080f, -3.0f);

    // Right
    glVertex3f(0.840f, -0.080f, -3.0f);
    glVertex3f(0.870f, -0.080f, -3.0f);
    glVertex3f(0.870f, 0.220f, -3.0f);
    glVertex3f(0.840f, 0.220f, -3.0f);

    glEnd();

    glPushMatrix();
    glTranslatef(-0.1f, 0.0f, 0.0f);
    // G
    glBegin(GL_QUADS);

    // Left
    glVertex3f(-0.380f, -0.580f, -3.0f);
    glVertex3f(-0.350f, -0.580f, -3.0f);
    glVertex3f(-0.350f, -0.280f, -3.0f);
    glVertex3f(-0.380f, -0.280f, -3.0f);

    // Top
    glVertex3f(-0.380f, -0.310f, -3.0f);
    glVertex3f(-0.200f, -0.310f, -3.0f);
    glVertex3f(-0.200f, -0.280f, -3.0f);
    glVertex3f(-0.380f, -0.280f, -3.0f);

    // Bottom
    glVertex3f(-0.380f, -0.580f, -3.0f);
    glVertex3f(-0.200f, -0.580f, -3.0f);
    glVertex3f(-0.200f, -0.550f, -3.0f);
    glVertex3f(-0.380f, -0.550f, -3.0f);

    // Right lower stroke
    glVertex3f(-0.230f, -0.580f, -3.0f);
    glVertex3f(-0.200f, -0.580f, -3.0f);
    glVertex3f(-0.200f, -0.430f, -3.0f);
    glVertex3f(-0.230f, -0.430f, -3.0f);

    // G crossbar
    glVertex3f(-0.290f, -0.450f, -3.0f);
    glVertex3f(-0.200f, -0.450f, -3.0f);
    glVertex3f(-0.200f, -0.420f, -3.0f);
    glVertex3f(-0.290f, -0.420f, -3.0f);

    glEnd();


    // R
    glBegin(GL_QUADS);

    // Left
    glVertex3f(-0.120f, -0.580f, -3.0f);
    glVertex3f(-0.090f, -0.580f, -3.0f);
    glVertex3f(-0.090f, -0.280f, -3.0f);
    glVertex3f(-0.120f, -0.280f, -3.0f);

    // Top
    glVertex3f(-0.120f, -0.310f, -3.0f);
    glVertex3f(0.060f, -0.310f, -3.0f);
    glVertex3f(0.060f, -0.280f, -3.0f);
    glVertex3f(-0.120f, -0.280f, -3.0f);

    // Right upper
    glVertex3f(0.030f, -0.430f, -3.0f);
    glVertex3f(0.060f, -0.430f, -3.0f);
    glVertex3f(0.060f, -0.310f, -3.0f);
    glVertex3f(0.030f, -0.310f, -3.0f);

    // Middle
    glVertex3f(-0.120f, -0.460f, -3.0f);
    glVertex3f(0.060f, -0.460f, -3.0f);
    glVertex3f(0.060f, -0.430f, -3.0f);
    glVertex3f(-0.120f, -0.430f, -3.0f);

    // Leg
    glVertex3f(-0.010f, -0.460f, -3.0f);
    glVertex3f(0.020f, -0.460f, -3.0f);
    glVertex3f(0.090f, -0.580f, -3.0f);
    glVertex3f(0.060f, -0.580f, -3.0f);

    glEnd();


    // O
    glBegin(GL_QUADS);

    // Left
    glVertex3f(0.160f, -0.580f, -3.0f);
    glVertex3f(0.190f, -0.580f, -3.0f);
    glVertex3f(0.190f, -0.280f, -3.0f);
    glVertex3f(0.160f, -0.280f, -3.0f);

    // Top
    glVertex3f(0.160f, -0.310f, -3.0f);
    glVertex3f(0.340f, -0.310f, -3.0f);
    glVertex3f(0.340f, -0.280f, -3.0f);
    glVertex3f(0.160f, -0.280f, -3.0f);

    // Right
    glVertex3f(0.310f, -0.580f, -3.0f);
    glVertex3f(0.340f, -0.580f, -3.0f);
    glVertex3f(0.340f, -0.280f, -3.0f);
    glVertex3f(0.310f, -0.280f, -3.0f);

    // Bottom
    glVertex3f(0.160f, -0.580f, -3.0f);
    glVertex3f(0.340f, -0.580f, -3.0f);
    glVertex3f(0.340f, -0.550f, -3.0f);
    glVertex3f(0.160f, -0.550f, -3.0f);

    glEnd();


    // U
    glBegin(GL_QUADS);

    // Left
    glVertex3f(0.430f, -0.580f, -3.0f);
    glVertex3f(0.460f, -0.580f, -3.0f);
    glVertex3f(0.460f, -0.280f, -3.0f);
    glVertex3f(0.430f, -0.280f, -3.0f);

    // Right
    glVertex3f(0.580f, -0.580f, -3.0f);
    glVertex3f(0.610f, -0.580f, -3.0f);
    glVertex3f(0.610f, -0.280f, -3.0f);
    glVertex3f(0.580f, -0.280f, -3.0f);

    // Bottom
    glVertex3f(0.430f, -0.580f, -3.0f);
    glVertex3f(0.610f, -0.580f, -3.0f);
    glVertex3f(0.610f, -0.550f, -3.0f);
    glVertex3f(0.430f, -0.550f, -3.0f);

    glEnd();


    // P
    glBegin(GL_QUADS);

    // Left
    glVertex3f(0.700f, -0.580f, -3.0f);
    glVertex3f(0.730f, -0.580f, -3.0f);
    glVertex3f(0.730f, -0.280f, -3.0f);
    glVertex3f(0.700f, -0.280f, -3.0f);

    // Top
    glVertex3f(0.700f, -0.310f, -3.0f);
    glVertex3f(0.880f, -0.310f, -3.0f);
    glVertex3f(0.880f, -0.280f, -3.0f);
    glVertex3f(0.700f, -0.280f, -3.0f);

    // Right upper
    glVertex3f(0.850f, -0.430f, -3.0f);
    glVertex3f(0.880f, -0.430f, -3.0f);
    glVertex3f(0.880f, -0.310f, -3.0f);
    glVertex3f(0.850f, -0.310f, -3.0f);

    // Middle
    glVertex3f(0.700f, -0.460f, -3.0f);
    glVertex3f(0.880f, -0.460f, -3.0f);
    glVertex3f(0.880f, -0.430f, -3.0f);
    glVertex3f(0.700f, -0.430f, -3.0f);

    glEnd();

    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.05f, -0.8f, 0.0f);
    glScalef(0.75f,0.50f,1.0f);
    Presents();
    glPopMatrix();

   
}

void stundent()
{
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);

    for (float angle = 0.0f; angle < 360.0f; angle += 0.6f)
    {
        float rad = angle * 3.14159f / 180.0f;

        //float x = 0.06f * cos(rad);
        //float y = 0.06f * sin(rad);
        float x = 0.05f * cos(rad);
        float y = 0.05f * sin(rad);

        glVertex3f(x, y, -3.0f);
        //fprintf(gpFile,"%lf \t %lf", x, y);
    }
    glEnd();
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.1f, 0.0f, -3.0f);
    glVertex3f(-0.1f, 0.0f, -3.0f);
    glVertex3f(-0.2f, 0.5f, -3.0f);
    glVertex3f(-0.2f, 0.5f, -3.0f);
    glEnd();
}

void classroom(void)
{
    //TopWall
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, 5.0f, -3.0f);
    glVertex3f(-5.0f, 0.0f, -3.0f);
    glVertex3f(5.0f, 0.0f, -3.0f);
    glVertex3f(5.0f, 5.0f, -3.0f);
    glEnd();

    //RightWall
    glColor3f(0.78f, 0.75f, 0.66f);
    glBegin(GL_QUADS);
    glVertex3f(4.0f, 2.0f, -3.0f);
    glVertex3f(-0.10f, 1.0f, -3.0f);
    glVertex3f(-0.10f, -0.0f, -3.0f);
    glVertex3f(4.0f, -1.0f, -3.0f);
    glEnd();

    // Leftwall 
    glColor3f(0.88f, 0.84f, 0.72f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, 2.0f, -3.0f);
    glVertex3f(-0.10f, 1.0f, -3.0f);
    glVertex3f(-0.10f, -0.0f, -3.0f);
    glVertex3f(-5.0f, -1.0f, -3.0f);
    glEnd();

    //BlackBoard
    


}

void Render(void)
{
    //code

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(0.0f, 1.0f, pushSaturntofront);
    Saturn();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.2f, -0.3f, pushNametoBack);
    GrpName();
    glPopMatrix();
    
   
   // classroom();


    //glTranslatef(0.5f, 0.0f, 0.0f);
    //stundent();
    SwapBuffers(ghdc);
    

}

void Update(void)
{
    if(pushSaturntofront < -3.8)
        pushSaturntofront = pushSaturntofront + 0.008f;

    if(pushNametoBack > -1.0)
        pushNametoBack = pushNametoBack - 0.008f;

}

void uninitialize(void)
{
    //code

    if (bFullScreen == TRUE)
    {
        ToggleFullscreen();
        bFullScreen = FALSE;
    }
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);

    }
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }
    if(quadric)
    {
        gluDeleteQuadric(quadric);
        quadric = NULL;
    }
    if (gpFile)
    {
        fprintf(gpFile, "OMJ: Program Terminated succesfully!!!\n\n");
        fclose(gpFile);
        gpFile = NULL;
    }

}

