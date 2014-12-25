/*

    ofxTableGestures (formerly OF-TangibleFramework)
    Developed for Taller de Sistemes Interactius I
    Universitat Pompeu Fabra

    Copyright (c) 2010 Daniel Gallardo Grassot <daniel.gallardo@upf.edu>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

#include <fstream>
#include "TableApp.hpp"
#include "GraphicDispatcher.hpp"
#include "Renderer_plane.hpp"
#include "Renderer_to_texture.hpp"
#include "ofxGlobalConfig.hpp"
#include "ObjectFeedback.hpp"
#include "Grid.hpp"
#include "Text.hpp"

#define WIDTH_STEP 0.005
#define ANGLE_STEP 1
//#define DISTORTION_PATH "calibration.conf"


float TableApp::height = 0;
float TableApp::width = 0;


TableApp::TableApp(std::string name):
//    calibration_enabled(false),
    calibration_mode(0),
    show_help(false),
    show_info(false),
    hide_cursor(true),
    #ifndef NO_SIMULATOR
    simulator(new simulator::Simulator()),
    is_simulating(false),
    #endif
    squaredInterface(ofxGlobalConfig::getRef("GLOBAL:SQUAREDINTERFACE",1)),
    matrix_updated(false)
{
    if(ofxGlobalConfig::getRef("GLOBAL:RENDERTOTEXTURE",0)) renderer = new Renderer_to_texture();
    else renderer = new Renderer_plane();

    renderer->LoadDistortion();
    show_grid = false;

    ofAddListener(ofEvents().update,this,&TableApp::update);
    ofAddListener(ofEvents().keyPressed,this,&TableApp::keyPressed);
    ofAddListener(ofEvents().keyReleased,this,&TableApp::keyReleased);
    ofAddListener(ofEvents().mouseDragged,this,&TableApp::mouseDragged);
    ofAddListener(ofEvents().mousePressed,this,&TableApp::mousePressed);
    ofAddListener(ofEvents().mouseReleased,this,&TableApp::mouseReleased);
    ofAddListener(ofEvents().windowResized,this,&TableApp::windowResized);
    
    win_name = name;
}

TableApp::~TableApp(){
    GenericManager::get<GraphicDispatcher>()->removeGraphic(grid);
    GenericManager::get<GraphicDispatcher>()->removeGraphic(helpText);
    GenericManager::get<GraphicDispatcher>()->removeGraphic(infoText);

#ifndef NO_SIMULATOR
    delete simulator;
    #endif
    delete renderer;

    ofRemoveListener(ofEvents().update,this,&TableApp::update);
    ofRemoveListener(ofEvents().keyPressed,this,&TableApp::keyPressed);
    ofRemoveListener(ofEvents().keyReleased,this,&TableApp::keyReleased);
    ofRemoveListener(ofEvents().mouseDragged,this,&TableApp::mouseDragged);
    ofRemoveListener(ofEvents().mousePressed,this,&TableApp::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased,this,&TableApp::mouseReleased);
    ofRemoveListener(ofEvents().windowResized,this,&TableApp::windowResized);

}


int TableApp::GetSquareSide(){
    if(ofGetWidth() > ofGetHeight())return ofGetHeight();
    else return ofGetWidth();
}

//--------------------------------------------------------------
void TableApp::setup(){
    ofSetFrameRate(60);
    ///starts the tuioinput thread
    //tuio::tuioinput::Instance().init();
    ofSetWindowTitle(win_name + "\t press 'h' to show help content");
    ofBackground(0, 0, 0);
    ofHideCursor();

    genericManager.initAll();
    GenericManager::get<GraphicDispatcher>()->createGraphic(grid, NOT_LAYER);
    grid->setVisible(show_grid);
    grid->setMode(calibration_mode);
    loadInfo();
    loadHelp();
}

//--------------------------------------------------------------
void TableApp::update(ofEventArgs & args){
    ///Update input events, it says to all input gestures to process the gesture stack.
    //tuio::tuioAreaDelivery::Instance().processTevents();
    oscInput.update();
    ///Update graphic data, with this command all update methods from all 'Graphics' are launched
    GenericManager::get<ObjectFeedback>()->update();
    ///Update simulator objects
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->Update();
    #endif
    updateInfo();
}

//--------------------------------------------------------------

void TableApp::loadInfo(){
    GenericManager::get<GraphicDispatcher>()->createGraphic(infoText, NOT_LAYER);
    infoText->ignoreMatrixStack(true);
    infoText->loadFont(10);
    infoText->setVisible(show_info);
    infoText->setColor(ofColor(0, 255, 0));
    infoText->setPosition(ofVec2f(64, 64));
}

void TableApp::updateInfo(){
    infoText->clear();
    *infoText << ofGetWidth() << "x" << ofGetHeight() << "@" << (int)ofGetFrameRate() << "fps" << std::endl;
    if(renderer->IsEnabled()){
        *infoText << "Distortion enabled" << std::endl;
    }else{
        *infoText << "Distortion disabled" << std::endl;
    }
    *infoText << "Calibration data:" << std::endl;
    *infoText << renderer->ToString();
}

void TableApp::loadHelp(){
    GenericManager::get<GraphicDispatcher>()->createGraphic(helpText, NOT_LAYER);
    helpText->ignoreMatrixStack(true);
    helpText->loadFont(10);
    helpText->setVisible(show_help);
    helpText->setColor(ofColor(0, 255, 0));
    helpText->setPosition(ofVec2f(64, 256));

    *helpText << "h - help content." << std::endl;
    *helpText << "i - shows graphic information content." << std::endl;
    *helpText << "f - toggle fullscreen." << std::endl;
    *helpText << "m - shows/hides cursor pointer." << std::endl;
    *helpText << "d - enable/disable distortion." << std::endl;
    *helpText << "c - calibration mode." << std::endl;
    *helpText << "Under calibration mode:" << std::endl;
    *helpText << "  r - reset calibration values." << std::endl;
    *helpText << "  l - load calibration file." << std::endl;
    *helpText << "  return  - toggle calibration parameter." << std::endl;
    *helpText << "  cursors - Changes the selected parameter." << std::endl;
#ifndef NO_SIMULATOR
    *helpText << "s - enable simulator." << std::endl;
    *helpText << "Under simulator mode:" << std::endl;
    *helpText << "  r - reset." << std::endl;
    *helpText << "  a - hold." << std::endl;
    *helpText << "  z - select." << std::endl;
#endif
}

void TableApp::draw(){
    ofPushMatrix();
    #ifndef NO_SIMULATOR
    if(is_simulating) ofScale(0.91f,0.91f,1.0f);
    #endif

    renderer->Start();
    ofPushMatrix();
    int shortside = min(ofGetWidth(),ofGetHeight());
    if(squaredInterface)
    {
        //if the surface is squared we center the drawing  plane
        glTranslatef((ofGetWidth()-shortside)/2.0,(ofGetHeight()-shortside)/2.0,0);
        TableApp::height = 1;
        TableApp::width = 1;
    }
    else
    {
        TableApp::height = float(ofGetHeight())/float(shortside);
        TableApp::width = float(ofGetWidth())/float(shortside);
    }
    glScalef(shortside,shortside,1);

    if(!matrix_updated)
    {
        matrix_updated = true;
        glGetDoublev(GL_MODELVIEW_MATRIX,Figures::CollisionHelper::ignore_transformation_matrix.data);
        Figures::CollisionHelper::ignore_transformation_matrix = Figures::CollisionHelper::ignore_transformation_matrix.GetInverse();
    }
    ///Draws all 'Graphics'
    glDisable(GL_DEPTH_TEST);
    ofPushMatrix();
    GenericManager::get<GraphicDispatcher>()->Draw();
    ofPopMatrix();
    glEnable(GL_DEPTH_TEST);
    renderer->End();
    ofPopMatrix();
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->Draw();
    #endif
}

//--------------------------------------------------------------
void TableApp::keyPressed(ofKeyEventArgs & event){
    int key = event.key;
    #ifndef NO_SIMULATOR
    switch(key)
    {
        case 'a':
            simulator->Hold(true);
        break;
        case 'z':
            simulator->Select(true);
        break;
        default:
            KeyPressed(key);
        break;
    }
    #endif
    Evaluate_Cursor(key);
}

//--------------------------------------------------------------
void TableApp::keyReleased(ofKeyEventArgs & event){
    int key = event.key;
    switch(key)
    {
        default:
            KeyReleased(key);
        break;
        #ifndef NO_SIMULATOR
        case 'a':
            simulator->Hold(false);
        break;
        case 'z':
            simulator->Select(false);
        break;
        #endif
        case 'f':
            ofToggleFullscreen();
		break;
		case 'c':
            if(renderer->IsEnabled()){
                renderer->SaveDistortion();
            }
            show_grid = !show_grid;
            grid->setVisible(show_grid);
        break;
        case OF_KEY_RETURN:
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid) calibration_mode ++;
            if(calibration_mode > 3) calibration_mode = 0;
            grid->setMode(calibration_mode);
        break;
        /*case OF_KEY_UP:
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid)
            {
                switch(calibration_mode)
                {
                    case 0:renderer->center_y--;break;
                    case 1:renderer->height_offset+=WIDTH_STEP;break;
                    case 2:renderer->angle+=ANGLE_STEP;break;
                    case 3:renderer->angle_h+=ANGLE_STEP;break;
                }
            }
        break;
        case OF_KEY_DOWN:
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid)
            {
                switch(calibration_mode)
                {
                    case 0:renderer->center_y++;break;
                    case 1:renderer->height_offset-=WIDTH_STEP;break;
                    case 2:renderer->angle-=ANGLE_STEP;break;
                    case 3:renderer->angle_h-=ANGLE_STEP;break;
                }
            }
        break;
        case OF_KEY_RIGHT:
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid)
            {
                switch(calibration_mode)
                {
                    case 0:renderer->center_x++;break;
                    case 1:renderer->width_offset+=WIDTH_STEP;break;
                    case 2:renderer->angle+=ANGLE_STEP;break;
                    case 3:renderer->angle_w+=ANGLE_STEP;break;
                }
            }
        break;
        case OF_KEY_LEFT:
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid)
            {
                switch(calibration_mode)
                {
                    case 0:renderer->center_x--;break;
                    case 1:renderer->width_offset-=WIDTH_STEP;break;
                    case 2:renderer->angle-=ANGLE_STEP;break;
                    case 3:renderer->angle_w-=ANGLE_STEP;break;
                }
            }
        break;*/
        case 'i':
            show_info = !show_info;
            infoText->setVisible(show_info);
        break;
        case 'm':
            if(hide_cursor)
            {
                hide_cursor=false;
                ofShowCursor();
            }else{
                hide_cursor=true;
                ofHideCursor();
            }
        break;
        case 'r':
            if(renderer->IsEnabled() && show_grid)
                renderer->LoadDefaultValues();
            #ifndef NO_SIMULATOR
                if(is_simulating)
                    simulator->Reset();
            #endif
        break;
        case 'l':
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            #endif
            if(renderer->IsEnabled() && show_grid)
                renderer->LoadDistortion();
        break;
        case 'h':
            show_help = !show_help;
            helpText->setVisible(show_help);
        break;
        case 'd':
            #ifndef NO_SIMULATOR
            if(!is_simulating)
            {
            #endif
            if(renderer->IsEnabled()) renderer->Disable();
            else renderer->Enable();
            #ifndef NO_SIMULATOR
            }
            #endif
        break;
        case 's':
            #ifndef NO_SIMULATOR
                if(is_simulating){
                    ///restore distorsion
                    ///restore cursor
                    if(was_distorsion_enabled) renderer->Enable();
                    else renderer->Disable();
                    if(was_cursor_hide){
                        hide_cursor=true;
                        ofHideCursor();
                    }
                    is_simulating=false;
                    simulator->Reset();
                }
                else{
                    was_distorsion_enabled = renderer->IsEnabled();
                    was_cursor_hide = hide_cursor;
                    renderer->Disable();
                    hide_cursor=false;
                    ofShowCursor();
                    is_simulating=true;
                }
                matrix_updated = false;
            #endif
        break;
        case 'b':
            Figures::CollisionHelper::debug_graphics = !Figures::CollisionHelper::debug_graphics;
        break;
    }
}

void TableApp::Evaluate_Cursor(int key)
{
    switch(key)
    {
        case OF_KEY_UP:
        #ifndef NO_SIMULATOR
            if(!is_simulating)
        #endif
                if(renderer->IsEnabled() && show_grid)
                {
                    switch(calibration_mode)
                    {
                        case 0:renderer->center_y--;break;
                        case 1:renderer->height_offset+=WIDTH_STEP;break;
                        case 2:renderer->angle+=ANGLE_STEP;break;
                        case 3:renderer->angle_h+=ANGLE_STEP;break;
                    }
                }
            break;
        case OF_KEY_DOWN:
        #ifndef NO_SIMULATOR
            if(!is_simulating)
        #endif
                if(renderer->IsEnabled() && show_grid)
                {
                    switch(calibration_mode)
                    {
                        case 0:renderer->center_y++;break;
                        case 1:renderer->height_offset-=WIDTH_STEP;break;
                        case 2:renderer->angle-=ANGLE_STEP;break;
                        case 3:renderer->angle_h-=ANGLE_STEP;break;
                    }
                }
            break;
        case OF_KEY_RIGHT:
        #ifndef NO_SIMULATOR
            if(!is_simulating)
        #endif
                if(renderer->IsEnabled() && show_grid)
                {
                    switch(calibration_mode)
                    {
                        case 0:renderer->center_x++;break;
                        case 1:renderer->width_offset+=WIDTH_STEP;break;
                        case 2:renderer->angle+=ANGLE_STEP;break;
                        case 3:renderer->angle_w+=ANGLE_STEP;break;
                    }
                }
            break;
        case OF_KEY_LEFT:
        #ifndef NO_SIMULATOR
            if(!is_simulating)
        #endif
                if(renderer->IsEnabled() && show_grid)
                {
                    switch(calibration_mode)
                    {
                        case 0:renderer->center_x--;break;
                        case 1:renderer->width_offset-=WIDTH_STEP;break;
                        case 2:renderer->angle-=ANGLE_STEP;break;
                        case 3:renderer->angle_w-=ANGLE_STEP;break;
                    }
                }
            break;
    }
}
//--------------------------------------------------------------
void TableApp::windowResized(ofResizeEventArgs & event){
    int w = event.width;
    int h = event.height;
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->windowResized(w,h);
    #endif
    grid->Resize();
    ///calls resize method of all 'Graphics' when nedded.
    GraphicDispatcher::Instance().Resize(w,h);
    matrix_updated = false;
}


//--------------------------------------------------------------
void TableApp::mouseDragged(ofMouseEventArgs & event){
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->mouseDragged(event.x,event.y,event.button);
    #endif
}

//--------------------------------------------------------------
void TableApp::mousePressed(ofMouseEventArgs & event){
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->mousePressed(event.x,event.y,event.button);
    #endif
}

//--------------------------------------------------------------
void TableApp::mouseReleased(ofMouseEventArgs & event){
    #ifndef NO_SIMULATOR
    if(is_simulating) simulator->mouseReleased(event.x,event.y,event.button);
    #endif
}


