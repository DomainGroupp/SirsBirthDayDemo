#include <windows.h>
#include <stdio.h> //for file i/o
#include <stdlib.h> //for exit()
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

//macros 
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Globle function call back 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Global variable 
HWND ghWnd = NULL;
HDC ghDC = NULL;
HGLRC ghRC = NULL; //opengl chya graphic library chya rendering context cha handle 

BOOL bFullScreen = FALSE;
DWORD dwStyle; 
WINDOWPLACEMENT wpPrev; 
FILE* gpFile = NULL; 

BOOL bActiveWindow = FALSE; 
BOOL bEscapKeyIsPressed = FALSE; 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdLine, int iCmdShow) 
{
    //Declarations - stub function 
    int  initialize(void); //init
    void uninitialize(void); //un-init
    void render(void); //display/draw
    void update(void); //for animation 

    //Var 
    WNDCLASSEX wndclass; 
    HWND hwnd = NULL; 
    MSG msg; 
    TCHAR szAppName[] = TEXT("RTR7-027");
    BOOL bDone = FALSE; //for game loop

    //Create log file
    gpFile = fopen("MyLogFile.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log file failed to create"), TEXT("ERROR"), MB_ICONERROR | MB_OK);
        exit(-1);
    }
    else
    {
        fprintf(gpFile, "SP-WinMain: Program started successfully!!!\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX); 
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
    wndclass.cbWndExtra = 0; 
    wndclass.lpfnWndProc = WndProc; 
    wndclass.hInstance = hInstance; 
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //gdi32.dll
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION); 
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName; 
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wndclass); 

    //Centering
    int desktop_width = GetSystemMetrics(SM_CXSCREEN); 
    int desktop_height = GetSystemMetrics(SM_CYSCREEN); 

    hwnd = CreateWindowEx(WS_EX_APPWINDOW, 
                        szAppName, 
                        TEXT("Triangle_OpenGL - Shubhangi Pardeshi"),
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, 
                        ((desktop_width/2) - (WIN_WIDTH/2)), //x
                        ((desktop_height/2) - (WIN_HEIGHT/2)), //y
                        WIN_WIDTH, //width
                        WIN_HEIGHT, //height 
                        NULL,
                        NULL, 
                        hInstance, 
                        NULL); 

    //set global window handle 
    ghWnd = hwnd; 

    int iResult = initialize();
    if (iResult != 0)
    {
        fprintf(gpFile, "SP-WinMain: Initialize function failed\n");
        DestroyWindow(hwnd); //mala WM_DESTROY patha
        hwnd = NULL; 
    }
    else
    {
        fprintf(gpFile, "SP-WinMain: Initialize function suceeded\n");
    }

    ShowWindow(hwnd, iCmdShow); 

    UpdateWindow(hwnd); 
  
    SetForegroundWindow(hwnd);//
    SetFocus(hwnd);

    //game loop
    while(bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) //PEAK MESSAGE
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
                if (bEscapKeyIsPressed == TRUE)
                    bDone = TRUE;

                //render and update as window is active 
                render();
                update();
            }
        }
    }

    //uninitialize 
    uninitialize(); 

    return((int)msg.wParam);
}

//os call karte WndProc la 
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    //local function declarations 
    void ToggleFullScreen(void);
    void resize(int, int); // width, height 
    void uninitialize(void); //un-init

    switch(iMsg)
    {
        case WM_CREATE: //window message
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
                case VK_ESCAPE: //virtual key code 
                    //Occurance - #3
                    bEscapKeyIsPressed = TRUE;  
                    break;
                default: 
                    break;
            }
            break;
        case WM_CHAR: 
            //Occurance - #4
            switch (wParam)
            {
                case 'F': 
                case 'f': 
                    //ToggleFullScreen();
                    //bFullScreen = !bFullScreen; 
                    if (bFullScreen == FALSE)
                    {
                        ToggleFullScreen();
                        bFullScreen = TRUE; 
                    }
                    else
                    {
                        ToggleFullScreen();
                        bFullScreen = FALSE; 
                    }
                    break;
                default: 
                    break;
            }
            break;
        case WM_CLOSE: //close, alt+f4, system menu - close 
            //Occurance - #5
            uninitialize(); 
            break; 
        case WM_DESTROY: //close, alt+f4, system menu - close and DestroyWindow() 
            //Occurance - #6
            PostQuitMessage(19860); // zero goes in msg.wparam //WM_QUIT
            break; 
        default: 
            break; 
    }
    
    return(DefWindowProc(hwnd, iMsg, wParam, lParam)); 
}

void ToggleFullScreen(void)
{
    //var 
    MONITORINFO mi; 

    //code 
    if (bFullScreen == FALSE)
    {
        //GetWindowLongPtr
        dwStyle = GetWindowLong(ghWnd, GWL_STYLE); // WINDOW CHa long de means type long(get window long of style)
        //Check if WS_OVERLAPPEDWINDOW in dwStyle
        if (dwStyle & WS_OVERLAPPEDWINDOW) //dwStyle MADHE WS_OVERLAPPED WINDOW ahe ka te check kar
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if (GetWindowPlacement(ghWnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right - mi.rcMonitor.left), (mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER|SWP_FRAMECHANGED);
            }
        }

        ShowCursor(FALSE);
    }
    else //alredy in full screen
    {
        SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghWnd, &wpPrev);
        SetWindowPos(ghWnd, HWND_TOP,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_FRAMECHANGED|SWP_NOZORDER);
        ShowCursor(TRUE);
    }
}

int initialize(void)
{
    //prototype  
    void resize(int, int);

    //var 
    PIXELFORMATDESCRIPTOR pfd; //
    int iPixelFormatIndex; 

    //code 
    //ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); //Windows memset
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1; // 1 is convensional 
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; //Window var draw karayacha ahe | opengl supported pixel format | for fast rendering use 2 buffers - 1 for current render ele and 2 for next rendering ele 
    pfd.iPixelType = PFD_TYPE_RGBA; //ashya pixel cha type de jyat rgb and transperecy pan ahe 
    pfd.cRedBits = 8; //T0tal 32 bits 
    pfd.cBlueBits = 8; 
    pfd.cGreenBits = 8; 
    pfd.cAlphaBits = 8; 
    pfd.cDepthBits = 32; //for depth perception 

    ghDC = GetDC(ghWnd);
    if (ghDC == NULL)
    { 
        return (-1);
    }

    iPixelFormatIndex = ChoosePixelFormat(ghDC, &pfd); // return index of choosen or selected pixel format, it is 1 based 

    if (iPixelFormatIndex == 0)
    {
        return (-2);
    }

    /*   STEP 2. Choose and select that OS given pixel format*/
    if (SetPixelFormat(ghDC, iPixelFormatIndex, &pfd) == FALSE)
    {
        return (-3);
    }

   /* STEP 3. Use bridging API to create rendering context according to the chosen pixel format and device context*/
    ghRC = wglCreateContext(ghDC);
    if (ghRC == NULL)
        return (-4);

   /* STEP 4. Make that rendering context as the current context for further rendeing*/
    if (wglMakeCurrent(ghDC, ghRC) == FALSE)
        return (-5);

    //Enable depth
    //1.depth enable kelyavar z axis yeto ani depth aalyavar barech artifacts yetat (artifact - jedisalya nako te)
    //glShadeModel - shade detana rang smooth thev
    glShadeModel(GL_FLAT);
    //2.glClearDepth - mazya depth buffer madhlya saglya value (mhanjech all bits) 1 kar 
    glClearDepth(1.0f);
    //3.glEnable - DEPTH TEST enble kar on my pixels 
    glEnable(GL_DEPTH_TEST); 
    //4.glDepthFunc - GL_LEQUAL he test enable kar (less than equal)
    //
    glDepthFunc(GL_LEQUAL);
    //5.glHint - KAHITARI HINT DE NAH MALA 
    //GL_PERSPECTIVE_CORRECTION_HINT -> persepctive view sathi correction hint 
    //GL_NICEST - DO A GOOD JOB
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


   /* STEP 5. Start rendering API*/
    //Choose screen clearning colour as blue 
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //My first OpenGL call //Blue colour, alpha
    //hi fakt colou choose karun thevte 
    
    //warmup resize 
    resize(WIN_WIDTH, WIN_HEIGHT); //calls WM_PAINT  

    return (0); 
}

void resize(int iWidth, int iHeight)
{
    //code
    if (iHeight <= 0)
        iHeight = 1; //in future while calculting perspective we need to devide by height hence cannot be zero  

    glMatrixMode(GL_PROJECTION);//Projection matrix load kar memory madhe 
    glLoadIdentity();//MATRIX la unit value de 

    glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight); //Jithun jas dole baghtat te

    gluPerspective(45.0f, (GLfloat)(iWidth)/(GLfloat)iHeight, 0.1f, 100.0f);
    
}

void uninitialize(void)
{
    //if exiting in full screen, first restore then exit 
    
    if (bFullScreen == TRUE)
    {
        ToggleFullScreen();
        bFullScreen = FALSE;
    }
    
    //First check the current context and if it is then make it un-current 
    if (wglGetCurrentContext() == ghRC)
    {
        wglMakeCurrent(NULL, NULL);//uNMAKE KARAN 
    }

    //Now destroy the rendering context 
    if (ghRC)
    {
        wglDeleteContext(ghRC);
        ghRC = NULL; 
    }

    //Release the device context 
    if (ghDC)
        ReleaseDC(ghWnd, ghDC);

    //code
    if (ghWnd)
    {
        DestroyWindow(ghWnd);
        ghWnd = NULL;
    }

    //close log file
    if (gpFile)
    {
        fprintf(gpFile, "SP-WinMain: Program terminated successfully!!!\n");
        fclose(gpFile);
        gpFile = NULL;
    }
        
}

void drawBox(float centerX, float centerY, float centerZ, float sizeX, float sizeY, float sizeZ)
{
    float halfX = sizeX * 0.5f;
    float halfY = sizeY * 0.5f;
    float halfZ = sizeZ * 0.5f;

    glBegin(GL_QUADS);
        // Front
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ + halfZ);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ + halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ + halfZ);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ + halfZ);
        // Back
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ - halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ - halfZ);
        // Left
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ + halfZ);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ + halfZ);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ - halfZ);
        // Right
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ + halfZ);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ - halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ + halfZ);
        // Bottom
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ - halfZ);
        glVertex3f(centerX + halfX, centerY - halfY, centerZ + halfZ);
        glVertex3f(centerX - halfX, centerY - halfY, centerZ + halfZ);
        // Top
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ + halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ + halfZ);
        glVertex3f(centerX + halfX, centerY + halfY, centerZ - halfZ);
        glVertex3f(centerX - halfX, centerY + halfY, centerZ - halfZ);
    glEnd();
}

void drawBase(void)
{
    glColor3f(0.55f, 0.55f, 0.55f);
    drawBox(0.0f, 0.075f, 0.0f, 1.40f, 0.15f, 1.40f);
    drawBox(0.0f, 0.210f, 0.0f, 1.10f, 0.12f, 1.10f);
    drawBox(0.0f, 0.320f, 0.0f, 0.85f, 0.10f, 0.85f);
}

void drawPole(void)
{
    float baseTop = 0.37f;
    float poleTop = baseTop + 3.2f;
    float radius = 0.06f;
    int slices = 20;

    glColor3f(0.39f, 0.25f, 0.09f);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++)
    {
        float angle = (float)(2.0 * M_PI * i / slices);
        float x = radius * (float)cos(angle);
        float z = radius * (float)sin(angle);
        glNormal3f(x / radius, 0.0f, z / radius);
        glVertex3f(x, poleTop, z);
        glVertex3f(x, baseTop, z);
    }
    glEnd();
}

void drawFlag(void)
{
    float flagStartX = 0.06f;
    float flagStartY = 3.57f - 0.05f;
    float flagHeight = 1.0f;
    float flagWidth = 2.0f;
    float bandHeight = flagHeight / 3.0f;
    float flagZ = 0.0f;
    float chakraZ = 0.02f;

    float centerX = flagStartX + flagWidth / 2.0f;
    float centerY = flagStartY - flagHeight / 2.0f;
    float radiusOuter = 0.15f;
    float radiusInner = 0.12f;
    float radiusHub = 0.03f;

    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);

    // Saffron
    glColor3f(1.0f, 0.55f, 0.20f);
    glVertex3f(flagStartX, flagStartY, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY - bandHeight, flagZ);
    glVertex3f(flagStartX, flagStartY - bandHeight, flagZ);

    // White
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(flagStartX, flagStartY - bandHeight, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY - bandHeight, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY - 2.0f * bandHeight, flagZ);
    glVertex3f(flagStartX, flagStartY - 2.0f * bandHeight, flagZ);

    // Green
    glColor3f(0.07f, 0.53f, 0.03f);
    glVertex3f(flagStartX, flagStartY - 2.0f * bandHeight, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY - 2.0f * bandHeight, flagZ);
    glVertex3f(flagStartX + flagWidth, flagStartY - 3.0f * bandHeight, flagZ);
    glVertex3f(flagStartX, flagStartY - 3.0f * bandHeight, flagZ);

    // Rim
    glColor3f(0.0f, 0.0f, 0.5f);
    for (int i = 0; i < 48; i++)
    {
        float angle0 = (float)(2.0 * M_PI * i / 48);
        float angle1 = (float)(2.0 * M_PI * (i + 1) / 48);
        glVertex3f(centerX + radiusInner * (float)cos(angle0), centerY + radiusInner * (float)sin(angle0), chakraZ);
        glVertex3f(centerX + radiusOuter * (float)cos(angle0), centerY + radiusOuter * (float)sin(angle0), chakraZ);
        glVertex3f(centerX + radiusOuter * (float)cos(angle1), centerY + radiusOuter * (float)sin(angle1), chakraZ);
        glVertex3f(centerX + radiusInner * (float)cos(angle1), centerY + radiusInner * (float)sin(angle1), chakraZ);
    }

    // Hub
    for (int i = 0; i < 20; i++)
    {
        float angle0 = (float)(2.0 * M_PI * i / 20);
        float angle1 = (float)(2.0 * M_PI * (i + 1) / 20);
        glVertex3f(centerX, centerY, chakraZ);
        glVertex3f(centerX + radiusHub * (float)cos(angle0), centerY + radiusHub * (float)sin(angle0), chakraZ);
        glVertex3f(centerX + radiusHub * (float)cos(angle1), centerY + radiusHub * (float)sin(angle1), chakraZ);
        glVertex3f(centerX, centerY, chakraZ);
    }

    // Spokes
    for (int i = 0; i < 24; i++)
    {
        float angle = (float)(2.0 * M_PI * i / 24);
        float cosine = (float)cos(angle);
        float sine = (float)sin(angle);
        float offsetX = -sine * 0.005f;
        float offsetY = cosine * 0.005f;
        glVertex3f(centerX + radiusHub * cosine + offsetX, centerY + radiusHub * sine + offsetY, chakraZ);
        glVertex3f(centerX + radiusInner * cosine + offsetX, centerY + radiusInner * sine + offsetY, chakraZ);
        glVertex3f(centerX + radiusInner * cosine - offsetX, centerY + radiusInner * sine - offsetY, chakraZ);
        glVertex3f(centerX + radiusHub * cosine - offsetX, centerY + radiusHub * sine - offsetY, chakraZ);
    }

    glEnd();
}

void render(void)
{
    //code 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    glMatrixMode(GL_MODELVIEW); //Model view metix memory madhe aan //mala matrix de 
    glLoadIdentity(); //unit matrix banav model view matrix la 1-1-1

    //Translate matrix tm*CTM 
    glTranslatef(0.0f, -1.2f, -8.0f);
    glRotatef(15.0f, 0.0f, 1.0f, 0.0f);

    drawBase();
    drawPole();
    drawFlag(); 
 
    SwapBuffers(ghDC); 

}

void update(void)
{

}
