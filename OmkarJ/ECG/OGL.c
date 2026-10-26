
#include<Windows.h>
#include<stdio.h>   
#include<stdlib.h>  
#include "OGL.h"

#include<gl/GL.h> 
#include<gl/GLU.h> 

#pragma comment(lib, "opengl32.lib")  
#pragma comment(lib, "GLU32.LIB")


//Macros

#define WINWIDTH 800
#define WINHEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM,LPARAM);

// global variable declarations
BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;


HWND ghwnd = NULL; 

HDC ghdc = NULL;
HGLRC ghrc = NULL;  
BOOL bFullScreen = FALSE;
DWORD dwStyle ;
WINDOWPLACEMENT wpPrev; 

FILE* gpFile = NULL;

 
int length =  1;
float pointx;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
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
    wndclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(MYICON)); 

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
        ((screenWidth/2) - (WINWIDTH/2)),
        ((screenHeigth/2) - (WINHEIGHT/2)),
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


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,LPARAM lParam){
    // code

    // Function Declarations
    void ToggleFullscreen(void);
    void Resize(int, int);
    void uninitialize(void);

    switch(iMsg) {
        case WM_CREATE :
           
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS :
            bActiveWindow = TRUE;
            break;

        case WM_KILLFOCUS:
            bActiveWindow = FALSE;
            break;

        case WM_SIZE :
  
            Resize(LOWORD(lParam), HIWORD(lParam));
            
            break;

        case WM_KEYDOWN: 
            switch (wParam)
            {
                case VK_ESCAPE : 
                    bEscapeKeyIsPressed = TRUE;  
                    break;
                default :
                    break;
            }
            break;

        case WM_CHAR :    
            switch (wParam)
            {
                case 'F':
                case 'f':
                    if(bFullScreen == FALSE)
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
                default :
                    break;
            }
            break;

        case WM_CLOSE :    
            
            uninitialize();
            break;

        case WM_DESTROY : 
            
            PostQuitMessage(0);
            break;

        default :
            break;  
    }
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen()
{
    // variable declarations
    MONITORINFO mi;
    // code

    if(bFullScreen == FALSE)// st 2
    {
        // step 3
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE); 

        if(dwStyle & WS_OVERLAPPEDWINDOW) 
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY), &mi)) 
            {
                //step 7
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW ); 
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
        ShowCursor(FALSE);
    }

    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);  
        SetWindowPlacement(ghwnd,&wpPrev);  
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

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE )
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


void YForCurrentxECG(float x, float* y)
{
    if (pointx >= x && pointx <= x + 0.6f)
        {
            if (pointx > x && pointx < x + 0.01f) //flat line before spike
            {
                *y = 0.0f;
            }
            else if (pointx >= x + 0.2f && pointx <= x + 0.6f) // spike starts here
            {
                if (pointx >= x + 0.2f && pointx < x + 0.32f) // first small triangular spike
                {
                    if (pointx <= x + 0.26f)// upward going
                        *y = *y + 0.0003f;
                    else                    //downward going
                        *y = *y - 0.00065f;
                }
                else if (pointx >= x + 0.32f && pointx < x + 0.4f)  // that big spike
                {
                    if (pointx <= x + 0.36f)    // upgoing
                        *y = *y + 0.004f;
                    else                        // downgoing
                        *y = *y - 0.005f;
                }
                else if (pointx > x + 0.4f && pointx <= x + 0.45f) // line comuing  to 0 after big spike
                {
                    *y = 0.0f;
                }
                else if (pointx > x + 0.45f && pointx < x + 0.6f) // last small triangular spike
                {
                    if (pointx <= x + 0.53f)       // upgoing
                        *y = *y + 0.0003f;
                    else                            // downgoing
                    {
                        *y = *y - 0.0005f;

                        if (*y < 0.0f)          // making it 0  after end of spike
                            *y = 0.0f;
                    }
                }
            }
        }
}

void DrawECG()
{
    pointx = -3.5f;
    float z = -4.0f;    
    float y = 0.0f;
    float *yptr = &y;
    int x = 0.0f;
    int i;

    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0f, 1.0f, 0.0f);
    for(i = 0; i < length; i++)
    {
         
                // First spike
        YForCurrentxECG(-2.9f, yptr);

        YForCurrentxECG(-1.9f, yptr);

        YForCurrentxECG(-0.9f, yptr);

        YForCurrentxECG(0.9f, yptr);

        YForCurrentxECG(-1.9f, yptr);

        YForCurrentxECG(-2.9f, yptr);
       
        glVertex3f(pointx, y, z);

        pointx = pointx + 0.0002;
    }
    glEnd();

}

void Render(void)
{
    //code
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    DrawECG();  

    SwapBuffers(ghdc);


}

void Update(void)
{
    //code
   if(length <= (3.5*2)/0.0002f)
   {
        length  = length+1;
   }
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
    if (gpFile)
    {
        fprintf(gpFile, "OMJ: Program Terminated succesfully!!!\n\n");
        fclose(gpFile);
        gpFile = NULL;
    }

}

