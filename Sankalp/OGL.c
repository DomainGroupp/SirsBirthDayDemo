#define _USE_MATH_DEFINES 1 // Must come before math.h for M_PI
#include<windows.h>
#include<stdio.h> // As such we do not need this as FILE is already in windows.h -> But we are writing this to explicity use this for File I/O
#include<stdlib.h> // For exiting with exit()
#include<string.h> // For memset()
#include<math.h>

//Add opengl header file -
#include<gl/GL.h>
#include<gl/GLU.h>

#include "OGL.h"


// Link with Opengl import library 
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib, "GLU32.lib")

// Macros
#define WIN_WIDTH 800 
#define WIN_HEIGHT 600

BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM,LPARAM);


// Global variable declarations 
HWND ghwnd = NULL; // g for global -> This is global hwnd; NULL can be skipped here as NULL internally is #define 0 
HDC ghdc =  NULL; // Graphic card ka specialist. It can tell the context of the graphics card
HGLRC ghrc = NULL;  // Rendering context of open gl ke bare main information rakhta h


BOOL bFullscreen = FALSE;  // By default our window is normal not fullscreen
DWORD dwStyle; // dw -> Double word
WINDOWPLACEMENT wpPrev; // hungarian notation -> Meaningfull name like hPrevInstance -> Just like this previous placement of the window yaha save karne wale h hum

// For log files 
FILE *gpFile = NULL; // Global pointer To File. File ka pointer abhi filal ke leye NULL 


// For theatre 
GLUquadric* quadric = NULL;
float curtainPhase = 0.0f;
const char pratibimba[] = "PRATIBIMBA";

// For curtain movement 
float leftCurtainXRight = 0.0f;
float rightCurtainXLeft = 0.0f;

// For zooming out movement 
float eyeZ = 1.0f;

// For pati movement 
float boardX = 9.0f;
float board2X = 5.8f; // second board

// Text rendering
GLuint font = 0;                 // Base of the 256 display lists, one per character
int gWinWidth = WIN_WIDTH;       // Current client size, updated in resize(); used for the text ortho projection
int gWinHeight = WIN_HEIGHT;


// Audience 
int audience_shoulder = -80;  
int audience_elbow = 0;  
#define MAX_AUDIENCE 16 
GLfloat skinColor[MAX_AUDIENCE][3]; // 3 for RGB 
GLfloat hairColor[MAX_AUDIENCE][3];
GLfloat clothColor[MAX_AUDIENCE][3];
int gCurrentMan = 0; // This we will use to know which man we are rendering out of say max 16 men


void randomMan(){
    GLfloat skinPalette[][3] = {
    {0.80f, 0.60f, 0.40f}, {0.95f, 0.76f, 0.60f}, {0.65f, 0.45f, 0.30f},
    {0.45f, 0.30f, 0.20f}, {0.88f, 0.68f, 0.52f}
    };

    GLfloat hairPalette[][3] = {
    {0.20f, 0.20f, 0.20f}, {0.05f, 0.05f, 0.05f}, {0.35f, 0.20f, 0.10f},
    {0.60f, 0.45f, 0.25f}, {0.70f, 0.70f, 0.70f}
    };
    int nSkin = sizeof(skinPalette) / sizeof(skinPalette[0]);
    int nHair = sizeof(hairPalette) / sizeof(hairPalette[0]);

    for(int i = 0 ; i < MAX_AUDIENCE ; i++){
        int s = rand() % nSkin;
        int h = rand() % nHair;
        for(int c = 0 ; c<3; c++){
            skinColor[i][c] = skinPalette[s][c];
            hairColor[i][c] = hairPalette[h][c];
            clothColor[i][c] = 0.15f + (rand() / (GLfloat)RAND_MAX) * 0.75f;
        }
    }
}


// Draws 2D text on top of the scene. x, y are in pixels from the bottom-left of the window.
void MyDrawText(const char* text, int x, int y, float r, float g, float b)
{
    // Switch to a pixel-based 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, gWinWidth, 0.0, gWinHeight, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST); // Text must not be hidden by the 3D scene

    glColor3f(r, g, b);
    glRasterPos2i(x, y);
    glListBase(font);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);

    glEnable(GL_DEPTH_TEST);

    // Restore the 3D matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Draws bitmap text anchored at a 3D point (uses the current modelview), horizontally centred on x.
// Bitmap text keeps a fixed pixel size, but its position follows the object it is written on.
void MyDrawText3D(const char* text, float x, float y, float z, float r, float g, float b)
{
    glColor3f(r, g, b);
    glRasterPos3f(x, y, z);
    glBitmap(0, 0, 0.0f, 0.0f, -(float)strlen(text) * 8.0f, 0.0f, NULL); // shift left by ~half the text width in pixels
    glListBase(font);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

float tileWidth = 0.5f;
// Shades for the tiles
float shades[4][3] = {
    {0.28f, 0.16f, 0.08f},  
    {0.24f, 0.13f, 0.06f}, 
    {0.31f, 0.18f, 0.09f},  
    {0.21f, 0.11f, 0.05f}   
};

// void textBig(float x, float y, float z,  float scaleX, float scaleY, float scaleZ,const char*string){
//     glPushMatrix();
//         glTranslatef(x, y, z);
//         glScalef(scaleX, scaleY, scaleZ); 
//         glColor3f(1.0f, 1.0f, 1.0f);
//         glLineWidth(3.0f);
//         for (const char *c = string; *c != '\0'; c++) {
//             glutStrokeCharacter(GLUT_STROKE_ROMAN,*c);
//         }
//     glPopMatrix();
// }

void curtainColor(float arg, float t) {
    float light = 0.5f + 0.5*cos(arg); // slope of wave 
    float k = (0.35f + 0.65f * light) * (1.0f - 0.35f * t);  // darker at the bottom
    glColor3f(0.65f*k, 0.08f*k,0.10f*k);
}

void drawCurtain(float xLeft, float xRight, float yTop, float yBottom, float amp, float freq, float phase){
    const int rows = 20;  // down the height
    const int cols = 60; // across the width
    float dx = (xRight - xLeft)/cols; 
    float dy = (yTop - yBottom)/rows;
    for(int c= 0; c< cols; c++){
        float xa = xLeft + c * dx;
        float xb = xa + dx; 
        float argA = freq * xa + phase;
        float argB = freq * xb + phase;

        glBegin(GL_QUAD_STRIP);
        for(int r = 0; r <= rows; r++){
            float y = yTop - r * dy; // Current height
            float t = (float)r/rows; // How far down we are ? 0 at the top, 1 at the bottom
            float amplitude = amp * (0.4f + 0.6f *t);
            curtainColor(argA, t);
            glVertex3f(xa,y,amplitude*sin(argA));
            curtainColor(argB, t);
            glVertex3f(xb, y, amplitude*sin(argB));
        }
        glEnd();
    }

}

void brownFrame(){
    glBegin(GL_QUADS);
        glColor3f(0.45f, 0.26f, 0.12f);  
        glVertex3f( 2.2f,  0.9f, 1.02f);
        glVertex3f(-2.2f,  0.9f, 1.02f);
        glColor3f(0.30f, 0.16f, 0.07f);  
        glVertex3f(-2.2f, -0.55f, 1.02f);
        glVertex3f( 2.2f, -0.55f, 1.02f);
    glEnd();
}


void cube(float x, float y , float z){
    glTranslatef(x,y,z);
    glScalef(0.3f,0.5f,0.1f);
    glBegin(GL_QUADS);
    // Front face (burgundy)
    glColor3f(0.50f, 0.07f, 0.09f);
    glVertex3f(1.0f,1.0f,1.0f);
    glVertex3f(-1.0f,1.0f,1.0f);
    glColor3f(0.20f, 0.02f, 0.04f);
    glVertex3f(-1.0f,-1.0f,1.0f);
    glVertex3f(1.0f,-1.0f,1.0f);

    // Right face (wood frame)
    glColor3f(0.28f, 0.15f, 0.08f);
    glVertex3f(1.0f,1.0f,-1.0f);
    glVertex3f(1.0f,1.0f,1.0f);
    glColor3f(0.18f, 0.09f, 0.05f);
    glVertex3f(1.0f,-1.0f,1.0f);
    glVertex3f(1.0f,-1.0f,-1.0f);

    // Back face
    glColor3f(0.30f, 0.04f, 0.06f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glVertex3f(1.0f,1.0f,-1.0f);
    glColor3f(0.15f, 0.02f, 0.03f);
    glVertex3f(1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);

    // Left face (wood frame)
    glColor3f(0.28f, 0.15f, 0.08f);
    glVertex3f(-1.0f,1.0f,1.0f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glColor3f(0.18f, 0.09f, 0.05f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,1.0f);

    // Top face
    glColor3f(0.42f, 0.05f, 0.07f);
    glVertex3f(1.0f,1.0f,-1.0f);
    glVertex3f(-1.0f,1.0f,-1.0f);
    glVertex3f(-1.0f,1.0f,1.0f);
    glVertex3f(1.0f,1.0f,1.0f);

    // Bottom face
    glColor3f(0.15f, 0.02f, 0.03f);
    glVertex3f(1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,-1.0f);
    glVertex3f(-1.0f,-1.0f,1.0f);
    glVertex3f(1.0f,-1.0f,1.0f);
    glEnd();

    // Seat cushion
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.05f, 0.07f);
    glVertex3f(1.0f,-1.0f,-1.0f); // LB
    glVertex3f(-1.0f,-1.0f,-1.0f); // RB
    glColor3f(0.25f, 0.03f, 0.05f);
    glVertex3f(-1.0f,-1.0f,-5.0f);
    glVertex3f(1.0f,-1.0f,-5.0f);
    glEnd();
}



// For audience 

void audience_arm(void)
{
    glPushMatrix();

    //audience_arm
    glTranslatef(0.8, -1.0f, 0.0f);
    
    //audience_shoulder joint
    glPushMatrix();
    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3fv(clothColor[gCurrentMan]);
    gluSphere(quadric, 0.25f, 10, 10);
    glPopMatrix();

    // glRotatef(-audience_armSwing, 1.0f, 0.0f, 0.0f);
    //upperaudience_arm
    glRotatef((GLfloat) audience_shoulder, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.75, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.7f, 0.45f, 0.5f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //foreaudience_arm
    glTranslatef(0.75f, 0.0f, 0.0f);
    
    //audience_elbow joint
    glPushMatrix();
    glColor3fv(clothColor[gCurrentMan]);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    // glRotatef((GLfloat)audience_elbow, 0.0f, 0.0f, 1.0f);
    glRotatef(-(GLfloat)audience_elbow, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.4f, 0.35f, 0.4f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //right hand
    glTranslatef(0.65f, 0.0f, 0.0f);
    glPushMatrix();
    glColor3fv(skinColor[gCurrentMan]); // Skin tone
    glScalef(1.2f, 0.8f, 0.6f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    glPopMatrix();

    //left audience_arm
    glPushMatrix();

    glTranslatef(-0.8, -1.0f, 0.0f);
    
    //left audience_shoulder joint
    glPushMatrix();
    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3fv(clothColor[gCurrentMan]); // Skin tone
    gluSphere(quadric, 0.25f, 10, 10);
    glPopMatrix();

    // glRotatef(audience_armSwing, 1.0f, 0.0f, 0.0f);

    //left upperaudience_arm
    glRotatef(-(GLfloat) audience_shoulder, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.75, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.7f, 0.45f, 0.5f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //left foreaudience_arm
    glTranslatef(-0.75f, 0.0f, 0.0f);
    
    //left audience_elbow joint
    glPushMatrix();
    glColor3fv(clothColor[gCurrentMan]);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    // glRotatef(-(GLfloat)audience_elbow, 0.0f, 0.0f, 1.0f);
    glRotatef((GLfloat)audience_elbow, 0.0f, 1.0f, 0.0f);
    glTranslatef(-0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(1.4f, 0.35f, 0.4f);
    gluSphere(quadric, 0.5f, 10, 10);
    glPopMatrix();

    //left hand
    glTranslatef(-0.65f, 0.0f, 0.0f);
    glPushMatrix();
    glColor3fv(skinColor[gCurrentMan]); // Skin tone
    glScalef(1.2f, 0.8f, 0.6f);
    gluSphere(quadric, 0.2f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}


void audience_cube(void)
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


void audience_torso(void)
{
    void audience_cube(void);
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);
    glScalef(0.8f, 1.2f, 0.4f);

    // glColor3f(0.2f, 0.3f, 0.8f);
    glColor3fv(clothColor[gCurrentMan]);
    
    audience_cube();

    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -3.0f, 0.0f);
    glScalef(1.0f, 1.0f, 0.55f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3fv(clothColor[gCurrentMan]);
    gluCylinder(quadric, 0.85f, 0.95f, 1.7f, 20, 20);
    glPopMatrix();
}

void audience_head(void)
{
    //audience_head
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3fv(skinColor[gCurrentMan]); // Skin tone
    gluSphere(quadric, 0.50, 30, 30);
    glPopMatrix();

    //hair
    glPushMatrix();
    glTranslatef(0.0f, 0.06f, -0.08f);
    glColor3fv(hairColor[gCurrentMan]);
    gluSphere(quadric, 0.53f, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glColor3fv(hairColor[gCurrentMan]);
    glTranslatef(0.12f, 0.3f, 0.07f);
    glScalef(1.0f, 0.4f, 0.9f);
    glColor3fv(hairColor[gCurrentMan]);
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

void audience_neck(void)
{
    //audience_neck
    glPushMatrix();

    glTranslatef(0.0f, -0.5f, 0.0f);  // Position audience_neck
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // 

    glColor3fv(skinColor[gCurrentMan]); // Skin tone
    gluCylinder(quadric, 0.15, 0.15, 0.25, 20, 5);

    glPopMatrix();
}


void manModel(void)
{

    void audience_head(void);
    void audience_neck(void);
    void audience_arm(void);
    void audience_torso(void);
    
    audience_head();
    audience_neck();
    audience_arm();
    audience_torso();
}


// For table tea and coffee 
// Cup is built along +Z (the gluCylinder axis), base at z = 0, so rotate -90 about X before calling to stand it up
void teaCup(){
    glPushMatrix();
    glTranslatef(1.2f, 0.0f, 1.0f);  // center = (cylinder radius, half of cylinder height along Z)
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Disk lies in XY, turn it into the XZ plane so it hugs the cylinder wall
    glColor3f(0.85f,0.85f,0.75f); // Tea cup handle - same cream as tea cup
    
    // For the handle
    gluPartialDisk(quadric,
                   0.6f,     // inner radius (hole of the handle)
                   0.8f,     // outer radius  -> thickness = 0.8 - 0.6
                   30,       // slices (smoothness)
                   1,        // loops
                   0.0f,     // start angle (0 = straight up, +Y)
                   180.0f);  // sweep: half circle, bulging out to +X, Like the full 1st and 4th quadrant
    glPopMatrix();


    // Cup
    glColor3f(0.95f,0.95f,0.85f);
    gluCylinder(quadric,1.2f,1.2f,2.0f,20,20);
    gluDisk(quadric,0.0f,1.2f,20,1); // Bottom of the cup
}


void coffeeCup(){
    // Lower part - filled with coffee, so it looks brown
    glColor3f(0.35f,0.18f,0.05f); // Coffee brown
    gluCylinder(quadric,0.76f,1.00f,2.4f,20,20);

    glPushMatrix();
    glTranslatef(0.0f,0.0f,2.4f);
    // Upper part - empty glass above the coffee
    glColor3f(0.8f,0.9f,1.0f); // Light bluish glass
    gluCylinder(quadric,1.0f,1.09f,0.6f,20,20);
    glPopMatrix();

}


int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow){

    // Function declaration 
    int initialize(void);
    void render(void); // Sometimes display, sometimes draw and sometimes render. Linux mai display nhi use kar sakte as display naame ka variable hota h waha joh hum declare karte h. 
    void update(void); // Pudha jana, animation yaha hota h 
    void uninitialize(void);


    // Create log file 
    gpFile = fopen("Log.txt","w");

    // Means fopen fail hogaya
    if(gpFile == NULL){
        // NULL as the window has not been created so we are showing in the desktop
        MessageBox(NULL, TEXT("Log file creation failed!!!"), TEXT("Error"), MB_OK);
        exit(0); // This is after we click on OK on the message box
    }
    else {
        fprintf(gpFile, "SNC - Program Started Successfully!!\n");
    }


    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_Sankalp_Chordia");

    // Game loop variable 
    BOOL bDone = FALSE;

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(MYICON)); // First parameter is hInstance and second one is a macro
    // Mera MYICON ka resource you make int equivalent. MYICON ko int resource bana. MYICON is coming from Window.h
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(MYICON));

    RegisterClassEx(&wndclass);

    // Centering
    // Window - Rumaal
    // Desktop - Tile 
    // We want to find the center of Desktop's width and height 
    // Gives Tile's width
    // CXSCREEN - Screen ka x ka count. Aka Width of the screen or desktop
    int screenWidth = GetSystemMetrics(SM_CXSCREEN); // Gives measurement, but kiska woh apan dete h 
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Give me Height of the screen. Aka tile ka height 


    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW, // Bhai you are window ki extended style. Mjhi window app window ahea - Top most order in the Z. 
        szAppName,
        TEXT("My First RTR Program : Sankalp Nilesh Chordia"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        screenWidth/2 - WIN_WIDTH/2, // X coordinate
        screenHeight/2 - WIN_HEIGHT/2, // Y coordinate
        WIN_WIDTH, // Width 
        WIN_HEIGHT, // Height
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // Set Global Window handle 
    ghwnd = hwnd; // We are assigning it here. Window handle are now in 3 places - ghwnd, hwnd and WndProc ka first parameter. Need of gwnd is that our function is udf function. This funcitno needs hwnd in form of global ghwnd. 

    // We need to call initialize here 
    int iResult = initialize();
    if (iResult != 0) {
        fprintf(gpFile, "SNC : WinMain() :  initialize() failed\n");
        uninitialize(); // Cleans up GL/DC/window and closes the log file
        exit(EXIT_FAILURE);
    }
    else {
        fprintf(gpFile, "SNC : WinMain() :  initialize() succeeded\n");
    }
 

    ShowWindow(hwnd, iCmdShow);

    UpdateWindow(hwnd);

    // We need to call two functions
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // Game loop 
    while(bDone == FALSE){
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            if(msg.message == WM_QUIT){
                bDone = TRUE;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if(bActiveWindow == TRUE){
                if(bEscapeKeyIsPressed == TRUE){
                    bDone = TRUE;
                }
                // Render
                render();

                // Update 
                update();
            }
        }
    }

    // Unintialize
    uninitialize();

    return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,LPARAM lParam){

    // function declarations
    void resize(int, int);
    void uninitialize(void);
    void toggleFullscreen(void);

    // code 
    switch(iMsg) {
        case WM_CREATE : 
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT)); // Fills a block of memory with a repeated byte value. 0 is the byte to write. 
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;
        
        // Focus iss window mai jayega
        // Why no message box ? 
        case WM_SETFOCUS : 
            bActiveWindow = TRUE;
            break;
    
        case WM_KILLFOCUS :
            bActiveWindow = FALSE;
            break;
        
        case WM_SIZE : 
            resize(LOWORD(lParam), HIWORD(lParam));
            break;
        
        // If we press any key still we get WM_KEYDOWN message. We are telling - wmkeydown aat yeh bare, aate yenar apan switch takto h. 
        // Konti key takle hai wparam sangto h, then VK_ESCAPE - VIRTUAL KEY CODE. This code comes when we press esapce key. They we are checking if there is a specific key is pressed or not. 
        // VK_ESCAPE this is the macro of the escape key. Virtual key code matlab ky ? 
        // Escape key -> WM_KEYDOWN -> wparam -> VK_ESCAPE 
        // You can see msdn to see the virtaul key code of any key. 
        // Keydown aata hi h but mujhe janna h ki konsi key h. 
        case WM_KEYDOWN : 
            switch(wParam) 
            {
                case VK_ESCAPE : 
                    bEscapeKeyIsPressed = TRUE;
                    break;
                
                default : 
                    break;
            }

            break; 

        // any alphabetic key if we press we get this. Even numeric key mai bhi aata h 
        // Any key not in A to Z or a to z then we get WM_KEYDOWN. For alphabet we should use WM_CHAR. To detech alphatbet we are handling WM_CHAR.
        // For non alpha - WM_KEYDOWN 
        // For alpha - WM_CHAR
        case WM_CHAR : 
            switch(wParam)
            {
                case 'F' : // if we press 'F'
                case 'f' : // fall through case 

                    if(bFullscreen == FALSE) {
                        toggleFullscreen();
                        bFullscreen = TRUE;
                    }else {
                        toggleFullscreen();
                        bFullscreen = FALSE;
                    }
                    
                    break;
            }
            break;
        
        // case WM_RBUTTONDOWN :
        //     bEscapeKeyPressed = TRUE; 
        //     break;
                
        case WM_CLOSE :
            uninitialize();
            break;

        case WM_DESTROY :
            PostQuitMessage(0); // The parameter here is the message loop ka wparam which was returned. This posts WM_QUIT message to GetMessage
            break;
        default :
            break;
    }
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void toggleFullscreen(void){

    // variable declarations
    MONITORINFO mi;


    // code
    // 2 - Check if the screen is already full screen or not 
    if(bFullscreen == FALSE){

        // 3-  Get current style 
        // GWL - GetWindowLong 
        // handle de 
        // Style deh long madhe
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE); // Mujhe kuch toh chaiye joh long type ka h 

        // 4 - Check if the style contains WS_OVERLAPPEDWINDOW
        // This will check if whatever you have on the right side is present in the left side. The left side is usally more than the right one. 
        if(dwStyle & WS_OVERLAPPEDWINDOW ){

            // memset the mi and set one member 
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);

            if(GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY),&mi)){
                // Iska matlab h ki dono mila h 
                // 5 - Remove WS_OVERLAPPED from current style 
                // Jaha mera ghwnd style ko dekh and usme se WS_OVERLAPPEDWINOW nikal deh
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); // We are using the negation operator (~)

                // 6 - Set window postion 
                /*
                BOOL SetWindowPos(
                [in]           HWND hWnd,
                [in, optional] HWND hWndInsertAfter,
                [in]           int  X,
                [in]           int  Y,
                [in]           int  cx, -> Width -> Right - Left
                [in]           int  cy, ->  height 
                [in]           UINT uFlags
                );
                */
               // HWND_TOP - Aise set kar , ki mera window top mai rakh
                SetWindowPos(ghwnd, 
                    HWND_TOP, 
                    mi.rcMonitor.left, 
                    mi.rcMonitor.top, 
                    mi.rcMonitor.right - mi.rcMonitor.left,  
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    // This is flag 
                    // Frame change 
                    SWP_NOZORDER | SWP_FRAMECHANGED  // Z order jaisa h waisa rakha
                );
            }
        }
        // 7 - Cursor nhi dikhna chaiye fulll screen ma
        ShowCursor(FALSE);
    }
    
    else 
    {
        // 8 - ADD OR BAR LAGA WS_OVERLAPPED
        // | Add
        // &  Check karne
        // ~ negate karne 
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        // 9 - Set Window Placement
        SetWindowPlacement(ghwnd, &wpPrev);

        // 10  - Set Window position 
        // HWND_TOP - This is like z order mai sabse aage
        // Why 0s here as they are ignored and SetWindowPlacement has done its job
        // SWP_NOMOVE - Window pos set kartana, joh window placement ne placement set kiya usko halvu nako
        // SWP_NOSIZE - Window pos set kartana, joh window placement ne size set kiya usko halvu nako
        // SWP_NOZORDER - Current window ki jaga badal maat
        SetWindowPos(ghwnd, HWND_TOP,0 ,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED );


        // 11 
        ShowCursor(TRUE);
    }
    
}



int initialize(void){
    // Funtion declaration 
    void resize(int, int);
    void randomMan(void); // This fills the global array for random man 

    randomMan();

    // variable declaration
    PIXELFORMATDESCRIPTOR pfd; 
    int iPixelFormatIndex ;

    // code
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    // n stands for short
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;  // Convention blindly put 1 
    //The buffer can draw to a window or device surface - PFD_DRAW_TO_WINDOW
    // Pixel format that supports opengl - PFD_SUPPORT_OPENGL
    // PFD_DOUBLEBUFFER - 
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    // Aisa pixel ka type joh rgba hoga
    // For each of these rgba we need 8 wires hence total 32 wires. Means 8 byte. 
    pfd.iPixelType = PFD_TYPE_RGBA;  
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32; // Depth is 2 ^ 32

    ghdc = GetDC(ghwnd);
    if (ghdc == NULL) {
        return -1;
    }

    // Choosing pixel format
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

    // We handle -2 
    if (iPixelFormatIndex == 0){
        return -2;
    }

    // Set pixel format 
    // ghdc - We are telling him to set index 
    // Konsa index 
    // Apna pfd ka address
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE){
        return -3;
    }

    // WGL - Windows Graphics Library - Bridging api between window and opengl 
    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL){
        return -4;
    }

    // ghdc se charge chodke rendering context ko 
    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return -5;
    }

    // Enable depth
    glShadeModel(GL_SMOOTH); // Color smooth deh 
    glClearDepth(1.0f); 
    // Now if we have done all the depth values to 1.0f then how will we decide what is ahead and back 
    glEnable(GL_DEPTH_TEST);
    // We are telling that upar ka test enable karo 
    glDepthFunc(GL_LEQUAL); // Less than or equal to - Mere depth buffer mai sabh 1.0 bhar and then jabh render shuru hoga then do the test of less than equal to test. The ones that pass the test will be 1 ke aage will get values of the depth. The rest will not have values for the depth. 
    /*
    GL_PERSPECTIVE_CORRECTION_HINT - Jaise deste tase - Perspective (Tom and Jerry sidi example). With this comes a lot of artifacts aka problem. We are 
    hinting to correct these problems using this hint 

    GL_NICEST - How to correct perspective ? Utaamatli utaam kar hence NICEST
    
    */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    // Choose screen clearing color as blue
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    quadric = gluNewQuadric();

    // Build the bitmap font display lists (needs ghdc and a current GL context)
    font = glGenLists(256);
    HFONT hFont = CreateFontA(
        -40, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE,
        "Times New Roman"
    );
    HFONT hOldFont = (HFONT)SelectObject(ghdc, hFont);
    wglUseFontBitmapsA(ghdc, 0, 256, font);
    SelectObject(ghdc, hOldFont);
    DeleteObject(hFont);

    // Warm up resize 
    resize(WIN_WIDTH, WIN_HEIGHT);

 
    return (0);
}

void resize(int width, int height){
    // code
    // In future - For perspective, there is w/h and h should not be 0 so checking
    if (height <= 0){
        height = 1;
    }
    //x,y,width, height
    // Left top - 0 
    // View port means we have a binocular , and we are telling binocolor ka left top windows ka left top se match hona chaiye
    gWinWidth = width;
    gWinHeight = height;

    glViewport(0,0,(GLsizei)width,(GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Perspective Projection - Jaisa deste tasa
    // fovy 
    // aspect ration 
    // Near 
    // Far
    gluPerspective(45.0f, ((GLfloat)width/(GLfloat)height), 0.1f, 100.0f);

}

void render(void){
    
    // Function declarations
    void cube(float, float, float);
    void drawStage();
    void manModel(void);
    void teaCup(void);
    void coffeeCup(void);

    // Code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f,1.3f,eyeZ,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);

    glTranslatef(0.0f,-1.0f,-2.0f);
    // Stage 
    int i = 0;
    for(float x = 10.0f; x >= -10.0f; x -= tileWidth, i++){
        glColor3fv(shades[i%4]);
        glBegin(GL_QUADS);
        glVertex3f(x,0.0f,-10.0f); // Right top 
        glVertex3f(x-tileWidth,0.0f,-10.0f); // Left top
        glVertex3f(x-tileWidth,0.0f,1.0f); // Left bottom
        glVertex3f(x,0.0f,1.0f); // Right bottom
        glEnd();
    }

    // Table on stage
    glPushMatrix();// Out push to push the whole table front 

    glTranslatef(0.0f,0.0f,5.0f);

    // Table size - top spans x: 0.1 to 2.9, z: -8.7 to -7.3, top surface at y = 1.0
    // Right edge stays well clear of the fully opened right curtain (inner edge at x = 3.5) from the camera's view
    float tableCenterX = 1.5f;
    float tableHalfWidth = 1.4f;
    float tableHalfDepth = 0.7f;
    float tableTopY = 1.0f;
    float tableTopHalfThick = 0.05f;

    glPushMatrix();
    glTranslatef(tableCenterX,tableTopY - tableTopHalfThick,-8.0f); // Centre of the table top
    glPushMatrix();
    glScalef(tableHalfWidth,tableTopHalfThick,tableHalfDepth);
    glColor3f(0.72f, 0.52f, 0.30f); // Light oak, so it stands out from the dark stage floor
    audience_cube(); // Top of the table
    glPopMatrix();

    // Legs of the table - from the floor (y = 0) up to the underside of the top
    float legHalfHeight = (tableTopY - 2.0f*tableTopHalfThick)/2.0f;
    glColor3f(0.12f, 0.07f, 0.04f); // Dark walnut
    for(int sx = -1; sx <= 1; sx += 2){
        for(int sz = -1; sz <= 1; sz += 2){
            glPushMatrix();
            glTranslatef(sx*(tableHalfWidth - 0.15f),-tableTopHalfThick - legHalfHeight,sz*(tableHalfDepth - 0.15f));
            glScalef(0.08f,legHalfHeight,0.08f);
            audience_cube();
            glPopMatrix();
        }
    }
    glPopMatrix();

    // Tea and coffee
    // 9 tea cups and 1 coffee cup (the last slot, front right)
    // 5 columns 0.55 apart (x: 0.4 to 2.6), 2 rows 0.6 apart (z: -8.3 and -7.7), ~0.3 clear gap between cups
    for(int k = 0; k < 10; k++){
        int col = k % 5;
        int row = k / 5;
        glPushMatrix();
        glTranslatef(tableCenterX + (col - 2)*0.55f, tableTopY, -8.3f + row*0.6f);
        glRotatef(-90.0f,1.0f,0.0f,0.0f); // Stand the cup up: its Z axis now points along +Y
        glScalef(0.08f,0.08f,0.08f); // Tea cup: radius 0.096, height 0.16 | Coffee cup: radius 0.087, height 0.24
        if(k == 9)
            coffeeCup();
        else
            teaCup();
        glPopMatrix();
    }

    glPopMatrix(); // Outermost pop for the table


    // Front of the stage
    glBegin(GL_QUADS);
    glColor3f(0.18f,0.09f,0.04f);
    glVertex3f(10.0f,0.0f,1.0f); // Right top
    glColor3f(0.18f,0.09f,0.04f);
    glVertex3f(-10.0f,0.0f,1.0f); // Left top 
    glColor3f(0.08f,0.04f,0.02f);
    glVertex3f(-10.0f,-1.5f,1.0f); // Left bottom
    glColor3f(0.08f,0.04f,0.02f);
    glVertex3f(10.0f,-1.5f,1.0f); // Right bottom
    glEnd();

    // Left pillar
    glPushMatrix();
    glTranslatef(-7.0f,0.0f,0.6f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f); // We need to rotate it first as originally it lies along the z axis.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.20f, 0.10f, 0.05f);
    gluCylinder(quadric, 0.5, 0.5, 4.0, 32, 1); // quadric, base radius, top radius, height, slices, stacks
    glPopMatrix();


    // Right pillar 
    glPushMatrix();
    glTranslatef(7.0f,0.0f,0.6f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f); // We need to rotate it first as originally it lies along the z axis.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.20f, 0.10f, 0.05f);
    gluCylinder(quadric, 0.5, 0.5, 4.0, 32, 1); // quadric, base radius, top radius, height, slices, stacks
    glPopMatrix();


    // Top of the pillar 
    glPushMatrix();
    glTranslatef(0.0,4.0f,0.6f);
    glBegin(GL_QUADS);
    glColor3f(0.20f, 0.10f, 0.05f);
    glVertex3f(10.0f,1.0f,1.0f); // Right top
    glVertex3f(-10.0f,1.0f,1.0f); // Left top 
    glVertex3f(-10.0f,-0.5f,1.0f); // Left bottom
    glVertex3f(10.0f,-0.5f,1.0f); // Right bottom
    glEnd();
    // Movie name written on the beam. Depth test is off only for the text (its pixels have a single depth),
    // the beam has already written its depth so the curtains behind it still cannot draw over the text.
    glDisable(GL_DEPTH_TEST);
    MyDrawText3D("PRATIBIMBA", 0.0f, -0.3f, 1.05f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    // Top curtains
    glPushMatrix();
    glScalef(1.2f,1.0,1.0f);
    for(float f = -2.5f; f<=2.5f; f+=1){
        glBegin(GL_TRIANGLE_FAN);
        {
            glColor3f(0.65f, 0.08f, 0.10f);
            glVertex3f(f,4.0f,0.3f);
            glColor3f(0.30f, 0.02f, 0.04f);
            for(int i = 0; i<= 360; i++){
                float angleInRadian = i * M_PI/180.0f;
                float x = 0.6f * cos(angleInRadian) + f;
                float y = 0.6f * sin(angleInRadian) + 4.0f;
                glVertex3f(x, y, 0.3f);
            }
        }
        glEnd();
    }glPopMatrix();

    // Left curtains
    drawCurtain(-8.0f, leftCurtainXRight, 4.0f, 0.0f, 0.25f, 12.0f, curtainPhase);

    // Right curtains
    drawCurtain(rightCurtainXLeft, 8.0f, 4.0f, 0.0f, 0.25f, 12.0f, curtainPhase);

    // Seats 
    int man =0;
    for(float x = -5.0f ; x <= 5.0f; x+=0.8, man++){
        // Audience member sitting in front of the seat back
        glPushMatrix();
        glTranslatef(x, -0.35f, 3.0f);   // to the seat, head just above the backrest
        glScalef(0.25f, 0.25f, 0.25f); 
        // Audience to face the stage 
        glRotatef(180.0f,0.0f,1.0f,0.0f);
        gCurrentMan = man;
        manModel();
        glPopMatrix();
        glPushMatrix();
        cube(x,-1.0f,3.2f);
        glPopMatrix();
    }


    if(leftCurtainXRight == -3.5f){
        glPushMatrix();
        glTranslatef(boardX,2.0f,-2.0f);
        brownFrame();
        // Text written on the board (follows it as it slides)
        // Depth test stays on so the curtain hides the text. Bitmap text has one depth for all its pixels while
        // the board's depth varies across it, so the text is lifted off the board (still behind the curtain).
        MyDrawText3D("The Death of", 0.0f, 0.4f, 1.45f, 1.0f, 1.0f, 1.0f);
        MyDrawText3D("Conquerer Hitler", 0.0f, 0.0f, 1.45f, 1.0f, 1.0f, 1.0f);
        glPopMatrix();

        // Second board enters once the first one has left
        if(boardX <= -8.0f){
            // The board is behind the right curtain, so it comes out from the curtain's inner edge
            glPushMatrix();
            glTranslatef(board2X,2.0f,-8.0f);
            brownFrame();
            MyDrawText3D("The Escape", 0.0f, 0.4f, 1.45f, 1.0f, 1.0f, 1.0f);
            MyDrawText3D("- An Ekanki", 0.0f, 0.0f, 1.45f, 1.0f, 1.0f, 1.0f);
            glPopMatrix();
        }
    }

    SwapBuffers(ghdc);
}

void update(void){
    // code 

    // Zoom out first; clamp so eyeZ lands exactly on the target and the zoom is "done"
    if(eyeZ < 8.0f){
        eyeZ += 0.0007f;
        if(eyeZ > 8.0f){
            eyeZ = 8.0f;
        }
    }
    BOOL zoomDone = (eyeZ >= 6.0f);

    // Boards start moving only after the zoom out has finished
    if(zoomDone){
        if(boardX > -8.0f){
            boardX -= 0.00088f;
        }
        else if(board2X > -8.0f){
            board2X -= 0.00088f; 
            if(board2X <= -8.0f){
                board2X = -8.0f;
            }
        }
    }

    if (leftCurtainXRight > -3.5f){
        leftCurtainXRight -= 0.00045f;
    }
    if(leftCurtainXRight < -3.5f){
        leftCurtainXRight = -3.5f;
    }
    if(rightCurtainXLeft < 4.5f){
        rightCurtainXLeft += 0.00045f;
    }
    if(rightCurtainXLeft > 3.5f){
        rightCurtainXLeft = 3.5f;
    }

}

void uninitialize(void){
    // function declarations
    void toggleFullscreen(void);

    // code

    // If in full screen then first restore then proceed
    if(bFullscreen){
        toggleFullscreen();
        bFullscreen = FALSE;
    }

    // Check the current context and if it is present then unmake it and then destroy
    // Release Rendering Context
    if(wglGetCurrentContext() == ghrc){
        // Delete font display lists while the context is still current
        if(font){
            glDeleteLists(font, 256);
            font = 0;
        }

        wglMakeCurrent(NULL, NULL);

        // Now destroy 
        if(ghrc){
            wglDeleteContext(ghrc);
            ghrc = NULL;
        }
    }

    // Release Device Context
    if(ghdc){
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // Destroy Window
    if(ghwnd){
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Closing quadric 
    if(quadric) {
        gluDeleteQuadric(quadric);
        quadric = NULL;
    }

    // Close the log file 
    if(gpFile){
        fprintf(gpFile, "SNC - Program Terminated Successfully !!!\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

