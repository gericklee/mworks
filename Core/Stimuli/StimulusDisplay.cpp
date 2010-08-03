/**
 * StimulusDisplay.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "OpenGLContextManager.h"
#include "Stimulus.h" 
#include "StimulusNode.h"
#include "Utilities.h"
#include "StandardVariables.h"
#include "Experiment.h"
#include "StandardVariables.h"
#include "ComponentRegistry.h"

#include "boost/bind.hpp"


#ifdef	__APPLE__
	#include <AGL/agl.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
    #include <CoreVideo/CVHostTime.h>
#elif	linux
	// TODO: where are these in linux?
#endif

#if M_OPENGL_SHARED_STATE == 1
    #define RENDER_ONCE
#endif


using namespace mw;



/**********************************************************************
 *                  StimulusDisplay Methods
 **********************************************************************/
StimulusDisplay::StimulusDisplay() {
    current_context_index = -1;

    // defer creation of the display chain until after the stimulus display has been created
    display_stack = shared_ptr< LinkedList<StimulusNode> >(new LinkedList<StimulusNode>());
    
	setDisplayBounds();

    opengl_context_manager = OpenGLContextManager::instance();
    clock = Clock::instance();
    
    displayLink = NULL;
    
}

StimulusDisplay::~StimulusDisplay(){
	if (displayLink) {
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
    }
}

void StimulusDisplay::setCurrent(int i){
    if ((i >= getNContexts()) || (i < 0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "invalid context index (%d)", i);
        return;
    }

	current_context_index = i;
	opengl_context_manager->setCurrent(context_ids[i]); 
}

shared_ptr<StimulusNode> StimulusDisplay::addStimulus(shared_ptr<Stimulus> stim) {
    if(!stim) {
        mprintf("Attempt to load NULL stimulus");
        return shared_ptr<StimulusNode>();
    }

	shared_ptr<StimulusNode> stimnode(new StimulusNode(stim));
	
    display_stack->addToFront(stimnode);
	
	return stimnode;
}

void StimulusDisplay::addStimulusNode(shared_ptr<StimulusNode> stimnode) {
    if(!stimnode) {
        mprintf("Attempt to load NULL stimulus");
        return;
    }
    
	// remove it, in case it already belongs to a list
	stimnode->remove();
	
	display_stack->addToFront(stimnode);  // TODO
}

void StimulusDisplay::setDisplayBounds(){
  shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
  shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
  
 Datum display_info = *main_screen_info; // from standard variables
	if(display_info.getDataType() == M_DICTIONARY &&
	   display_info.hasKey(M_DISPLAY_WIDTH_KEY) &&
	   display_info.hasKey(M_DISPLAY_HEIGHT_KEY) &&
	   display_info.hasKey(M_DISPLAY_DISTANCE_KEY)){
	
		GLdouble width_unknown_units = display_info.getElement(M_DISPLAY_WIDTH_KEY);
		GLdouble height_unknown_units = display_info.getElement(M_DISPLAY_HEIGHT_KEY);
		GLdouble distance_unknown_units = display_info.getElement(M_DISPLAY_DISTANCE_KEY);
	
		GLdouble half_width_deg = (180. / M_PI) * atan((width_unknown_units/2.)/distance_unknown_units);
		GLdouble half_height_deg = half_width_deg * height_unknown_units / width_unknown_units;
		//GLdouble half_height_deg = (180. / M_PI) * atan((height_unknown_units/2.)/distance_unknown_units);
		
		left = -half_width_deg;
		right = half_width_deg;
		top = half_height_deg;
		bottom = -half_height_deg;
	} else {
		left = M_STIMULUS_DISPLAY_LEFT_EDGE;
		right = M_STIMULUS_DISPLAY_RIGHT_EDGE;
		top = M_STIMULUS_DISPLAY_TOP_EDGE;
		bottom = M_STIMULUS_DISPLAY_BOTTOM_EDGE;
	}
	
	mprintf("Display bounds set to (%g left, %g right, %g top, %g bottom)",
			left, right, top, bottom);
}

void StimulusDisplay::getDisplayBounds(GLdouble &left, GLdouble &right, GLdouble &bottom, GLdouble &top) {
    left = this->left;
    right = this->right;
    bottom = this->bottom;
    top = this->top;
}

void StimulusDisplay::addContext(int _context_id){
    boost::mutex::scoped_lock lock(display_lock);
    
	context_ids.push_back(_context_id);
	current_context_index = context_ids.size() - 1;
    opengl_context_manager->setCurrent(_context_id);

#ifdef RENDER_ONCE
    if (!(glewIsSupported("GL_EXT_framebuffer_object") && glewIsSupported("GL_EXT_framebuffer_blit"))) {
        throw SimpleException("renderer does not support required OpenGL framebuffer extensions");
    }
#endif
    
    if (!displayLink) {
        if (kCVReturnSuccess != CVDisplayLinkCreateWithCGDisplay(opengl_context_manager->getMainDisplayID(),
                                                                 &displayLink) ||
            kCVReturnSuccess != CVDisplayLinkSetOutputCallback(displayLink,
                                                               &StimulusDisplay::displayLinkCallback,
                                                               this) ||
            kCVReturnSuccess != CVDisplayLinkStart(displayLink))
        {
            throw SimpleException("unable to schedule display updates");
        }

        fprintf(stderr, "\nMain display = %d\n", CGMainDisplayID());
        fprintf(stderr, "Active display = %d\n", CVDisplayLinkGetCurrentCGDisplay(displayLink));
    }
}

void StimulusDisplay::getCurrentViewportSize(GLint &width, GLint &height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
}


CVReturn StimulusDisplay::displayLinkCallback(CVDisplayLinkRef _displayLink,
                                              const CVTimeStamp *now,
                                              const CVTimeStamp *outputTime,
                                              CVOptionFlags flagsIn,
                                              CVOptionFlags *flagsOut,
                                              void *_display)
{
    StimulusDisplay *display = static_cast<StimulusDisplay*>(_display);

    if (-1 == display->opengl_context_manager->getMainDisplayIndex()) {
        return CVDisplayLinkStop(display->displayLink);
    }
    
    display->refreshDisplay();
    
    uint64_t currentTime = CVGetCurrentHostTime();
    if (currentTime > outputTime->hostTime) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "display refresh did not complete within allotted time");
        return kCVReturnError;
    }

    return kCVReturnSuccess;
}


void StimulusDisplay::refreshDisplay() {
    {
        boost::mutex::scoped_lock lock(display_lock);
        
        int refresh_rate = opengl_context_manager->getDisplayRefreshRate(opengl_context_manager->getMainDisplayIndex());
        if(refresh_rate <= 0){
            refresh_rate = 60;
        }
        
        MWTime before_draw = clock->getCurrentTimeUS();
        
#ifdef RENDER_ONCE
        setCurrent(0);  // Main display context
        GLint srcWidth, srcHeight;
        getCurrentViewportSize(srcWidth, srcHeight);
        
        GLuint framebuffer, renderbuffer;
        glGenFramebuffersEXT(1, &framebuffer);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
        glGenRenderbuffersEXT(1, &renderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, srcWidth, srcHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
        
        GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (GL_FRAMEBUFFER_COMPLETE_EXT != status) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "framebuffer setup failed (status = %d)", status);
            glDeleteRenderbuffersEXT(1, &renderbuffer);
            glDeleteFramebuffersEXT(1, &framebuffer);
            return;
        }
        
        drawDisplayStack();
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);  // Send subsequent rendering to back buffer
#endif /*RENDER_ONCE*/
        
        for(unsigned int i = 0; i < context_ids.size(); i++){
            
            setCurrent(i);
            
#ifndef RENDER_ONCE
            drawDisplayStack();
#else
            GLint dstWidth, dstHeight;
            getCurrentViewportSize(dstWidth, dstHeight);
            
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, framebuffer);
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
            glBlitFramebufferEXT(0, 0, srcWidth, srcHeight,
                                 0, 0, dstWidth, dstHeight,
                                 GL_COLOR_BUFFER_BIT,
                                 GL_LINEAR);
#endif /*RENDER_ONCE*/
            
            if (i != 0) {  // non-main display
                opengl_context_manager->updateAndFlush(i);
                continue;
            }
            
#define USE_GL_FENCE
#define ERROR_ON_LATE_FRAMES
            
#ifdef USE_GL_FENCE
            if(opengl_context_manager->hasFence()){
                glSetFenceAPPLE(opengl_context_manager->getFence());
            }
#endif
            
            opengl_context_manager->flush(i);
            
#ifdef USE_GL_FENCE
            if(opengl_context_manager->hasFence()){
                glFinishFenceAPPLE(opengl_context_manager->getFence());
            }
#endif
            
            MWTime now = clock->getCurrentTimeUS();
            stimDisplayUpdate->setValue(getAnnounceData(), now);
            announceDisplayStack(now);
            
#ifdef ERROR_ON_LATE_FRAMES
            MWTime slop = 2*(1000000/refresh_rate);
            
            if(now-before_draw > slop) {
                merror(M_DISPLAY_MESSAGE_DOMAIN,
                       "updating main window display is taking longer than two frames (%lld > %lld) to update", 
                       now-before_draw, 
                       slop);		
            }
#endif
        }	
        
#ifdef RENDER_ONCE
        glDeleteRenderbuffersEXT(1, &renderbuffer);
        glDeleteFramebuffersEXT(1, &framebuffer);
#endif
        
        refreshComplete = true;
    }
    
    // Signal waiting threads that refresh is complete
    refreshCond.notify_all();
}


void StimulusDisplay::clearDisplay() {
    {
        boost::mutex::scoped_lock lock(display_lock);

        shared_ptr<StimulusNode> node = display_stack->getFrontmost();
        while(node) {
            node->setVisible(false);
            node = node->getNext();
        }
    }
	
	updateDisplay();
}


void StimulusDisplay::glInit() {

    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // DDC added
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset The Projection Matrix
    
    gluOrtho2D(left, right, bottom, top);
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

}


void StimulusDisplay::drawDisplayStack() {
    // OpenGL setup

    glInit();
    
    // Draw all of the stimuli in the chain, back to front

    shared_ptr<StimulusNode> node = display_stack->getBackmost();
    while (node) {
        if (node->isVisible()) {
            node->draw(shared_from_this());
        }
        node = node->getPrevious();
    }
}


void StimulusDisplay::updateDisplay() {
	boost::mutex::scoped_lock lock(display_lock);
    
    shared_ptr<StimulusNode> node = display_stack->getFrontmost();
    while (node) {
        if (node->isPending()) {
            // we're taking care of the pending state, so
            // clear this flag
            node->clearPending();
            
            // convert "pending visible" stimuli to "visible" ones
            node->setVisible(node->isPendingVisible());
            
            if (node->isPendingRemoval()) {
                node->clearPendingRemoval();
                node->remove();
                continue;
            }
        }
        
        node = node->getNext();
    }

    // Wait for next display refresh to complete
    refreshComplete = false;
    do {
        refreshCond.wait(lock);
    } while (!refreshComplete);
}


void StimulusDisplay::announceDisplayStack(MWTime time) {
    shared_ptr<StimulusNode> node = display_stack->getBackmost(); //tail;
	
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
            node->announceStimulusDraw(time); 
		}
		
		node = node->getPrevious();
    }
	
}


Datum StimulusDisplay::getAnnounceData() {
    shared_ptr<StimulusNode> node = display_stack->getBackmost(); //tail;
	
    Datum stimAnnounce(M_LIST, 1);
    while(node != shared_ptr< LinkedListNode<StimulusNode> >()) {
		if(node->isVisible()) {
            Datum individualAnnounce(node->getCurrentAnnounceDrawData());
			if(!individualAnnounce.isUndefined()) {
				stimAnnounce.addElement(individualAnnounce);
			}
		}
		
		node = node->getPrevious();
    }	
	return stimAnnounce;
}





