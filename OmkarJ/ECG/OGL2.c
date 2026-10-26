// how to go to open GL from our stubb code


// header files
//freeglut.h contains this files
#include<Windows.h>
#include<stdio.h>   // for File io
#include<stdlib.h>  // for exit()
#include <string.h>
#include "OGL.h"


//OpenGL Related HeaderFiles

#include<gl/GL.h> //mazya include headerfile chya path mdhe gl navachi directery ahe tyatli GL.h ghe
#include<gl/GLU.h> //opengl utility library for gluPerspective(

// link with openGl import library
#pragma comment(lib, "opengl32.lib")  // ddl file link karaychi 2nd technique //opengl32 navachya link library sobat mala link kr
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
HGLRC ghrc = NULL;  // open gl ya librarychya rendering context baddal mahiti asnara specialist

BOOL bFullScreen = FALSE;
DWORD dwStyle ;
WINDOWPLACEMENT wpPrev; 

FILE* gpFile = NULL;

float angleCube = 0.0f;
float anglePyramid = 0.0f;

// Texture related global varibales
GLuint Texture_Stone;
GLuint Texture_John;
GLUquadric* quadric = NULL;
// texture chi identifire ahea atta nuste tayar kele

// Text rendering
GLuint font = 0;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function Declaration
    int initialize(void);       
    void Render(void);      
    void Update(void);      
    void uninitialize(void);

    // variable declarations - These are local variables. 
    
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
                Render();//glutDisplayFunc

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
                    bEscapeKeyIsPressed = TRUE;  //in glut: glutLeaveMainLoop();//
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
    BOOL LoadGLTexture(GLuint*, TCHAR[]);
    void MyDrawText(const char*, int , int ,float , float , float);
    
    // variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    //code
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));  
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);       
    pfd.nVersion = 1;   // conventional by microsoft
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

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd); // ghdc & pfd type mdhe pixel format ghe  // iPixel format mdhe apn gheto kitva format dilas
    // index is 1 based not zero based
    
    if (iPixelFormatIndex == 0)
    {
        return(-2);
    }

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE )
    {
        fprintf(gpFile, "SetPixelFormat() Failed\n");
        return(-3);
    }
    
  
    ghrc = wglCreateContext(ghdc); // mala context create krun de
    if (ghrc == NULL)
    {
        return(-4);
    }
    // ata ghdc jagi charge RC  Rendering Context la de
    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return(-5);
    }

    // Initialize font display lists
    font = glGenLists(256);

    HFONT hFont = CreateFontA(
        -24, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE,
        "Arial"
    );

    HFONT hOldFont = (HFONT)SelectObject(ghdc, hFont);

    wglUseFontBitmapsA(ghdc, 0, 256, font);

    SelectObject(ghdc, hOldFont);
    DeleteObject(hFont);

    // Enable Depth
    glShadeModel(GL_SMOOTH);// shade krtana smooth thev  // jevha depth enable nste tevha dusr model ast
    glClearDepth(1.0f);     // mazya depthbuffer mdhlya saglya values 1.0f kr // saglya mhnje all bits
    glEnable(GL_DEPTH_TEST);// depth test enable kr
    glDepthFunc(GL_LEQUAL); // lessthan equal mazya depth la enable kr buffer chya saglya values change krun 1.0 kr ani depth test enable kr aani tyavr less than or equal tp chi test kr jr 1.0 peksha jast asel trch tya value  la buffer mdhe bhr
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// 1st parameter : perspective mul je artifact yetil te correct kr hi hind    //2nd parameter : ks kru tr nicest kr

    // Choose Screen Clearing Coolor as Blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  //Blue

    // Load texture
	if (LoadGLTexture(&Texture_Stone, MAKEINTRESOURCE(IDBITMAP_STONE)) == FALSE)
	{
		fprintf(gpFile, " IN INITIALZE LoadGLTexture FAILED !!!!");
		return(-6);
	}

	glEnable(GL_TEXTURE_2D);
    // Warmup Resize 
    Resize(WINWIDTH, WINHEIGHT);


    return(0);
}

void printGLInfo(void)
{
	// code
	// printGLInfo
	fprintf(gpFile, "OPENGLINFORMATION\n");
	fprintf(gpFile, "*****************\n");
	fprintf(gpFile, "OpenGL Vendor : %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGl Renderer : %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGl Version : %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "*****************\n");
}

void MyDrawText(const char* text, int x, int y,float r, float g, float b)
{
    glColor3f(r, g, b);

    glRasterPos2i(x, y);
    glListBase(font);

    glCallLists((GLsizei)strlen(text),GL_UNSIGNED_BYTE,text);
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

void Resize(int width, int height)
{
    //code
    if (height <= 0)
        height = 1; 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); 

    // Perspective Projection
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);//first Parameter is a angle standard value, 0.1 is also standard
    
}

void Render(void)
{
    //code

           
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Open gl color cha buffer initialize mdhlya color ne clear kr // maza depth buffer pn clear kr ... kshane initialize mdhe dilelya value ne
    
    // Pyramid
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLoadIdentity();

	// translate triangle backwards by z
	glTranslatef(0.0f, 0.0f, -3.0f);

	// // rotate rectangle along y axis
	// glRotatef(anglePyramid, 0.0f, 1.0f, 0.0f);


    glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, Texture_Stone);
    
	// Rectangle Drawing Code
	glBegin(GL_QUADS);
	// right Top 
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);

	// Left top 
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);

	// left bottom
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);

	// right bottom
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

    // Render text 
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, WINWIDTH, 0.0, WINHEIGHT, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    MyDrawText("Hello Domain", 0, 0, 1.0f, 1.0f, 0.0f);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

	// Swap The Buffers
    SwapBuffers(ghdc);


}

void Update(void)
{
    //code
    // update for rectangle
    angleCube = angleCube+ 0.03f;
    if (angleCube >= 360.0f)
	{
		angleCube = angleCube - 360.0f;
	}

	anglePyramid = anglePyramid + 0.05f;
	if (anglePyramid >= 360.0f)
	{
		anglePyramid = anglePyramid - 360.0f;
	}

}

void uninitialize(void)
{

    if (font != 0)
    {
        glDeleteLists(font, 256);
        font = 0;
    }

	if (Texture_Stone)
	{
		glDeleteTextures(1, &Texture_Stone);
		Texture_Stone = 0;
	}

	// if user is exitting in full screen then restore full screen back to normal
	if (bFullScreen == TRUE)
    {
        ToggleFullscreen();
        bFullScreen = FALSE;
    }
    
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL); // donhi parameter null mhnje unmake

    }
    // Now destroy rendering contect
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // Now release device context
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }


    //Destroy Window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }


    //closelog file
    if (gpFile)
    {
        fprintf(gpFile, "OMJ: Program Terminated succesfully!!!\n\n");
        fclose(gpFile);
        gpFile = NULL;
    }

}

