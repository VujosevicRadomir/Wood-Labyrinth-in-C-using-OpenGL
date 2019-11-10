#include<stdlib.h>
#include<GL/glut.h>
#include<stdbool.h>
#include<stdio.h>
#include<math.h>
#include<sys/types.h>
#include<unistd.h>

#define EPSILON 0.0001
#define PI 3.141592653589793

#define TIMER_ID 0
#define TIMER_INTERVAL 10

static void on_keyboard(unsigned char, int ,int);
static void on_keyboard_up(unsigned char, int, int);
static void on_reshape(int, int);
static void on_display(void);

typedef struct{
    double x, y, z;
}v3;

v3 ballLocation;
v3 ballSpeed;
v3 *holes;

typedef struct{
    double x1, z1, x2, z2;
    bool isVertical;
}wall;

wall *walls;
bool wallsAdded = 0;
bool holesAdded = 0;
bool ballInHole = 0;


void on_timer(int);
void initMaterials();
void initLights();
void initVariables();
void drawPlane();
void freeMemory();

int animation_ongoing;
int t;
int dimension;
int NumberOfWalls = 0;
int NumberOfHoles = 0;

double camera_distance = 50;
double planeXAngle = 0;
double planeYAngle = 0;
double maximumPlaneRotation = 15;
double speed = 0.02;
double wallThickness = 0.5;
double wallHeight = 2;
double rotationSpeed = 0.5;
double horizontalBounceTimer = 0;
double verticalBounceTimer = 0;
double bounciness = 0.2;
double minBallSpeed = 0.005;
double friction = 0.005;


double planeYRotation = 0;
double planeXRotation = 0;

int main(int argc, char **argv){
    
    
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(argv[0]);

	glutKeyboardFunc(on_keyboard);
    glutKeyboardUpFunc(on_keyboard_up);
	glutReshapeFunc(on_reshape);
	glutDisplayFunc(on_display);
    
    glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);

	glClearColor(0.8, 0.8, 0.6, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
    
    initVariables();
    initLights();

	

	glutMainLoop();

	return 0;
}

double dabs(double expr){
    return expr >= 0 ? expr : -expr; 
}

static void on_keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27:
                freeMemory();
				exit(EXIT_SUCCESS);
				break;
		case 'r':
		case 'R':
				t = 0;
				glutPostRedisplay();
                ballLocation.x = ballLocation.z = dimension - 1;
                ballLocation.y = 0;
                planeYAngle = planeXAngle = 0;
                ballSpeed.x = ballSpeed.y = ballSpeed.z = 0;
                ballInHole = false;
				break;
        case 'D':
        case 'd':
                planeYRotation = -1;
                break;
                
        case 'A':
        case 'a':
                planeYRotation = 1;
                break;
        case 'W':
        case 'w':
                planeXRotation = -1;
                break;
        case 'S':
        case 's':
                planeXRotation = 1;
                break;
	}
}

void on_keyboard_up(unsigned char key, int x, int y){
    switch(key){
        case 'w':
        case 'W':
        case 's':
        case 'S':
                planeXRotation = 0;
                break;
        case 'a':
        case 'A':
        case 'D':
        case 'd':
                planeYRotation = 0;
                break;
    }
}

static void on_reshape(int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)(width)/height, 1, 1000);
}

void initVariables(){
    animation_ongoing = 0;
    t = 0;
    dimension = 20;
    
    
    ballLocation.x = dimension - 1;
    ballLocation.y = 1;
    ballLocation.z = dimension - 1;
    
    walls = malloc(sizeof (*walls) * 50);
    holes = malloc(sizeof (*holes) * 50);
}

void freeMemory(){
    free(holes);
    free(walls);
}


void initLights(){
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat light_position [] = {1, 1, 1, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_ambient [] = {0.1, 0.1, 0.1, 1};
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	GLfloat light_specular [] = {0.9, 0.9, 0.9, 1};
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular); 

	GLfloat light_diffuse [] = {0.7, 0.7, 0.7, 1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse); 
}

double sinDegree(double degree){
    double rads = degree * PI/180;
    return sin(rads);
}

double min(double a, double b){
    return a > b ? b : a;
}

double max(double a, double b){
    return a > b ? a : b;
}

bool checkVerticalWallCollision(int i){
    if(!walls[i].isVertical) return false;
    
    double begin = min(walls[i].z1, walls[i].z2);
    double end = max(walls[i].z1, walls[i].z2);
    
    if((begin <= ballLocation.z) & ( ballLocation.z <= end)
    & (dabs(walls[i].x1 - ballLocation.x) < ( 1 + wallThickness))
    & (ballSpeed.x * (ballLocation.x - walls[i].x1) < 0)){
        ballSpeed.x = -ballSpeed.x * bounciness;
        verticalBounceTimer = 5;
        return true;
    }
    return false;
}

bool checkHorizontalWallCollision(int i){
    if(walls[i].isVertical) return false;
    
    double begin = min(walls[i].x1, walls[i].x2);
    double end = max(walls[i].x1, walls[i].x2);
    
    if((begin < ballLocation.x) & (end  > ballLocation.x)
    &(dabs(walls[i].z1 - ballLocation.z) < ( 1 + wallThickness))
    &(ballSpeed.z * (ballLocation.z - walls[i].z1) < 0)){
        ballSpeed.z = -ballSpeed.z * bounciness;
        horizontalBounceTimer = 5;
        return true;
    }
    return false;
}

double XZpointDistance(v3 begin, v3 end){
    return sqrt((begin.x - end.x) * (begin.x - end.x) + (begin.z - end.z) * (begin.z - end.z));
}

void dropBallInHole(int i){
    
    ballLocation.x = holes[i].x;
    ballLocation.z = holes[i].z;
    ballLocation.y = -0.7;
}

void checkHoleCollision(int i){
    if(XZpointDistance(ballLocation, holes[i]) < 1.5){
       ballInHole = true;
       dropBallInHole(i);
    }
}

void checkGoalCollision(int i){
    
}

void checkForCollision(){
    int i;
    for(i = 0; i < NumberOfWalls; i++){
        if(walls[i].isVertical & checkVerticalWallCollision(i)) return ;
        else if(checkHorizontalWallCollision(i)) return;
    }
    
    for(i = 0; i < NumberOfHoles; i++){
        checkHoleCollision(i);
    }
    
}

void moveBall(){
    
    checkForCollision();
    
    if(ballInHole) return ;
    
    if(horizontalBounceTimer <= 0){
        ballSpeed.z += sinDegree(planeXAngle) * speed;
    }else{
        /*if(ballSpeed.z < minBallSpeed) ballSpeed.z = 0;*/
    }
    
    if(verticalBounceTimer <= 0){
        ballSpeed.x -= sinDegree(planeYAngle) * speed;
    }else{
       /* if(ballSpeed.x < minBallSpeed) ballSpeed.x = 0; */
    }
    
    ballLocation.z += ballSpeed.z;
    ballLocation.x += ballSpeed.x;
}

void on_timer(int value){
	if(value != TIMER_ID) return ;
	t+= 1;
    moveBall();
    glutPostRedisplay();
    
    if(horizontalBounceTimer > 0)
        horizontalBounceTimer --;
    if(verticalBounceTimer > 0)
        verticalBounceTimer --;
    
    glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
}

void planeColor(){
	GLfloat material_ambient [] = {0.3, 0.3, 0.7, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);

	GLfloat material_diffuse [] = {1, 1, 0, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	GLfloat material_specular [] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);

	GLfloat material_shininess [] = {30};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
}

double clamp(double x, double min, double max){
    if(x < min) return min;
    if(x > max) return max;
    return x;
}

void rotatePlane(){
    if(planeXRotation == 1){
        planeXAngle += rotationSpeed;
    }else if(planeXRotation == -1){
        planeXAngle -= rotationSpeed;
    }
    if(planeYRotation == 1){
        planeYAngle += rotationSpeed ;
    }else if(planeYRotation == -1){
        planeYAngle -= rotationSpeed;;
    }
    planeYAngle = clamp(planeYAngle, -maximumPlaneRotation, maximumPlaneRotation);
    planeXAngle = clamp(planeXAngle, -maximumPlaneRotation, maximumPlaneRotation);
    
   
    
    glRotatef(planeXAngle, 1, 0, 0);
    glRotatef(planeYAngle, 0, 0, 1);
}

void drawPlane(){

    
    planeColor();
    rotatePlane();
    glBegin(GL_POLYGON);
        glNormal3f(0, 1, 0);
        glVertex3f(-dimension, 0, -dimension);
        glVertex3f(-dimension, 0, dimension);
        glVertex3f(dimension, 0, dimension);
        glVertex3f(dimension, 0, -dimension);
    glEnd();
    
    
}

void ballColor(){
	GLfloat material_ambient [] = {0.7, 0.7, 0.7, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);

	GLfloat material_diffuse [] = {0.3, 0.3, 0.3, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	GLfloat material_specular [] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);

	GLfloat material_shininess [] = {30};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
}

void drawBall(){
    glPushMatrix();
    glTranslatef(ballLocation.x, ballLocation.y, ballLocation.z);
    ballColor();
    glutSolidSphere(1, 16, 16);
    
    glPopMatrix();
}

void holeColor(){
    GLfloat material_ambient [] = {0, 0, 0, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);

	GLfloat material_diffuse [] = {0, 0, 0, 1};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);

	GLfloat material_specular [] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);

	GLfloat material_shininess [] = {30};
	glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
}

void goalColor(){
    GLfloat material_ambient [] = {1, 0.2, 0.2, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);

	GLfloat material_diffuse [] = {0.3, 0.3, 0.3, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

	GLfloat material_specular [] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);

	GLfloat material_shininess [] = {30};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
}


void drawGoal(double x, double z){
    glPushMatrix();
    
    glTranslatef(x, 0.1, z);
    goalColor();
    double i;
    glBegin(GL_POLYGON);
        for(i = 0; i < 2 * PI; i+= PI/10){
            glVertex3f(1.5 * sin(i), 0, 1.5 * cos(i)); 
        }
    glEnd();
    glPopMatrix();
    
}


void drawHole(double x, double z){
    glPushMatrix();
    
    glTranslatef(x, 0.1, z);
    holeColor();
    double i;
    glBegin(GL_POLYGON);
        for(i = 0; i < 2 * PI; i+= PI/10){
            glVertex3f(1.5 * sin(i), 0, 1.5 * cos(i)); 
        }
    glEnd();
    if(!holesAdded){
        holes[NumberOfHoles].x = x;
        holes[NumberOfHoles].y = 0;
        holes[NumberOfHoles].z = z;
        NumberOfHoles++;
    }
    glPopMatrix();
}

void drawWall(double x1, double z1, double x2, double z2){
   
    
    double isVertical = 0 ;
    double isHorizontal = 0;
    
    if(x1 == x2) isVertical = wallThickness;
    else if(z1 == z2) isHorizontal = wallThickness;
    else{
        perror("Please only use vertical or horizontal walls");
    }
    
    
    glPushMatrix();
    if(isHorizontal > 0){
        glRotatef(90, 0, 1, 0);
    }
    glPopMatrix();
    ballColor();
    
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    
    glVertex3f(x1, 0, z1);
    glVertex3f(x1 + wallThickness, 0, z1);
    glVertex3f(x2 + wallThickness, 0, z2);
    glVertex3f(x2, 0, z2);
    
    glVertex3f(x1, wallHeight, z1);
    glVertex3f(x1 + wallThickness, wallHeight, z1);
    glVertex3f(x2 + wallThickness, wallHeight, z2);
    glVertex3f(x2, wallHeight, z2);
    
    
    glVertex3f(x1, 0, z1);
    glVertex3f(x1 + wallThickness, 0, z1);
    glVertex3f(x1 + wallThickness, wallHeight, z1);
    glVertex3f(x1, wallHeight, z1);
    
    glVertex3f(x2, 0, z2);
    glVertex3f(x2 + wallThickness, 0, z2);
    glVertex3f(x2 + wallThickness, wallHeight, z2);
    glVertex3f(x2, wallHeight, z2);
    
    glVertex3f(x1, 0, z1);
    glVertex3f(x1, wallHeight, z1);
    glVertex3f(x2, wallHeight, z2);
    glVertex3f(x2, 0, z2);
    
    glVertex3f(x1 + wallThickness, 0, z1);
    glVertex3f(x1 + wallThickness, wallHeight, z1);
    glVertex3f(x2 + wallThickness, wallHeight, z2);
    glVertex3f(x2 + wallThickness, 0, z2);
    
    
    glEnd();
    
    
    
    if(!wallsAdded){
        walls[NumberOfWalls].x1 = x1;
        walls[NumberOfWalls].z1 = z1;
        walls[NumberOfWalls].x2 = x2;
        walls[NumberOfWalls].z2 = z2;
        walls[NumberOfWalls].isVertical = isVertical > 0 ? true : false;
        NumberOfWalls++;
    }
}

void drawBox(){
    glPushMatrix();
    drawPlane();
    drawBall();
    
    
    drawWall(-dimension, -dimension, -dimension, dimension);
    drawWall(-dimension, dimension, dimension, dimension);
    drawWall(dimension, dimension, dimension, -dimension);
    drawWall(dimension, -dimension, -dimension, -dimension);
    
    
    drawWall(15, 20, 15, 10);
    drawWall(20, 3, 10, 3);
    drawWall(10, 3, 10, 15);
    drawWall(0, 20, 0, 0);
    drawWall(0, 0, 13, 0);
    drawWall(20, -5, 6, -5);
    drawWall(20, -5, 6, -5);
    drawWall(0, 0, 0, -10);
    drawWall(0, -10, 15, -10);
    drawWall(10, -15, 10, -10);
    drawWall(15, -13, 15, -17);
    drawWall(5, -20, 5, -15);
    drawWall(5, -15, -5, -15);
    drawWall(-5, -15, -5, -6);
    drawWall(-5, -8, -14, -8);
    drawWall(-14, -8, -14, 3);
    drawWall(-14, 3, -5, 3);
    drawWall(0, -2, -6, -2);
    drawWall(-10, 3, -10, 15);
    drawWall(-10, 15, -15, 15);
    drawWall(-14, 9, -14 , 6);
    drawWall(-13, -20, -13, -14);

    wallsAdded = true;
    
    drawHole(18, 5);
    drawHole(12, 5);
    drawHole(6, 18);
    drawHole(2, 18);
    drawHole(8, 11);
    drawHole(2, 2);
    drawHole(2, 7);
    drawHole(18, 1);
    drawHole(4, -5);
    drawHole(18.5, -18.5);
    drawHole(11.5, -11.5);
    drawHole(10, -16);
    drawHole(8.5, -11.5);
    drawHole(-3, -13.5);
    drawHole(-8, -2);
    drawHole(-8, 5);
    drawHole(-2, 9);
    drawHole(-2, 18);
    drawHole(-8, 14);
    drawHole(-18, 18);
    drawHole(-12.5, 13.5);
    drawHole(-18.5, 9);
    drawHole(-16.5, -9);
    drawHole(-18.5, -18);
    drawHole(-14.5, -18);
    drawHole(-7, -10);
    drawHole(-12, -18);
    
    drawGoal(3, -18);
    
    holesAdded = true;
    
    glPopMatrix();
}



static void on_display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
    /*Glavno podesavnje za kameru. Kamera koja prati loptu.
     */
	gluLookAt(ballLocation.x, camera_distance, ballLocation.z + camera_distance,
              ballLocation.x, 0 , ballLocation.z,
              0, 1, 0);
  
    
    /*Debug kamera koja sluzi samo za testiranje
     */
    /*gluLookAt(0, 3*camera_distance, 3*camera_distance, 0, 0, 0, 0, 1, 0);
    */
    
	drawBox();
	
    
	glutSwapBuffers();
}
