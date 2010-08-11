/*
 *  DynamicStimulusDriver.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DynamicStimulusDriver.h"
#include "boost/bind.hpp"
#include "MWorksCore/StandardVariables.h"

namespace mw{

DynamicStimulusDriver::DynamicStimulusDriver(const boost::shared_ptr<Scheduler> &a_scheduler,
								   const boost::shared_ptr<StimulusDisplay> &a_display,
								   const boost::shared_ptr<Variable> &_frames_per_second){
    start_time = -1;
    scheduler = a_scheduler;
    clock = scheduler->getClock();
	display = a_display;
	frames_per_second = _frames_per_second;
	
	started = false;
    
                                                                                              
    state_system_callback = shared_ptr<VariableCallbackNotification>(
                                new VariableCallbackNotification(boost::bind(&DynamicStimulusDriver::stateSystemCallback, this, _1,_2))
                            );
    state_system_mode->addNotification(state_system_callback);
}

DynamicStimulusDriver::DynamicStimulusDriver(const DynamicStimulusDriver &tocopy){}

void DynamicStimulusDriver::stateSystemCallback(const Datum& data, MWorksTime time){
    if(data.getInteger() == IDLE){
        stop();
    }
}

DynamicStimulusDriver::~DynamicStimulusDriver(){
 
    state_system_callback->remove();
}

void DynamicStimulusDriver::play() {
	boost::mutex::scoped_lock locker(stim_lock);
	
    //mprintf("CALLED PLAY!");
    
    if (started) {
        return;
    }
    
    willPlay();
    
    start_time = clock->getCurrentTimeUS();
    
    //const float frames_per_us = frames_per_second->getValue().getFloat()/1000000;
    interval_us = (MWorksTime)((double)1000000 / frames_per_second->getValue().getFloat());
    
    started = true;
}

void DynamicStimulusDriver::stop() {
	boost::mutex::scoped_lock locker(stim_lock);
	
    //mprintf("CALLED STOP!");
    
    if (!started) {
        return;
    }

	started = false;
    
    didStop();
}

// TODO: must work out what to do here, because this is ugly
Datum DynamicStimulusDriver::getCurrentAnnounceDrawData() {
	Datum announce_data(M_DICTIONARY, 4);
	announce_data.addElement(STIM_NAME,"");        // char
	announce_data.addElement(STIM_ACTION,STIM_ACTION_DRAW);
	announce_data.addElement(STIM_TYPE,"dynamic_stimulus");  
	announce_data.addElement("start_time", start_time);  
	return announce_data;
}

MWTime DynamicStimulusDriver::getElapsedTime(){
    
    if(!started){
        return -1;
    }
    
    MWTime now = clock->getCurrentTimeUS();
    return now - start_time;
}

int DynamicStimulusDriver::getFrameNumber(){
    
    if (!started) {
        return -1;
    }

    MWTime elapsed = getElapsedTime();
    return (elapsed * (long)(frames_per_second->getValue())) / 1e6;
}

} // end namespace








