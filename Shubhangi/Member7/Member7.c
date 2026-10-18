// Header files
#include <Windows.h>
#include <stdlib.h> // for exit
#include <stdio.h>  // For file IO
#include <string.h> // for strlen

// opengl related header files
#include <GL/GL.h>
#include <GL/GLU.h>

// link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")

// globale variable declaration
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL bFullScreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
FILE *gpFile = NULL;

BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;

#define NUMBER_OF_POINTS 1000
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

struct points
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

// Scene 1 
struct points points[NUMBER_OF_POINTS];
static int nextPoint;
float sweepXLeft = -1.0f;
float sweepXRight = 1.0f;
float sweepDelta = 0.0006f; //for golden sweep 
int delay = 0;
static int slowSpeed = 0;

// Scene 2 / 3
GLuint fontBase = 0;
float fadeAlpha = 0.0f;

// Scene 2 
float scene2SweepX = -1.0f;
int scene2ConnectCount = 0;
int scene2SlowSpeed = 0;

// Scene 2 ECG
int ecgLength = 1;
float ecgPointX = -3.5f;

//state machine 
enum stateMachine
{
    GreenPoints,    // Scene 1: green points (default)
    GoldenPoints,   // Scene 1: gold sweep Left -> Right 
    LineConnect,    // Scene 1: connect points
    BlankScreen,    // empty black between Scene 1 and 2
    Scene2Green,    // Scene 2: upper green points + birthday + ECG
    Scene2Gold,     // Scene 2: upper green->gold sweep
    Scene2Connect,  // Scene 2: connect gold points
    Scene2Hold,     
    Scene3          
} state = GreenPoints;

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // func declarations
    int initialize(void);
    void unintialize(void);
    void render(void);
    void update(void);

    // create log file
    gpFile = fopen("Log.txt", "w");

    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log file creation failed"), TEXT("ERROR"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program started successfully\n");
    }

    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_SSH");
    BOOL bDone = FALSE;

    // WNDCLASSEX initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Register above wndClass
    RegisterClassEx(&wndclass);

    // centering
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    // create the window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          szAppName,
                          TEXT("RTR7-027:Shubhangi Pardeshi"),
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                          ScreenWidth / 2 - WIN_WIDTH / 2,
                          ScreenHeight / 2 - WIN_HEIGHT / 2,
                          WIN_WIDTH,
                          WIN_HEIGHT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    // set global window handle
    ghwnd = hwnd;

    int iResult = initialize();
    if (iResult != 0)
    {
        fprintf(gpFile, "SSH: WinMain: initialize() failed");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "SSH: WinMain: initialize() successed");
    }

    // Show Window
    ShowWindow(hwnd, iCmdShow);

    // Update the window to paint its background
    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // Game loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
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
                {
                    bDone = TRUE;
                }
                // render
                render();
                // update
                update();
            }
        }
    }

    // unintialize
    unintialize();

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // func declarations
    void toggleFullScreen(void);
    void resize(int, int);
    void unintialize(void);

    // code
    switch (iMsg)
    {
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
        resize(LOWORD(lParam), HIWORD(lParam));
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
                toggleFullScreen();
                bFullScreen = TRUE;
            }
            else
            {
                toggleFullScreen();
                bFullScreen = FALSE;
            }
            break;
        case 'S':
        case 's':
            state = BlankScreen; //from scene 1 to scene 2
            delay = 89; 
            break;
        case 'B':
        case 'b':
            state = Scene3; //from scene 2 to scene 3
            fadeAlpha = 0.0f;
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        unintialize();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    // variable decln
    MONITORINFO mi;

    // code
    if (bFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
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
    // Func declarations
    void resize(int, int);

    // variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);
    if (ghdc == NULL)
    {
        return -1;
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        return -2;
    }

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return -3;
    }

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        return -4;
    }

    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -5;
    }

    // Enable depth
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // choose screen clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Network point genration 
    srand(54);
    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        points[i].x = -1.0f + (rand() % 1000) / 500.0f;
        points[i].y = -1.0f + (rand() % 1000) / 500.0f;
        points[i].z = 0.0f;
    }
    
    HFONT hFont;
    HFONT hOldFont;

    fontBase = glGenLists(96);
    hFont = CreateFont(100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                       ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
                       TEXT("Arial"));
    hOldFont = (HFONT)SelectObject(ghdc, hFont);
    wglUseFontBitmaps(ghdc, 32, 96, fontBase);
    SelectObject(ghdc, hOldFont);
    DeleteObject(hFont);

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    // code
    if (height <= 0)
        height = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

//From Omkar 
void YForCurrentxofECG(float x, float *y)
{
    if (ecgPointX >= x && ecgPointX <= x + 0.6f)
    {
        if (ecgPointX > x && ecgPointX < x + 0.01f)
        {
            *y = 0.0f;
        }
        else if (ecgPointX >= x + 0.2f && ecgPointX <= x + 0.6f)
        {
            if (ecgPointX >= x + 0.2f && ecgPointX < x + 0.32f)
            {
                if (ecgPointX <= x + 0.26f)
                    *y = *y + 0.0003f;
                else
                    *y = *y - 0.00065f;
            }
            else if (ecgPointX >= x + 0.32f && ecgPointX < x + 0.4f)
            {
                if (ecgPointX <= x + 0.36f)
                    *y = *y + 0.004f;
                else
                    *y = *y - 0.005f;
            }
            else if (ecgPointX > x + 0.4f && ecgPointX <= x + 0.45f)
            {
                *y = 0.0f;
            }
            else if (ecgPointX > x + 0.45f && ecgPointX < x + 0.6f)
            {
                if (ecgPointX <= x + 0.53f)
                    *y = *y + 0.0003f;
                else
                {
                    *y = *y - 0.0005f;
                    if (*y < 0.0f)
                        *y = 0.0f;
                }
            }
        }
    }
}

void drawECG(void)
{
    float z = -4.0f;
    float y = 0.0f;
    int i;
    int maxLen = (int)((3.5f * 2.0f) / 0.0002f);

    if (ecgLength < 2)
        ecgLength = 2;
    if (ecgLength > maxLen)
        ecgLength = maxLen;

    ecgPointX = -3.5f;
    y = 0.0f;

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 1.0f, 0.0f);
    for (i = 0; i < ecgLength; i++)
    {
        YForCurrentxofECG(-2.9f, &y);
        YForCurrentxofECG(-1.9f, &y);
        YForCurrentxofECG(-0.9f, &y);
        YForCurrentxofECG(0.9f, &y);
        YForCurrentxofECG(-1.9f, &y);
        YForCurrentxofECG(-2.9f, &y);

        glVertex3f(ecgPointX, y, z);
        ecgPointX = ecgPointX + 0.0002f;
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

//secene 2: 
void drawUpperNetwork(void)
{
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < (NUMBER_OF_POINTS/4); i++)
    {
        if (points[i].x <= scene2SweepX)
            glColor3f(0.85f, 0.70f, 0.20f); // gold
        else
            glColor3f(0.0f, 0.55f, 0.0f); // green

        glVertex3f(points[i].x, points[i].y, 0.0f);
    }
    glEnd();

    // Connections after gold sweep
    if (state == Scene2Connect || state == Scene2Hold || state == Scene3)
    {
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glColor3f(0.85f, 0.70f, 0.20f);
        for (int i = 0; i < scene2ConnectCount && i < (NUMBER_OF_POINTS/4) - 1; i++)
        {
            if (points[i].x <= scene2SweepX && points[i + 1].x <= scene2SweepX)
            {
                glVertex3f(points[i].x, points[i].y, 0.0f);
                glVertex3f(points[i + 1].x, points[i + 1].y, 0.0f);
            }
        }
        glEnd();
    }
}

void drawHappyBirthday(void)
{
    char text[] = "Happy Birthday!!!";

    glColor3f(0.85f, 0.70f, 0.20f);
    glRasterPos3f(-0.90f, -0.15f, 0.0f);
    glListBase(fontBase - 32);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void drawFadeToBlack(void)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void drawScene2(void)
{
    //1st half 
    glPushMatrix();
    glTranslatef(0.0f, 0.72f, -2.8f);
    glScalef(2.20f, 0.42f, 1.0f); // keep flat in upper third
    drawUpperNetwork();
    glPopMatrix();

    //2nd half 
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, -2.2f);
    drawHappyBirthday();
    glPopMatrix();

    //remaining part of screen 
    glPushMatrix();
    glTranslatef(0.0f, -2.5f, -5.5f);
    glScalef(1.5f, 1.20f, 1.0f);
    drawECG();
    glPopMatrix();
}

BOOL isScene2State(void)
{
    return (state == Scene2Green || state == Scene2Gold ||
            state == Scene2Connect || state == Scene2Hold);
}

void render(void)
{
    void networkPoints(int);
    void connectNetwork(int);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (state == GreenPoints || state == GoldenPoints || state == LineConnect)
    {
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -2.0f);
        glScalef(1.5f, 0.85f, 1.0f);
        networkPoints(NUMBER_OF_POINTS);
        if (state == LineConnect)
            connectNetwork(NUMBER_OF_POINTS);
        glPopMatrix();
    }
    else if (state == BlankScreen)
    {
        //Nthng
    }
    else if (isScene2State() == TRUE || state == Scene3)
    {
        drawScene2();

        //fading ...........
        if (state == Scene3)
            drawFadeToBlack();
    }

    SwapBuffers(ghdc);
}

void update(void)
{
    if (state == GreenPoints)
    {
        delay++;
        if (delay > 300)
        {
            state = GoldenPoints;
            delay = 0;
        }
    }
    else if (state == GoldenPoints)
    {
        if (sweepXLeft < sweepXRight)
            sweepXLeft += sweepDelta;
        else
        {
            state = LineConnect;
            delay = 0;
        }
    }
    else if (state == LineConnect)
    {
        if (nextPoint < (NUMBER_OF_POINTS - 1))
        {
            if (slowSpeed++ >= 70)
            {
                nextPoint = nextPoint + 4;
                slowSpeed = 0;
            }
        }
        else
        {
            state = BlankScreen;
            delay = 0;
        }
    }
    else if (state == BlankScreen)
    {
        delay++;
        if (delay > 120)
        {
            state = Scene2Green;
            delay = 0;
            scene2SweepX = -1.0f; //left 
            scene2ConnectCount = 0;
            scene2SlowSpeed = 0;
            ecgLength = 1;
        }

    }
    else if (state == Scene2Green)
    {
        if (ecgLength <= (int)((3.5f * 2.0f) / 0.0002f))
            ecgLength += 15;

        delay++;
        if (delay > 240)
        {
            state = Scene2Gold;
            delay = 0;
            scene2SweepX = -1.0f;
        }
    }
    else if (state == Scene2Gold)
    {
        if (ecgLength <= (int)((3.5f * 2.0f) / 0.0002f))
            ecgLength += 15;

        if (scene2SweepX < 1.0f)
            scene2SweepX += 0.0009f;
        else
        {
            scene2SweepX = 1.0f;
            state = Scene2Connect;
            scene2ConnectCount = 0;
            scene2SlowSpeed = 0;
            delay = 0;
        }
    }
    else if (state == Scene2Connect)
    {
        if (ecgLength <= (int)((3.5f * 2.0f) / 0.0002f))
            ecgLength += 15;

        if (scene2ConnectCount < NUMBER_OF_POINTS - 1)
        {
            if (scene2SlowSpeed++ >= 20)
            {
                scene2SlowSpeed = 0;
                scene2ConnectCount += 4;
            }
        }
        else
        {
            state = Scene2Hold;
            delay = 0;
        }
    }
    else if (state == Scene2Hold)
    {
        if (ecgLength <= (int)((3.5f * 2.0f) / 0.0002f))
            ecgLength += 15;

        delay++;
        if (delay > 480)
        {
            state = Scene3;
            delay = 0;
            fadeAlpha = 0.0f;
        }
    }
    else if (state == Scene3)
    {
        // slower fade to black
        if (fadeAlpha < 1.0f)
        {
            fadeAlpha += 0.0015f;
            if (fadeAlpha > 1.0f)
                fadeAlpha = 1.0f;
        }
    }
}

void unintialize(void)
{
    // code
    // if exiting in full screen first restore and then proceed
    if (bFullScreen == TRUE)
    {
        toggleFullScreen();
        bFullScreen = FALSE;
    }

    // first check the current context and if it is then unmake it
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // now destroy
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // release device context
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // destroy window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Close log file
    if (gpFile)
    {
        fprintf(gpFile, "Program termionated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

void networkPoints(int networkSize)
{
    //stars
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.5f, 0.0f); //green 
    for (int i = 0; i < (networkSize-1); i++)
    {
        if (points[i].x <= sweepXLeft)
            glColor3f(0.85f, 0.70f, 0.20f);  // gold
        else
            glColor3f(0.0f, 0.5f, 0.0f);     // green

        glVertex3f(points[i].x, points[i].y, points[i].z);
    }
    glEnd();
}

void connectNetwork(int networkSize)
{
    glBegin(GL_LINES);
    glColor3f(0.85f, 0.70f, 0.20f); // gold
    for (int i = 0; i < nextPoint && i < networkSize - 1; i++)
    {
        glVertex3f(points[i].x, points[i].y, points[i].z);
        glVertex3f(points[i + 1].x, points[i + 1].y, points[i + 1].z);
    }
    glEnd();
}
