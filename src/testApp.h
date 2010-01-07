#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "tuioApp.h"
#include "InputGestureClasses.h"

#define WINDOW_HEIGHT 768
#define WINDOW_WIDTH 1024

using namespace tuio;

class testApp : public CanBasicObjects< CanDirectFingers<tuioApp <ofBaseApp> > >
{
private:

    double dx, dy;
    DirectFinger * mydf;

public:

    void setup();
    void update();
    void draw();

    void updateTuioCursor(int32 id, float xpos,float ypos,float xspeed,float yspeed,float maccel);

    void newCursor(int32 id, DirectFinger * df);
    void removeCursor(int32 id);

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);

};

#endif
