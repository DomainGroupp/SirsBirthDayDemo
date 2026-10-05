#define _USE_MATH_DEFINES 1
#ifdef __APPLE__
	#include<GLUT/glut.h>
	#include<cstdlib>
	static inline void glutLeaveMainLoop() {exit(0);}
	static inline void glutLeaveFullScreen() {}
	static inline void glutCloseFunc(void (*)(void)){}
#else 
	#include <GL/freeglut.h>
#endif
#include <stdlib.h>
#include<math.h>
#include<stdio.h>

GLUquadric* quad = NULL;


void drawTree();
void drawCircle(float radius);

void resize(int width, int height){
    if(height<=0){
        height = 1;
    }

    glViewport(0,0,GLsizei(width), GLsizei(height));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat(width)/ GLfloat(height)), 0.1f, 100.0f);
}


void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 2.0, 10.0,    // above and in front
          0.0, 0.5, 0.0,    
          0.0, 1.0, 0.0);
    drawTree();
    glutSwapBuffers();
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    quad = gluNewQuadric();
    glutCreateWindow("Scene");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutMainLoop();
    return (0);
}


void drawTree(){

    // Circle for the tree
    float radiusCircle = 0.12f;
    glColor3f(0.60f, 0.38f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3f(0.0f,0.0f,0.0f);
        for(int i = 0; i<= 360; i++){
            float angleInRadian = i * M_PI/180.0f;
            float x = radiusCircle * cos(angleInRadian) ;
            float z = radiusCircle * sin(angleInRadian);
            glVertex3f(x, 0.0f, z);
        }
    }
    glEnd();

    // Trunk of the tree using cylinder
    int step = 60;
    float heightOfTrunk = 2.0f;
    glBegin(GL_QUADS);
    {
        for(int i = 0; i<=360; i++){
            float angleInRadian1 = i * (M_PI/180.0f);
            float angleInRadian2 = (i+step) * (M_PI/180.0f);
            glVertex3f(radiusCircle * cos(angleInRadian1), 0.0f, radiusCircle * sin(angleInRadian1));
            glVertex3f(radiusCircle * cos(angleInRadian2), 0.0f, radiusCircle * sin(angleInRadian2));
            glVertex3f(radiusCircle * cos(angleInRadian1), heightOfTrunk, radiusCircle * sin(angleInRadian1));
            glVertex3f(radiusCircle * cos(angleInRadian2), heightOfTrunk, radiusCircle * sin(angleInRadian2));
        }
    }
    glEnd();

    // Leaves of the tree using solid sphere
    float radiusSphere = 0.5f;
    int slices = 50;
    int stacks = 50;
    // Why translate by 2.0f - As the  heightOfTrunk = 2.0f and the leaves have to be above it. 
    glPushMatrix();
    glTranslatef(-0.5f,2.0f,0.0f);
    glColor3f(0.30f, 0.65f, 0.25f);   
    glutSolidSphere(radiusSphere, slices, stacks);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.5f,2.49f,0.0f);
    glColor3f(0.30f, 0.65f, 0.25f);   
    glutSolidSphere(radiusSphere, slices, stacks);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.5f,2.0f,0.0f);
    glColor3f(0.30f, 0.65f, 0.25f);   
    glutSolidSphere(radiusSphere, slices, stacks);
    glPopMatrix();

        
    glPushMatrix();
    glTranslatef(0.5f,2.49f,0.0f);
    glColor3f(0.30f, 0.65f, 0.25f);   
    glutSolidSphere(radiusSphere, slices, stacks);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(0.0f,2.8f,0.0f);
    glColor3f(0.30f, 0.65f, 0.25f);  
    glutSolidSphere(radiusSphere+0.4f, slices, stacks);
    glPopMatrix();

}

// The radius of the circle and at what height I want to draw the circle
void drawCircle(float radius, float height){
    glColor3f(0.60f, 0.38f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3f(0.0f,0.0f,0.0f);
        for(int i = 0; i<= 360; i++){
            float angleInRadian = i * M_PI/180.0f;
            float x = radius * cos(angleInRadian) ;
            float z = radius * sin(angleInRadian);
            glVertex3f(x, height, z);
        }
    }
    glEnd();

}

void drawTrunk(float radius, int step, float height){
    glBegin(GL_QUADS);
    {
        for(int i = 0; i<=360; i++){
            float angleInRadian1 = i * (M_PI/180.0f);
            float angleInRadian2 = (i+step) * (M_PI/180.0f);
            glVertex3f(radius * cos(angleInRadian1), 0.0f, radius * sin(angleInRadian1));
            glVertex3f(radius * cos(angleInRadian2), 0.0f, radius * sin(angleInRadian2));
            glVertex3f(radius * cos(angleInRadian1), height, radius * sin(angleInRadian1));
            glVertex3f(radius * cos(angleInRadian2), height, radius * sin(angleInRadian2));
        }
    }
    glEnd();
}

