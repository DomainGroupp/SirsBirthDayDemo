// Header files
#include<Windows.h>
#include<stdlib.h> // for exit
#include<stdio.h> // For file IO
#include "OGL.h"
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")


//opengl related header files
#include<GL/GL.h>
#include<GL/GLU.h>

//link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")

/***********************************************************************/
#define _USE_MATH_DEFINES 1
#include<math.h>

//user define
// #define USE_CAMERA 0

#define get_rad_angle(theta) theta * M_PI/180
#define sinf_angle(amp,theta) (amp * sinf(get_rad_angle(theta)))
#define cosf_angle(amp,theta) (amp * cosf(get_rad_angle(theta)))

void textureCube(void);
void drawTextureCube(GLfloat, GLfloat, GLfloat, GLuint);
void toggleFullScreen(void);

int moveToward(GLfloat *, GLfloat, GLfloat);
void updateLimbs(void);
void torso(void);
void head(void);
void neck(void);
void arm(void);
void leg(GLfloat, int, int);
void cube(void);
void model(void);

int hip = 0;
int knee = 0;
GLfloat twist = 0.0f;
GLfloat shoulder = -75.0f;
GLfloat elbow = 15.0f;

GLfloat hipAuto;
GLfloat left_knee;
GLfloat right_knee;
GLfloat armSwing = 0;
GLfloat elbowRotate = 0;
GLfloat angle = 0.0f;
GLfloat posZ = -36.0f;
GLfloat posX = -14.0f;
GLfloat heading = 30.3f;
GLfloat reveal = 0.0f;
GLfloat revealSpeed = 0.0f;
GLfloat camZ = 0.0f;
GLfloat orbitAngle = 0.0f;
GLfloat orbitSpeed = 0.0f;
GLuint texture_cbook = 0;
GLuint texture_unix = 0;
GLuint texture_kundali = 0;
GLuint texture_opengl = 0;
GLfloat phase = 0.0f;
GLfloat walkAmount = 1.0f;

GLuint texture_win32 = 0;
GLuint texture_medicine = 0;
GLuint texture_anatomy = 0;
GLuint texture_cuda = 0;

BOOL bStopAnime = FALSE;
BOOL bStartAnime = TRUE;

int sceneState = 0;
float stateTimer = 0.0f;
ULONGLONG pauseStart = 0;
/***********************************************************************/

//globale variab;le declaration
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL bFullScreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
FILE *gpFile = NULL;

BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;

//Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global fuction delcarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLUquadric *quadric = NULL;

//entry point function

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE  hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    PlaySound(TEXT("Hukum.wav"), NULL, SND_ASYNC | SND_FILENAME);
    //func declarations
    int initialize(void);
    void unintialize(void);
    void render(void);
    void update(void);
    
    //create log file 
    gpFile = fopen("Log.txt", "w");
    
    if(gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log file creation failed"), TEXT("ERROR"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program started successfully\n");
    }

    //variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_SSH");
    BOOL bDone = FALSE;

    //code
    //WNDCLASSEX initialization

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style =  CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    //Register above wndClass
    RegisterClassEx(&wndclass);

    //centering
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    //create the window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR7 Siddhesh Sharad Hande"),WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, ScreenWidth/2 - WIN_WIDTH/2, ScreenHeight/2 - WIN_HEIGHT/2, WIN_WIDTH, WIN_HEIGHT, NULL,NULL, hInstance, NULL);

    //set global window handle
    ghwnd = hwnd;

    int iResult = initialize();
    if(iResult != 0)
    {
        fprintf(gpFile, "SSH: WinMain: initialize() failed");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "SSH: WinMain: initialize() successed");
    }

    //Show Window
    ShowWindow(hwnd, iCmdShow);

    //Update the window to paint its background

    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    toggleFullScreen();

    //Game loop
    while(bDone == FALSE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
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
            if(bActiveWindow == TRUE)
            {
                if(bEscapeKeyIsPressed == TRUE)
                {
                    bDone = TRUE;
                }
                //render
                render();
                //update
                update();
            }
        }
    }
    
    //unintialize
    unintialize();

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //func declarations
    void toggleFullScreen(void);
    void resize(int, int);
    void unintialize(void);

    //code
    switch(iMsg)
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
            switch(wParam)
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
                if(bFullScreen == FALSE)
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

            // case 'E':
            //     elbow = (elbow + 3) % 360;
            //     break;
            
            // case 'e':
            //     elbow = (elbow - 3) % 360;
            //     break;
            
            // case 'S':
            //     shoulder = (shoulder + 3) % 360;
            //     break;

            // case 's':
            //     shoulder = (shoulder - 3) % 360;
            //     break;
            
            case 'H':
                hip = (hip + 3) % 360;
                break;

            case 'h':
                hip = (hip - 3) % 360;
                break;
            
            // case 'T':
            //     twist = (twist + 3) % 360;
            //     break;

            // case 't':
            //     twist = (twist - 3) % 360;
            //     break;

            case 'K':
                knee = (knee + 12) % 360;
                break;

            case 'k':
                knee = (knee - 12) % 360;
                break;
            
            case 'r':
                bStartAnime = TRUE;
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
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    //variable decln
    MONITORINFO mi;

    //code
    if(bFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED); //WM_NCCALCSIZE            
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
    //code
    //Func declarations
    void resize(int, int);

    //variable declarations
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
    if(ghdc == NULL)
    {
        return -1;
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        return -2;
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return -3;
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        return -4;
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -5;
    }

    //Enable depth
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    //choose screen clear color as blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    BOOL bResult = FALSE;
    bResult = loadGLTexture(&texture_cbook, MAKEINTRESOURCE(IDBITMAP_C_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }

    bResult = loadGLTexture(&texture_opengl, MAKEINTRESOURCE(IDBITMAP_GL_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }

    bResult = loadGLTexture(&texture_kundali, MAKEINTRESOURCE(IDBITMAP_KUNDALI_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }

    bResult = loadGLTexture(&texture_unix, MAKEINTRESOURCE(IDBITMAP_UNIX_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }

    bResult = loadGLTexture(&texture_anatomy, MAKEINTRESOURCE(IDBITMAP_ANA_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }

    bResult = loadGLTexture(&texture_cuda, MAKEINTRESOURCE(IDBITMAP_CUDA_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }
    bResult = loadGLTexture(&texture_medicine, MAKEINTRESOURCE(IDBITMAP_MED_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }
    bResult = loadGLTexture(&texture_win32, MAKEINTRESOURCE(IDBITMAP_WIN_BOOK));
    if(bResult == FALSE)
    {
        fprintf(gpFile, "Load gl texutre failed for c book \n");
        return -7;
    }


    quadric = gluNewQuadric();

    //warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    //code
    if(height <= 0)
    {
        height = 1;
    }

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    //Perspective projection
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
}

void render(void)
{
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    #ifdef USE_CAMERA
    gluLookAt(0.0f, 0.0f, camZ, posX, 1.0f, posZ, 0.0f, 1.0f, 0.0f);
    #endif

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPushMatrix();
    glTranslatef(posX, 3.0f, posZ);
    glRotatef(heading, 0.0f, 1.0f, 0.0f);
    model();

    glPushMatrix();

    glTranslatef(0.0f, -2.5f, 0.0f);
    
    //Enable texturing
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    drawTextureCube(0.0f, 0.0f + orbitAngle, 2.5f, texture_cbook);
    drawTextureCube(0.0f, 45.0f + orbitAngle, 2.5f, texture_kundali);
    drawTextureCube(0.0f, 90.0f + orbitAngle, 2.5f, texture_opengl);
    drawTextureCube(0.0f, 135.0f + orbitAngle, 2.5f,texture_unix);
    drawTextureCube(0.0f, 180.0f + orbitAngle, 2.5f, texture_anatomy);
    drawTextureCube(0.0f, 225.0f + orbitAngle, 2.5f, texture_cuda);
    drawTextureCube(0.0f, 270.0f + orbitAngle, 2.5f, texture_win32);
    drawTextureCube(0.0f, 315.0f + orbitAngle, 2.5f, texture_medicine);

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();

    if(reveal < 8.0f)
    {
        glPushMatrix();
        glTranslatef(0.0f, -7.2f + reveal, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
        gluCylinder(quadric, 3.5f, 3.5f, 8.0f - reveal, 40, 1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -7.2f + reveal, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        // glColor3f(1.0f, 0.85f, 0.3f);
        glColor3f(0.85f, 1.0f, 1.0f);
        gluCylinder(quadric, 3.55f, 3.55f, 0.08f, 40, 1);
        glPopMatrix();

        // glPushMatrix();
        // glTranslatef(0.0f, -7.2f + reveal, 0.0f);
        // glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        // // glColor3f(0.5f, 0.4f, 0.1f);
        // glColor3f(0.0f, 0.9f, 1.0f);
        // gluCylinder(quadric, 3.6f, 3.6f, 0.2f, 40, 1);
        // glPopMatrix();

        // glPushMatrix();
        // glTranslatef(0.0f, -7.2f + reveal, 0.0f);
        // glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        // // glColor3f(0.25f, 0.2f, 0.05f);
        // glColor3f(0.0f, 0.3f, 0.4f);
        // gluCylinder(quadric, 3.65f, 3.65f, 0.35f, 40, 1);
        // glPopMatrix();
    }

    glPopMatrix();

    //do double buffring
    SwapBuffers(ghdc);
}

void update(void)
{
    //code

    switch(sceneState)
    {
        case 0:
        //for inital full screen mode
        stateTimer = stateTimer + 1.0f;
        if(stateTimer >= 150.0f)
        {
            sceneState = 1;
            stateTimer = 0.0f;
        }
        break;

        case 1:
        // model revealing cylinder code
        revealSpeed = revealSpeed + 0.00001f;
        #ifdef USE_CAMERA
        camZ = camZ - 0.019;
        #endif
        reveal = reveal + revealSpeed;
        if(reveal >= 8.0f)
        {
            reveal = 8.0f;
            sceneState = 2;
        }
        break;

        case 2:
        //pause before movement
        stateTimer = stateTimer + 1.0f;
        if(stateTimer >= 60.0f)
        {
            sceneState = 3;
            stateTimer = 0.0f;
        }
        break;

        case 3:
        //complete model movement
        orbitSpeed = orbitSpeed + 0.005f;
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        if(orbitSpeed >= 0.5f)
        {
            sceneState = 4;
            orbitSpeed = 0.5f;
        }
        break;

        case 4:
        #ifdef USE_CAMERA
        camZ = posZ + 12.0f;
        #endif
        orbitSpeed = orbitSpeed + 0.005f;
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        if(orbitSpeed >= 0.5f)
        {
            orbitSpeed = 0.5f;
        }
        if(!bStopAnime)
        {
            updateLimbs();
            GLfloat rad = heading * M_PI / 180;
            posZ = posZ + 0.012f * cos(rad);
            posX = posX + 0.012f * sin(rad);

            if(posZ > -12.0f)
            {
                posX = 0.0f;
                posZ = -12.0f;
                sceneState = 5;
            }
        }
        break;

        case 5:
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        updateLimbs();
        walkAmount = walkAmount - 0.02;
        heading = 30.3 * walkAmount;
        if(walkAmount <= 0.0f)
        {
            walkAmount = 0;
            hipAuto = 0.0f;
            left_knee = 0.0f;
            right_knee = 0.0f;
            armSwing = 0.0f;
            heading = 0.0f;
            // stateTimer = 0.0f;
            pauseStart = GetTickCount64();
            sceneState = 10;
        }
        break;
        
        case 6:
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        if (moveToward(&shoulder, 75.0f, 0.3f))
            sceneState = 7;

        break;

        case 7:
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        if (moveToward(&twist, -72.0f, 0.5f))
            sceneState = 8;

        break;

        case 8:
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }
        if (moveToward(&elbow, 72.0f, 0.5f))
            // sceneState = 7;

        break;

        case 10:
        orbitAngle = orbitSpeed + orbitAngle;
        if (orbitAngle >= 360.0f)
        {
            orbitAngle =orbitAngle -360.0f;
        }

        if (GetTickCount64() - pauseStart >= 3500)
            sceneState = 6;
        break;
        // stateTimer = stateTimer + 1.0f;
        // if(stateTimer >= 210.0f)
        // {
        //     sceneState = 6;
        //     stateTimer = 0.0f;
        // }
        break;


        default:
        break;
    }
    return;
}

void unintialize(void)
{
    //code
    // if exiting in full screen first restore and then proceed
    if(bFullScreen == TRUE)
    {
        toggleFullScreen();
        bFullScreen = FALSE;
    }

    //first cehck the current context and if it is then make it unmake it
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // now destroy
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    //release device context
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if(quadric)
    {
        gluDeleteQuadric(quadric);
        quadric = NULL;
    }

    //destroy window
    if(ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    //Close log file
    if(gpFile)
    {
        fprintf(gpFile,"Program termionated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

void updateLimbs(void)
{
    //hip movement
            
    phase = phase + 0.02f;

    if(phase > 6.283f)
    {
        phase = phase - 6.283f;
    }
    hipAuto = 15.0f * sin(phase) * walkAmount;

    right_knee = 35 * (-cos(phase)) * walkAmount;
    if(right_knee < 0)
        right_knee = 0;

    left_knee = 35 * (cos(phase)) * walkAmount;
    if(left_knee < 0)
        left_knee = 0;

    armSwing = 20 * sin(phase) *walkAmount;
}

int moveToward(GLfloat *value, GLfloat target, GLfloat step)
{
    if (fabsf(target - *value) <= step)
    {
        *value = target;   // close enough: snap exactly to target
        return 1;          // arrived
    }

    if (*value < target)
        *value = *value + step;
    else
        *value = *value - step;

    return 0;              // still moving
}

void arm(void)
{
    glPushMatrix();

    //ARM
    glTranslatef(0.8, -1.0f, 0.0f);
    
    //shoulder joint
    glPushMatrix();
    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3f(0.80f, 0.50f, 0.55f);
    gluSphere(quadric, 0.25f, 10, 10);
    glPopMatrix();

    glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    //upperarm
    glRotatef((GLfloat) shoulder, 0.0f, 0.0f, 1.0f);
    glRotatef(twist, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.75, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.7f, 0.45f, 0.5f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //forearm
    glTranslatef(0.75f, 0.0f, 0.0f);
    
    //elbow joint
    glPushMatrix();
    glColor3f(0.80f, 0.50f, 0.55f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    // glRotatef((GLfloat)elbow, 0.0f, 0.0f, 1.0f);
    glRotatef(-(GLfloat)elbow, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.4f, 0.35f, 0.4f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //right hand
    glTranslatef(0.65f, 0.0f, 0.0f);
    glPushMatrix();
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    glScalef(1.2f, 0.8f, 0.6f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    glPopMatrix();

    //left arm
    glPushMatrix();

    glTranslatef(-0.8, -1.0f, 0.0f);
    
    //left shoulder joint
    glPushMatrix();
    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3f(0.80f, 0.50f, 0.55f);
    gluSphere(quadric, 0.25f, 10, 10);
    glPopMatrix();

    glRotatef(armSwing, 1.0f, 0.0f, 0.0f);

    //left upperarm
    glRotatef(-(GLfloat) shoulder, 0.0f, 0.0f, 1.0f);
    glRotatef(twist, 1.0f, 0.0f, 0.0f);
    glTranslatef(-0.75, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.7f, 0.45f, 0.5f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //left forearm
    glTranslatef(-0.75f, 0.0f, 0.0f);
    
    //left elbow joint
    glPushMatrix();
    glColor3f(0.80f, 0.50f, 0.55f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    // glRotatef(-(GLfloat)elbow, 0.0f, 0.0f, 1.0f);
    glRotatef((GLfloat)elbow, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.4f, 0.35f, 0.4f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //left hand
    glTranslatef(-0.65f, 0.0f, 0.0f);
    glPushMatrix();
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    glScalef(1.2f, 0.8f, 0.6f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    glPopMatrix();

    fprintf(gpFile, "shoulder : %f, elblow ; %f, twist : %f\n", (GLfloat)shoulder, (GLfloat) elbow, twist);
}

void torso(void)
{
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);
    glScalef(0.8f, 1.2f, 0.4f);

    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3f(0.80f, 0.50f, 0.55f);
    
    cube();

    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -3.0f, 0.0f);
    glScalef(1.0f, 1.0f, 0.55f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(0.80f, 0.50f, 0.55f);
    gluCylinder(quadric, 0.85f, 0.95f, 1.7f, 20, 20);
    glPopMatrix();
}

void head(void)
{
    //head
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    gluSphere(quadric, 0.50, 30, 30);
    glPopMatrix();

    //hair
    glPushMatrix();
    glTranslatef(0.0f, 0.06f, -0.08f);
    glColor3f(0.20f, 0.20f, 0.20f);
    gluSphere(quadric, 0.53f, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.20f, 0.20f, 0.20f);
    glTranslatef(0.12f, 0.3f, 0.07f);
    glScalef(1.0f, 0.4f, 0.9f);
    glColor3f(0.20f, 0.20f, 0.20f);
    gluSphere(quadric, 0.45f, 20, 20);
    glPopMatrix();

    //eyes
    glPushMatrix();
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-6.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.48f);
    glScalef(1.0f, 1.0f, 0.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluSphere(quadric, 0.07, 20, 20);
    glTranslatef(0.0f, 0.0f, 0.06f);
    glColor3f(0.0f, 0.0f, 0.0f);
    gluSphere(quadric, 0.035, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(-6.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.48f);
    glScalef(1.0f, 1.0f, 0.5f);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluSphere(quadric, 0.07, 20, 20);
    glTranslatef(0.0f, 0.0f, 0.06f);
    glColor3f(0.0f, 0.0f, 0.0f);
    gluSphere(quadric, 0.035, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glRotatef(24.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.47f);
    glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.09f, 0.0f, 0.0f);
    glScalef(1.0f, 0.25f, 0.6f);
    glColor3f(0.20f, 0.20f, 0.20f);
    gluSphere(quadric, 0.1f, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glRotatef(24.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.47f);
    glRotatef(15.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.09f, 0.0f, 0.0f);
    glScalef(1.0f, 0.25f, 0.6f);
    glColor3f(0.20f, 0.20f, 0.20f);
    gluSphere(quadric, 0.1f, 20, 20);
    glPopMatrix();
}

void neck(void)
{
    //neck
    glPushMatrix();

    glTranslatef(0.0f, -0.5f, 0.0f);  // Position neck
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // 

    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    gluCylinder(quadric, 0.15, 0.15, 0.25, 20, 5);

    glPopMatrix();
}

void leg(GLfloat hipX, int hipAngle, int kneeAngle)
{
    glPushMatrix();

    glColor3f(0.7f, 0.7f, 0.7f);
    
    //thigs
    glTranslatef(hipX, -3.1f, 0.0f);
    gluSphere(quadric, 0.3f, 10, 10);
    glRotatef(hipAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.30f, 0.27f, 1.9f, 20, 5);

    //knee
    glTranslatef(0.0f, 0.0f, 1.85f);
    gluSphere(quadric, 0.23f, 10, 10);
    glRotatef(kneeAngle, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.27f, 0.25f, 1.7f, 10, 10);

    glTranslatef(0.0f, 0.0f, 1.65f);
    gluSphere(quadric, 0.17, 20, 20);

    glPushMatrix();
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    glTranslatef(0.0f, 0.25f, 0.1f);
    glScalef(0.2f, 0.4f, 0.1f);
    cube();
    glPopMatrix();

    glPopMatrix();
}

void l_eg(void)
{
    glPushMatrix();

    glColor3f(0.7f, 0.7f, 0.7f);
    
    //thigs
    glTranslatef(0.4f, -3.1f, 0.0f);
    gluSphere(quadric, 0.3f, 10, 10);
    glRotatef(hip, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.30f, 0.27f, 1.9f, 20, 5);

    //knee
    glTranslatef(0.0f, 0.0f, 1.85f);
    gluSphere(quadric, 0.23f, 10, 10);
    glRotatef(knee, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.27f, 0.25f, 1.7f, 10, 10);

    glTranslatef(0.0f, 0.0f, 1.65f);
    gluSphere(quadric, 0.17, 20, 20);

    glPushMatrix();
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    glTranslatef(0.0f, 0.25f, 0.1f);
    glScalef(0.2f, 0.4f, 0.1f);
    cube();
    glPopMatrix();

    glPopMatrix();

    //left leg
    glPushMatrix();

    glColor3f(0.7f, 0.7f, 0.7f);
    
    //thigs
    glTranslatef(-0.4f, -3.1f, 0.0f);
    gluSphere(quadric, 0.3f, 10, 10);
    glRotatef(hip, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.30f, 0.27f, 1.9f, 20, 5);

    //knee
    glTranslatef(0.0f, 0.0f, 1.85f);
    gluSphere(quadric, 0.23f, 10, 10);
    glRotatef(-knee, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.27f, 0.25f, 1.7f, 10, 10);

    glTranslatef(0.0f, 0.0f, 1.65f);
    gluSphere(quadric, 0.17, 20, 20);

    glPushMatrix();
    glColor3f(0.8f, 0.6f, 0.4f); // Skin tone
    glTranslatef(0.0f, 0.25f, 0.1f);
    glScalef(0.2f, 0.4f, 0.1f);
    cube();
    glPopMatrix();

    glPopMatrix();

}

void cube(void)
{
    glBegin(GL_QUADS);
	//red, right green, back blue, left cyn, top magenta, bot yellow

    // front
    // glColor3f(1.0f, 0.0f, 0.0f);
	
    glVertex3f(1.0f, 1.0f, 1.0f);

    glVertex3f(-1.0f, 1.0f, 1.0f);
	
	glVertex3f(-1.0f, -1.0f, 1.0f);
	
	glVertex3f(1.0f, -1.0f, 1.0f);

    //right
    // glColor3f(0.0f, 1.0f, 0.0f);
	
    glVertex3f(1.0f, 1.0f, -1.0f);

    glVertex3f(1.0f,  1.0f,  1.0f);
	
	glVertex3f(1.0f, -1.0f,  1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);

    //back
    // glColor3f(0.0f, 0.0f, 1.0f);
	
    glVertex3f(1.0f,  1.0f, -1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);

    //left
    // glColor3f(1.0f, 0.0f, 1.0f);
	
    glVertex3f(-1.0f,  1.0f,  1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f,  1.0f);
	
    //top
    // glColor3f(0.0f, 1.0f, 1.0f);
	
    glVertex3f(1.0f,  1.0f, -1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f,  1.0f,  1.0f);
	
	glVertex3f(1.0f,  1.0f,  1.0f);

    //bottom
    // glColor3f(1.0f, 1.0f, 0.0f);
	
    glVertex3f(1.0f, -1.0f,  1.0f);

    glVertex3f(-1.0f, -1.0f,  1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);
    glEnd();
}

void model(void)
{
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // glTranslatef(posX, 3.0f, posZ);
    // glRotatef(heading, 0.0f, 1.0f, 0.0f);
    
    head();
    neck();
    arm();
    torso();
    leg(0.4f, hipAuto, right_knee);
    leg(-0.4f, -hipAuto, left_knee);
}

void textureCube(void)
{
    glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

    //right
    //glColor3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f,  1.0f,  1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f,  1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

    //back
    //glColor3f(0.0f, 0.0f, 1.0f);
	
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f,  1.0f, -1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

    //left
    //glColor3f(1.0f, 0.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f,  1.0f);
	
    //top
    //glColor3f(0.0f, 1.0f, 1.0f);
	
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f,  1.0f, -1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f,  1.0f,  1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f,  1.0f,  1.0f);

    //bottom
    //glColor3f(1.0f, 1.0f, 0.0f);
	
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f,  1.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
	
    glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
    glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
    glEnd();
}

void drawTextureCube(GLfloat yAxis, GLfloat angle, GLfloat amp, GLuint texture_num)
{
    glPushMatrix();
    glTranslatef(sinf_angle(amp, angle), yAxis, cosf_angle(amp, angle));
    glScalef(0.5f, 0.5f, 0.5f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, texture_num);
    textureCube();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

BOOL loadGLTexture(GLuint *tex, TCHAR ImageResouceId[])
{
    //variable declarations
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    //code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ImageResouceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if(hBitmap == NULL)
    {
        fprintf(gpFile, "Failed\n");
        return FALSE;
    }

    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
    glBindTexture(GL_TEXTURE_2D, 0);
    DeleteObject(hBitmap);
    
    hBitmap = NULL;

    return TRUE;
}
