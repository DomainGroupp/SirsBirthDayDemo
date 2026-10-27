// Header files
#include<Windows.h>
#include<stdlib.h> // for exit
#include<stdio.h> // For file IO
#include "OGL.h"

#include <float.h>
#define _USE_MATH_DEFINES 1
#include<math.h>

//opengl related header files
#include<GL/GL.h>
#include<GL/GLU.h>

//link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")


//netwrok
////////////////////////////////////////////////////////////

#define NUMBER_OF_POINTS 300
#define MAX_CONNECTIONS (NUMBER_OF_POINTS * 3)
#define TEX_IMAGE_COUNT 4
BOOL LoadGLTexture(GLuint*, TCHAR[]);


void networkPoints(int);
void connectNetwork(int);
float distanceBetweenPoints(int, int);
void generateConnections(void);
void classRoom(void);
void classNetwork(void);


////////////////////////////////////////////////////////////


float anglePyramid =  0.0f;
float angleCube = 0.0f;

//network
////////////////////////////////////////////////////////////
struct network
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct NetworkConnection
{
    int startPoint;
    int endPoint;
};


struct NetworkConnection connections[MAX_CONNECTIONS];
int totalConnections = 0;
struct network points[NUMBER_OF_POINTS];
int numberOfPointsShow = 5;
int nextPointVal;
GLfloat boardColor[7][3] = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}};
int row;
GLuint Texture_Stone;
GLuint textureImageArray[TEX_IMAGE_COUNT];

////////////////////////////////////////////////////////////

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

//entry point function

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE  hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
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
            case 'P':
            case 'p':
                numberOfPointsShow = numberOfPointsShow + 5;
                if(numberOfPointsShow == NUMBER_OF_POINTS)
                {
                    numberOfPointsShow = 5;
                }
                break;
            case 'n':
                row = row + 1;
                if(row > TEX_IMAGE_COUNT - 1)
                {
                    row = 0;
                }
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


    //Filling STAR points
    // srand(67);
    //stars
    // for (int i = 0; i < NUMBER_OF_POINTS; i++)
    // {
    //     points[i].x = -1.0f + (rand() % 1000) / 500.0f;
    //     points[i].y = -1.0f + (rand() % 1000) / 500.0f;
    //     fprintf(gpFile, "points[%d].x : %f, points[%d].y : %f\n", i, points[i].x, i, points[i].y);
    // }

    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        //Generate depth first: -90 (far) to -70 (near)
        points[i].z = -90.0f + (rand() % 2001) / 100.0f;

        //Calculate how far the point can spread
        float t = (points[i].z + 90.0f) / 20.0f;

        //Interpolate the X/Y limits from far to near
        // float xLimit = 8.0f + t * (25.0f - 8.0f);
        // float yLimit = 4.0f + t * (12.0f - 4.0f);

        // //Generate X/Y within those limits
        points[i].x = -35.0f + (rand() % 10000) / 10000.0f * 70.0f;
        points[i].y = -35.0f + (rand() % 10000) / 10000.0f * 70.0f;

        // fprintf(gpFile,"points[%d]: x=%f, y=%f, z=%f, xLimit=%f, yLimit=%f\n",i,points[i].x, points[i].y, points[i].z,xLimit, yLimit);
    }

    generateConnections();
    //choose screen clear color as blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Load texture
	if (LoadGLTexture(&textureImageArray[0], MAKEINTRESOURCE(IDBITMAP_PIC1)) == FALSE)
	{
		fprintf(gpFile, " IN INITIALZE LoadGLTexture FAILED !!!!");
		return(-6);
	}

    if (LoadGLTexture(&textureImageArray[1], MAKEINTRESOURCE(IDBITMAP_PIC2)) == FALSE)
	{
		fprintf(gpFile, " IN INITIALZE LoadGLTexture FAILED !!!!");
		return(-7);
	}

    if (LoadGLTexture(&textureImageArray[2], MAKEINTRESOURCE(IDBITMAP_PIC3)) == FALSE)
	{
		fprintf(gpFile, " IN INITIALZE LoadGLTexture FAILED !!!!");
		return(-7);
	}

    if (LoadGLTexture(&textureImageArray[3], MAKEINTRESOURCE(IDBITMAP_PIC4)) == FALSE)
	{
		fprintf(gpFile, " IN INITIALZE LoadGLTexture FAILED !!!!");
		return(-7);
	}

	glEnable(GL_TEXTURE_2D);
    
    //warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

BOOL LoadGLTexture(GLuint* Texture, TCHAR imageResourceId[])
{
	// varable declerations
	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	BOOL bResult = FALSE;

	// code
	// Load the bit map as image
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	// null pass kela first parameter manje hinstance dyele aplayala la swtacha without using global varibale for handle 
	// width ani height dyeyche nahi 4,5 parameter la karan 0 deun apan snagt ahot bitmap ahea cursor kiva icon nahi 
	// mazy image la device independent bitmap mahnun load kar mazya exemadhe 

	if (hBitmap == NULL)
	{
		fprintf(gpFile, "Loadimage failed!!!!");
		return(FALSE);

	}

	if (hBitmap)
	{
		bResult = TRUE;

		// get bitmap structure from the loaded bitmap image
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		// generate opengl texture object 
		glGenTextures(1, Texture); // rikama texture tayar zala 
		// bind to thenewly created object
		glBindTexture(GL_TEXTURE_2D, *Texture);
		// Unpack the image into memory for faster loading
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
		glBindTexture(GL_TEXTURE_2D, 0);// unbind karat ahot asa kalta 0 deun
		DeleteObject(hBitmap);

		hBitmap = NULL;
	}

	return(bResult);

	// generate opengl 
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

    glTranslatef(0.0f, 0.0f, -3.0f);

    // Class RTR1 to 7
    classRoom();

    //Network
    classNetwork();

    //do double buffring
    SwapBuffers(ghdc);
}

void update(void)
{
    static int slowSpeed;
    //code
    
    if(nextPointVal < (numberOfPointsShow - 1))
    {
        if(slowSpeed++ == 30)
        {
            nextPointVal = nextPointVal + 1;
            slowSpeed = 0;
        }
    }
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

    for(int i = 0; i < TEX_IMAGE_COUNT; i++ )
    {
        glDeleteTextures(1, &textureImageArray[i]);
        textureImageArray[i] = 0;
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

void networkPoints(int networkSize)
{
    //stars
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.5f, 0.0f); //green 
    for (int i = 0; i < networkSize; i++)
    {
        glVertex3f(points[i].x, points[i].y, points[i].z);
        // glVertex3f(points[networkSize].x, points[networkSize].y, points[networkSize].z);
    }
    glEnd();
}

void connectNetwork(int visiblePoints)
{
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.5f, 0.0f);

    for (int i = 0; i < totalConnections; i++)
    {
        int start = connections[i].startPoint;
        int end = connections[i].endPoint;

        if (start < visiblePoints && end < visiblePoints)
        {
            glVertex3f(points[start].x,
                       points[start].y,
                       points[start].z);

            glVertex3f(points[end].x,
                       points[end].y,
                       points[end].z);
        }
    }

    glEnd();
}

float distanceBetweenPoints(int i, int j)
{
    float dx = points[i].x - points[j].x;
    float dy = points[i].y - points[j].y;
    float dz = points[i].z - points[j].z;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void generateConnections(void)
{
    totalConnections = 0;

    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        int nearest[3] = { -1, -1, -1 };
        float nearestDistance[3] = {
            FLT_MAX, FLT_MAX, FLT_MAX
        };

        for (int j = 0; j < NUMBER_OF_POINTS; j++)
        {
            if (i == j)
                continue;

            float distance = distanceBetweenPoints(i, j);

            for (int k = 0; k < 3; k++)
            {
                if (distance < nearestDistance[k])
                {
                    for (int m = 2; m > k; m--)
                    {
                        nearestDistance[m] = nearestDistance[m - 1];
                        nearest[m] = nearest[m - 1];
                    }

                    nearestDistance[k] = distance;
                    nearest[k] = j;
                    break;
                }
            }
        }

        for (int k = 0; k < 3; k++)
        {
            if (nearest[k] == -1)
                continue;

            int a = i;
            int b = nearest[k];

            // Store each undirected connection only once.
            if (a > b)
            {
                int temp = a;
                a = b;
                b = temp;
            }

            int duplicate = 0;

            for (int c = 0; c < totalConnections; c++)
            {
                if (connections[c].startPoint == a &&
                    connections[c].endPoint == b)
                {
                    duplicate = 1;
                    break;
                }
            }

            if (!duplicate && totalConnections < MAX_CONNECTIONS)
            {
                connections[totalConnections].startPoint = a;
                connections[totalConnections].endPoint = b;
                totalConnections++;
            }
        }
    }
}

void classRoom(void)
{
    glPushMatrix();
    glTranslatef(1.25f, 0.0f, 0.0f);

    //background
    glBegin(GL_QUADS);
    {
    glColor3f(1.0f, 0.898f, 0.8f);

    glVertex3f(1.0f, 1.20f, 0.0f);
    glVertex3f(-1.0f, 1.20f, 0.0f);
    glVertex3f(-1.0f, -1.20f, 0.0f);
    glVertex3f(1.0f, -1.20f, 0.0f);
    }
    glEnd();

    //outer borders
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    {
        glColor3f(0.902f, 0.8f, 0.702f);

        glVertex3f(0.59f, 0.96f, 0.0f);
        glVertex3f(-0.59f, 0.96f, 0.0f);
        glVertex3f(-0.59f, -0.80f, 0.0f);
        glVertex3f(0.59f, -0.80f, 0.0f);
    }
    glEnd();

    //TOP right left side
    glBegin(GL_QUADS);
    {
        glColor3f(0.902f, 0.8f, 0.702f);
        
        glVertex3f(0.53f, 0.87f, 0.0f);
        glVertex3f(0.36f, 0.87f, 0.0f);
        glVertex3f(0.36f, 0.51f, 0.0f);
        glVertex3f(0.53f, 0.51f, 0.0f);

        //left side
        glVertex3f(-0.53f, 0.87f, 0.0f);
        glVertex3f(-0.36f, 0.87f, 0.0f);
        glVertex3f(-0.36f, 0.51f, 0.0f);
        glVertex3f(-0.53f, 0.51f, 0.0f);
    }
    glEnd();

    //borders
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    {  
    glColor3f(0.902f, 0.8f, 0.702f);
    
    glVertex3f(0.55f, 0.91f, 0.0f);
    glVertex3f(-0.55f, 0.91f, 0.0f);
    glVertex3f(-0.55f, 0.46f, 0.0f);
    glVertex3f(0.55f, 0.46f, 0.0f);
    }
    glEnd();

    glBegin(GL_LINES);
    {    
    glVertex3f(0.33f, 0.91f, 0.0f);
    glVertex3f(0.33f, 0.46f, 0.0f);

    glVertex3f(-0.32f, 0.91f, 0.0f);
    glVertex3f(-0.32f, 0.46f, 0.0f);
    }
    glEnd();

    //inside
    glBegin(GL_QUADS);
    {
        glColor3f(0.0f, 0.396f, 0.239f);
        
        glVertex3f(0.29f, 0.87f, 0.0f);
        glVertex3f(0.18f, 0.87f, 0.0f);
        glVertex3f(0.18f, 0.51f, 0.0f);
        glVertex3f(0.29f, 0.51f, 0.0f);

        glColor3f(0.008f, 0.239f, 0.404f);
        
        glVertex3f(0.18f, 0.87f, 0.0f);
        glVertex3f(0.07f, 0.87f, 0.0f);
        glVertex3f(0.07f, 0.51f, 0.0f);
        glVertex3f(0.18f, 0.51f, 0.0f);

        glColor3f(0.0f, 0.396f, 0.239f);

        glVertex3f(0.07f, 0.87f, 0.0f);
        glVertex3f(-0.04f, 0.87f, 0.0f);
        glVertex3f(-0.04f, 0.51f, 0.0f);
        glVertex3f(0.07f, 0.51f, 0.0f);

        glColor3f(0.008f, 0.239f, 0.404f);

        glVertex3f(-0.04f, 0.87f, 0.0f);
        glVertex3f(-0.15f, 0.87f, 0.0f);
        glVertex3f(-0.15f, 0.51f, 0.0f);
        glVertex3f(-0.04f, 0.51f, 0.0f);

        glColor3f(0.0f, 0.396f, 0.239f);

        glVertex3f(-0.15f, 0.87f, 0.0f);
        glVertex3f(-0.27f, 0.87f, 0.0f);
        glVertex3f(-0.27f, 0.51f, 0.0f);
        glVertex3f(-0.15f, 0.51f, 0.0f);
    }
    glEnd();

    glPushMatrix();
    glTranslatef(0.0f, -1.2f, 0.0f);
    //BOTTOM right left side
    glBegin(GL_QUADS);
    {
        glColor3f(0.902f, 0.8f, 0.702f);
        
        glVertex3f(0.53f, 0.87f, 0.0f);
        glVertex3f(0.36f, 0.87f, 0.0f);
        glVertex3f(0.36f, 0.51f, 0.0f);
        glVertex3f(0.53f, 0.51f, 0.0f);

        //left side
        glVertex3f(-0.53f, 0.87f, 0.0f);
        glVertex3f(-0.36f, 0.87f, 0.0f);
        glVertex3f(-0.36f, 0.51f, 0.0f);
        glVertex3f(-0.53f, 0.51f, 0.0f);
    }
    glEnd();

    //BOTTOM CENTRE filling
    glBegin(GL_QUADS);
    {
        glColor3f(0.902f, 0.8f, 0.702f);
        
        glVertex3f(0.30f, 0.87f, 0.0f);
        glVertex3f(0.04f, 0.87f, 0.0f);
        glVertex3f(0.04f, 0.51f, 0.0f);
        glVertex3f(0.30f, 0.51f, 0.0f);

        //left side
        glVertex3f(-0.30f, 0.87f, 0.0f);
        glVertex3f(-0.04f, 0.87f, 0.0f);
        glVertex3f(-0.04f, 0.51f, 0.0f);
        glVertex3f(-0.30f, 0.51f, 0.0f);
    }
    glEnd();

    //borders
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.902f, 0.8f, 0.702f);
    {
        glVertex3f(0.55f, 0.91f, 0.0f);
        glVertex3f(-0.55f, 0.91f, 0.0f);
        glVertex3f(-0.55f, 0.46f, 0.0f);
        glVertex3f(0.55f, 0.46f, 0.0f);
    }
    glEnd();

    glBegin(GL_LINES);
    {
    glVertex3f(0.33f, 0.91f, 0.0f);
    glVertex3f(0.33f, 0.46f, 0.0f);

    glVertex3f(-0.32f, 0.91f, 0.0f);
    glVertex3f(-0.32f, 0.46f, 0.0f);

    glVertex3f(0.0f, 0.91f, 0.0f);
    glVertex3f(0.0f, 0.46f, 0.0f);
    }
    glEnd();

    glPopMatrix();

    //Black board
    // glBegin(GL_QUADS);
    // {
    //     glColor3f(boardColor[row][0], boardColor[row][1], boardColor[row][2]);

    //     glVertex3f(0.53f, 0.44f, 0.0f);
    //     glVertex3f(-0.53f, 0.44f, 0.0f);
    //     glVertex3f(-0.53f, -0.26f, 0.0f);
    //     glVertex3f(0.53f, -0.26f, 0.0f);
    // }
    // glEnd();
    glBindTexture(GL_TEXTURE_2D, textureImageArray[row]);
    
	// Rectangle Drawing Code
	glBegin(GL_QUADS);
	// right Top 
	glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.53f, 0.44f, 0.0f);
	// glVertex3f(1.0f, 1.0f, 0.0f);

	// Left top 
	glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-0.53f, 0.44f, 0.0f);
	// glVertex3f(-1.0f, 1.0f, 0.0f);

	// left bottom
	glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-0.53f, -0.26f, 0.0f);
	// glVertex3f(-1.0f, -1.0f, 0.0f);

	// right bottom
	glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.53f, -0.26f, 0.0f);
	// glVertex3f(1.0f, -1.0f, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

void classNetwork(void)
{
    glPushMatrix();
    glTranslatef(-15.0f, 0.0f, -7.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    networkPoints(numberOfPointsShow);
    connectNetwork(numberOfPointsShow);
    glPopMatrix();
}