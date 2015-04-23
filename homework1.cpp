//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <sys/time.h>

//#include "ppm.h"
//#include "log.h"
extern "C" {
	#include "fonts.h"
}

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 4000
#define GRAVITY 0.15

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
    Vec velocity;
    bool isMovable;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    bool bubbler;
    Shape box[5];
    Shape circle;
    Particle * particle;
    Particle * shot;
    int n;
    int nShot;
    ~Game() {delete [] particle;}
    Game(){
        n=0;
        nShot = 0;
        particle = new Particle[MAX_PARTICLES];
        shot = new Particle[MAX_PARTICLES];
        //declare a box shape
        for(int i = 0; i < 5; i++){
            box[i].width = 70;
            box[i].height = 10;
            box[i].center.x = 85 + i*55;
            box[i].center.z = 0;
            box[i].center.y = 320 - i*50;
            box[i].isMovable = 0;
        }
        //declare a circle shape
        circle.center.x = 410;
        circle.center.z = 0;
        circle.center.y = -50;
        circle.radius = 130;
        circle.velocity.x = 0.4;
	bubbler = false;
    }

};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e);
void movement(Game *game);
void render(Game *game);

int checkMoveKey(XEvent *e, Game *game);
int checkShootKey(XEvent *e);
void makeShot(Game *game, int x, int y);
int upPressed;
int downPressed;
int leftPressed;
int rightPressed;
int shootPressed;

struct timeval threshold;
int oldMilliSec = 0;
int timeLapse = 0;
int thresh = 20000;


int main(void)
{
    upPressed = 0;
    downPressed = 0;
    leftPressed = 0;
    rightPressed = 0;
    shootPressed = 0;
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object


    Game game;
    while(!done) {

        gettimeofday(&threshold,NULL);
        timeLapse = (threshold.tv_usec >= oldMilliSec) ? threshold.tv_usec - oldMilliSec 
            : (1000000 - oldMilliSec) + threshold.tv_usec;

        if (timeLapse < thresh){
    
        
        }
        else{
        oldMilliSec = threshold.tv_usec;
        
    while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e);
            checkMoveKey(&e, &game);
            checkShootKey(&e);
        }
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}
#define rnd() (float) rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5;
    p->velocity.x = rnd() + 0.2;
    game->n++;
}
void makeShot(Game *game, int x, int y){

    Particle *p = &game->shot[game->nShot];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = 2;
    p->velocity.x = 0;
    game->nShot++;

}
void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            int y = WINDOW_HEIGHT - e->xbutton.y;
            for (int i = 0; i < 10; i++)
                makeParticle(game, e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        for (int j = 0; j < 5; j++){
            if (game->box[j].isMovable == 1){
                game->box[j].center.x = e->xbutton.x;
                game->box[j].center.y = 600 - e->xbutton.y;
            }
        }
        if (++n < 10)
            return;
   //     int y = WINDOW_HEIGHT - e->xbutton.y;
   //     for (int i = 0; i < 10; i++)
   //         makeParticle(game, e->xbutton.x, y);
    }
}
int checkShootKey(XEvent *e){
    if (e->type == KeyPress){
        int key = XLookupKeysym(&e->xkey,0);
        if (key == XK_w){
            shootPressed = 1;
        }
    }
    if (e->type == KeyRelease){
        int key = XLookupKeysym(&e->xkey,0);
        if (key == XK_w){
            shootPressed = 0;
        }
    }
    return 0;
}
int checkMoveKey(XEvent *e, Game * game){

    if (e->type == KeyPress){
        int key = XLookupKeysym(&e->xkey,0);
	if (key == XK_b){
	    if(game->bubbler == false)
		game->bubbler = true;
	    else
		game->bubbler = false;
	}
        if (key == XK_e){
            thresh += 5000;
        }
        if (key == XK_q){
            thresh -= 5000;
        }
        if (key == XK_Up){
            upPressed = 1;
        }
        if (key == XK_Down){
            downPressed = 1;
        }
        if (key == XK_Left){
            leftPressed = 1;
        }
        if (key == XK_Right){
            rightPressed = 1;
        }
        if (key == XK_Return){
            
            for (int j = 0; j < 5; j++){
                if ( e->xbutton.x >= game->box[j].center.x - game->box[j].width &&
                e->xbutton.x <= game->box[j].center.x + game->box[j].width &&
                600 - e->xbutton.y < game->box[j].center.y + game->box[j].height &&
                600 - e->xbutton.y > game->box[j].center.y - game->box[j].height){
                
                    (game->box[j].isMovable == 1)? game->box[j].isMovable = 0: 
                        game->box[j].isMovable = 1;
                }
            }
        }

    }
    if (e->type == KeyRelease){
        int key = XLookupKeysym(&e->xkey,0);
        if ( key == XK_Up){
            upPressed = 0;
        }
        if ( key == XK_Down){
            downPressed = 0;
        }
        if ( key == XK_Left){
            leftPressed = 0;
        }
        if ( key == XK_Right){
            rightPressed = 0;
        }
    }
    return 0;


}

int check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.


    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;
    if (game->circle.center.x >= 430) 
        game->circle.velocity.x =  -0.4;
    if (game->circle.center.x <= 390)
        game->circle.velocity.x = 0.4;
    game->circle.center.x += game->circle.velocity.x;
    if (game->bubbler == true){
    	for (int i = 0; i < 5; i++)
	    makeParticle(game, 100,358);
    }
    if (game->n <= 0 && game->nShot <= 0)
        return;

    Particle *shot;
    for (int i = 0; i < game->nShot; i++){

        shot = &game->shot[i];
        for (int j = 0; j < game->n; j++){
 
            p = &game->particle[j];
            
            if ( p->s.center.x >= shot->s.center.x - 6 &&
                    p->s.center.x <= shot->s.center.x + 6 &&
                    p->s.center.y < shot->s.center.y + 6 &&
                    p->s.center.y > shot->s.center.y - 6){
            
                p->velocity.y = (rnd() - 0.5) * 6;
                p->velocity.x = (rnd() - 0.5) * 6;
                shot->velocity.y = (rnd() - 1) * 6;
                shot->velocity.x = (rnd() - 0.5) * 6;
                
            }


        }
        shot->velocity.y -= 0.2;
        shot->s.center.y += 10 + shot->velocity.y;
        shot->s.center.x += shot->velocity.x;
        if (shot->s.center.y > 600) {
            memcpy(&game->shot[i], &game->shot[game->nShot-1], sizeof(Particle));
            //std::cout << "off screen" << std::endl;
            game->nShot--;
        }
    

    }
    for (int i=0; i < game->n; i++){
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;

        for (int j = 0; j <5; j++){
            if ( p->s.center.x >= game->box[j].center.x - game->box[j].width &&
                    p->s.center.x <= game->box[j].center.x + game->box[j].width &&
                    p->s.center.y-2 < game->box[j].center.y + game->box[j].height &&
                    p->s.center.y > game->box[j].center.y - game->box[j].height){

                p->s.center.y = game->box[j].center.y + game->box[j].height + 0.1;
                p->velocity.y *= rnd() * -0.5;
            }
        }

        if ( pow(game->circle.center.x - p->s.center.x,2) + 
                pow(game->circle.center.y - p->s.center.y,2) <= 
                pow(game->circle.radius,2)){

            p->s.center.y = sqrt(pow(game->circle.radius,2) - 
                    pow(game->circle.center.x - p->s.center.x,2)) + 
                game->circle.center.y + 0.1 ;

            p->velocity.y = pow( ((p->s.center.y - game->circle.center.y) / game->circle.radius),8) * 
                (p->velocity.y/5) * -1;
            if ((p->s.center.y - game->circle.center.y) < 100 )
                p->velocity.y = (100 - (p->s.center.y - game->circle.center.y)) * -0.1;
            p->velocity.x = (p->velocity.x/2) + 
                ((p->s.center.x - game->circle.center.x) / game->circle.radius) * 2;

        }

        //check for collision with shapes...

        if (p->s.center.y < 0.0) {
            memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
            //std::cout << "off screen" << std::endl;
            game->n--;
        }

        /*
        Shape *s;
        int k = 0;
        while ( k < game->n && game->n > 1){
            s = &game->particle[k].s;
            //check for off-screen
            if (s->center.y < 0.0) {
                memcpy(&game->particle[k], &game->particle[game->n-1], sizeof(Particle));
                std::cout << "off screen" << std::endl;
                game->n--;
                continue;
            }
            k++;
        }*/
    }
}

void DrawCircle(float cx, float cy, float r, int num_segments) 
{
    glPushMatrix(); 
    float theta = 2 * 3.1415926 / float(num_segments); 
    float c = cosf(theta);//precalculate the sine and cosine
    float s = sinf(theta);
    float t;

    float x = r;//we start at angle = 0 
    float y = 0; 

    glColor3ub(90,140,90);
    glBegin(GL_LINE_LOOP); 
    for(int ii = 0; ii < num_segments; ii++) 
    { 
        glVertex2f(x + cx, y + cy);//output vertex 

        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    } 
    glEnd(); 
    glPopMatrix();
}

void render(Game *game)
{
    glClear(GL_COLOR_BUFFER_BIT);

        if (upPressed == 1){
            game->circle.center.y += 6;
        }
        if (downPressed == 1){
            game->circle.center.y += -6;
        }
        if (rightPressed == 1){
            game->circle.center.x += 6;
        }
        if (leftPressed == 1){
            game->circle.center.x += -6;
        }
        if (shootPressed == 1){
            makeShot(game, game->circle.center.x, game->circle.center.y + game->circle.radius);    
        }
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    DrawCircle(game->circle.center.x,game->circle.center.y,game->circle.radius,100);

    //draw box
    
    Shape *s;
    glColor3ub(90,140,90);
    for(int i =0; i < 5; i++){
        s = &game->box[i];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();

        glPopMatrix();
    }

    //draw all particles here
    for (int i=0; i <game->n; i++){
        glPushMatrix();
	if (i % 5 == 0)
	    glColor3ub(150,160,220);
	if (i % 5 == 1)
	    glColor3ub(160,170,230);
	if (i % 5 == 2)
	    glColor3ub(140,150,210);
	if (i % 5 == 3)
	    glColor3ub(130,140,200);
	if (i % 5 == 4)
	    glColor3ub(170,180,240);
        Vec *c = &game->particle[i].s.center;
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }
    for (int i=0; i <game->nShot; i++){
        glPushMatrix();
        glColor3ub(100,100,220);
        Vec *c = &game->shot[i].s.center;
        w = 6;
        h = 6;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }
    Rect r;
    r.bot = 310;
    r.left = 30;
    r.center = 0;
    ggprint16(&r, 16, 0x0000ff00, "Requirements");
    r.bot = 260;
    r.left = 110;
    r.center = 0;
    ggprint16(&r, 16, 0x000000ff, "Design");
    r.bot = 210;
    r.left = 170;
    r.center = 0;
    ggprint16(&r, 16, 0xff000000, "Coding");
    r.bot = 160;
    r.left = 220;
    r.center = 0;
    ggprint16(&r, 16, 0x00ff0000, "Testing");
    r.bot = 110;
    r.left = 250;
    r.center = 0;
    ggprint16(&r, 16, 0xfff0f000, "Maintenance");

}

