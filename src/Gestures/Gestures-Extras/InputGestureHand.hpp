/*

    ofxTableGestures (formerly OF-TangibleFramework)
    Developed for Taller de Sistemes Interactius I
    Universitat Pompeu Fabra

    Copyright (c) 2010 Carles F. Julià <carles.fernandez@upf.edu>

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
#ifndef INPUTGESTUREHAND_HPP_INCLUDED
#define INPUTGESTUREHAND_HPP_INCLUDED

#include "InputGestureBasicFingers.hpp"
#include "InputGestureDirectFingers.hpp"
#include "DirectPoint.hpp"
#include <algorithm>



class InputGestureHands;

class Hand : public DirectFinger
{
private:
    DirectPoint  getCenter();
    bool isValid();
    float getRadius();
protected:
    friend class InputGestureHands;
    std::list<DirectFinger *> fingers;
    void UpdateExposedInfo()
    {
        center = getCenter();
        DirectPoint nc = center + offset;
        set(nc.getX(),nc.getY());
        is_valid = isValid();
        radius = getRadius();
        population = fingers.size();
    }
    void UpdateOffset()
    {
        DirectPoint pc(getX(),getY());
        UpdateExposedInfo();
        offset = pc - center;
    }
public:
    Hand(unsigned int ID):
        id(ID),
        center(),
        offset()
    {
        s_id=id;
    }
    ///Exposed info:
    unsigned int id;
    bool is_valid;
    float radius;
    unsigned int population;
    DirectPoint center;
    DirectPoint offset;
};




class InputGestureHands : public CanDirectFingers<  CompositeGesture>, public Singleton<InputGestureHands>
{
    std::map<int,Hand*> assignations;
    std::list<Hand *> hands;
    Hand * getNeighbour(DirectPoint * p, Hand * ignore = NULL);
    float & HAND_MAX_RADIUS;
public:
    struct newHandArgs: public EventArgs
    {
        Hand * hand;
    };
    struct removeHandArgs: public EventArgs
    {
        Hand * hand;
    };
    struct updateHandArgs: public EventArgs
    {
        Hand * hand;
    };
    ofEvent<newHandArgs> newHand;
    ofEvent<removeHandArgs> removeHand;
    ofEvent<updateHandArgs> updateHand;
    InputGestureHands():
        HAND_MAX_RADIUS(GlobalConfig::getRef("GESTURES:HANDS:MAX_RADIUS",0.1f))
    {}
    void newCursor(DirectFinger *df);
    void removeCursor(DirectFinger *df);
    void updateCursor(DirectFinger *df);

};

template <class Base>
class CanHands : public Base
{
public:
    void EnewHand(InputGestureHands::newHandArgs & eventargs)
    {
        newHand(eventargs.hand);
    }
    virtual void newHand( Hand * hand) {}
    void EremoveHand(InputGestureHands::removeHandArgs & eventargs)
    {
        removeHand(eventargs.hand);
    }
    virtual void removeHand( Hand * hand) {}
    void EupdateHand(InputGestureHands::updateHandArgs & eventargs)
    {
        updateHand(eventargs.hand);
    }
    virtual void updateHand( Hand * hand) {}


    CanHands()
    {
        ofAddListener(InputGestureHands::Instance().newHand,this,&CanHands::EnewHand);
        ofAddListener(InputGestureHands::Instance().removeHand,this,&CanHands::EremoveHand);
        ofAddListener(InputGestureHands::Instance().updateHand,this,&CanHands::EupdateHand);
    }
};

template<class Base>
class OrphanHandsAsFingers: public CanHands<Base>
{
    std::map<unsigned int,Hand*> hands;
public:
    //Interface redefined by ofApp
    virtual void newCursor(DirectFinger *) {}
    virtual void removeCursor(DirectFinger *) {}
    virtual void updateCursor(DirectFinger *) {}

    void newHand(Hand * p)
    {
        if(p->population == 1)
        {
            hands[p->id]=p;
            newCursor(p);
        }
    }
    void removeHand(Hand * h)
    {
        if(hands.find(h->id) == hands.end())return;
        hands.erase(h->id);
        removeCursor(h);
    }
    void updateHand(Hand * p)
    {
        if(hands.find(p->id) == hands.end())return;
        if(p->population != 1)
        {
            removeHand(p);
        }
        else
        {
            updateCursor(p);
        }
    }
};

template<class Base>
class NonOrphanHandsAsFingers: public CanHands<Base>
{
    std::set<unsigned int> nonorphan;
public:
    //Interface redefined by ofApp
    virtual void newCursor(DirectFinger *) {}
    virtual void removeCursor(DirectFinger *) {}
    virtual void updateCursor(DirectFinger *) {}

    void newHand( Hand * p)
    {
        if(p->population > 1)
        {
            nonorphan.insert(p->id);
            newCursor(p);
        }
    }
    void removeHand( Hand * p)
    {
        if(nonorphan.find(p->id) == nonorphan.end())return;
        nonorphan.erase(p->id);
        removeCursor(p);
    }
    void updateHand( Hand * p)
    {
        if((p->population <=1) && (nonorphan.find(p->id) != nonorphan.end()))
        {
            nonorphan.erase(p->id);
            removeCursor(p);
        }
        else if((p->population > 1) && (nonorphan.find(p->id) == nonorphan.end()))
        {
            nonorphan.insert(p->id);
            newCursor(p);
        }
        else if(nonorphan.find(p->id) != nonorphan.end())
        {
            updateCursor(p);
        }
    }
};


#endif // INPUTGESTUREHAND_HPP_INCLUDED
