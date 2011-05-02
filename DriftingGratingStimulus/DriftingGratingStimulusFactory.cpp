/*
 *  DriftingGratingStimulusFactory.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulusFactory.h"
#include "DriftingGratingStimulus.h"
#include "EllipseMask.h"
#include "GaussianMask.h"
#include "RectangleMask.h"
#include "SinusoidGratingData.h"
#include "TriangleGratingData.h"
#include "SawtoothGratingData.h"
#include "SquareGratingData.h"
#include <boost/regex.hpp>
#include "MWorksCore/ComponentRegistry.h"
using namespace mw;



shared_ptr<mw::Component> DriftingGratingStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                                        mw::ComponentRegistry *reg) {
	
	const char *TAG = "tag";
	const char *FRAMES_PER_SECOND = "frames_per_second";
	const char *X_SIZE = "x_size";
	const char *Y_SIZE = "y_size";
	const char *X_POSITION = "x_position";
	const char *Y_POSITION = "y_position";
	const char *ROTATION = "rotation";
	const char *DIRECTION = "direction";
	const char *STARTING_PHASE = "starting_phase";
	const char *FREQUENCY = "spatial_frequency";
	const char *SPEED = "speed";
	const char *GRATING_TYPE = "grating_type";
	const char *ALPHA_MULTIPLIER = "alpha_multiplier";
	const char *MASK = "mask";
	const char *GRATING_SAMPLE_RATE = "grating_sample_rate";
	
	REQUIRE_ATTRIBUTES(parameters, 
					   TAG,
					   FRAMES_PER_SECOND,
					   X_SIZE,
					   Y_SIZE,
					   X_POSITION,
					   Y_POSITION,
					   ROTATION,
					   DIRECTION,
					   FREQUENCY,
					   SPEED,
					   GRATING_TYPE,
					   MASK);
	
	std::string tagname(parameters.find(TAG)->second);
	shared_ptr<Variable> frames_per_second = reg->getVariable(parameters.find(FRAMES_PER_SECOND)->second);	
	shared_ptr<Variable> x_size = reg->getVariable(parameters.find(X_SIZE)->second);	
	shared_ptr<Variable> y_size = reg->getVariable(parameters.find(Y_SIZE)->second);	
	shared_ptr<Variable> x_position = reg->getVariable(parameters.find(X_POSITION)->second);	
	shared_ptr<Variable> y_position = reg->getVariable(parameters.find(Y_POSITION)->second);	
	shared_ptr<Variable> rotation = reg->getVariable(parameters.find(ROTATION)->second);	
	shared_ptr<Variable> alpha_multiplier = reg->getVariable(parameters[ALPHA_MULTIPLIER], "1");
	shared_ptr<Variable> direction_in_degrees = reg->getVariable(parameters.find(DIRECTION)->second);	
	shared_ptr<Variable> spatial_frequency = reg->getVariable(parameters.find(FREQUENCY)->second);	
	shared_ptr<Variable> speed = reg->getVariable(parameters.find(SPEED)->second);	
	shared_ptr<Variable> starting_phase = reg->getVariable(parameters[STARTING_PHASE], "0");	
	shared_ptr<Variable> grating_sample_rate = reg->getVariable(parameters[GRATING_SAMPLE_RATE], "32");	
	
	
	checkAttribute(frames_per_second, 
				   parameters.find("reference_id")->second, 
				   FRAMES_PER_SECOND, 
				   parameters[FRAMES_PER_SECOND]);
	checkAttribute(x_size, 
				   parameters.find("reference_id")->second,	
				   X_SIZE,	
				   parameters.find(X_SIZE)->second);                                                    
	checkAttribute(y_size, 
				   parameters.find("reference_id")->second,
				   Y_SIZE, 
				   parameters.find(Y_SIZE)->second);                                                    
	checkAttribute(x_position, 
				   parameters.find("reference_id")->second, 
				   X_POSITION, 
				   parameters.find(X_POSITION)->second);                                        
	checkAttribute(y_position, 
				   parameters.find("reference_id")->second,
				   Y_POSITION,
				   parameters.find(Y_POSITION)->second);                                        
	checkAttribute(rotation,
				   parameters.find("reference_id")->second, 
				   ROTATION, 
				   parameters.find(ROTATION)->second);                                              
	checkAttribute(alpha_multiplier,
				   parameters.find("reference_id")->second,
				   ALPHA_MULTIPLIER, 
				   parameters.find(ALPHA_MULTIPLIER)->second);                      
	checkAttribute(direction_in_degrees, 
				   parameters.find("reference_id")->second, 
				   DIRECTION, 
				   parameters.find(DIRECTION)->second);                                           
	checkAttribute(spatial_frequency,
				   parameters.find("reference_id")->second, 
				   FREQUENCY, 
				   parameters.find(FREQUENCY)->second);                                           
	checkAttribute(speed, 
				   parameters.find("reference_id")->second,
				   SPEED, 
				   parameters.find(SPEED)->second);                                                       
	checkAttribute(starting_phase,
				   parameters.find("reference_id")->second, 
				   STARTING_PHASE, 
				   parameters.find(STARTING_PHASE)->second);                            
	checkAttribute(grating_sample_rate,
				   parameters.find("reference_id")->second,
				   GRATING_SAMPLE_RATE, 
				   parameters.find(GRATING_SAMPLE_RATE)->second);                            
	
	
	
	shared_ptr <GratingData> grating;
	if(parameters.find(GRATING_TYPE)->second == "sinusoid") {
		grating = shared_ptr<SinusoidGratingData>(new SinusoidGratingData(grating_sample_rate));
	} else if(parameters.find(GRATING_TYPE)->second == "square") {
		grating = shared_ptr<SquareGratingData>(new SquareGratingData(grating_sample_rate));
	} else if(parameters.find(GRATING_TYPE)->second == "triangle") {
		grating = shared_ptr<TriangleGratingData>(new TriangleGratingData(grating_sample_rate));
	} else if(parameters.find(GRATING_TYPE)->second == "sawtooth") {
		const char *INVERTED = "inverted";
		
		shared_ptr <Variable> inverted = reg->getVariable(parameters[INVERTED], "0");	
		checkAttribute(inverted,
					   parameters.find("reference_id")->second, 
					   INVERTED, 
					   parameters.find(INVERTED)->second);                                              
		grating = shared_ptr<SawtoothGratingData>(new SawtoothGratingData(grating_sample_rate,
																			inverted));
	} else {
		throw SimpleException("illegal grating type: " + parameters.find(GRATING_TYPE)->second);		
	}
	
	shared_ptr <Mask> mask;
	shared_ptr <Variable> mask_size = shared_ptr<Variable>(new ConstantVariable(128L));
	if(parameters.find(MASK)->second == "rectangle") {
		mask = shared_ptr<Mask>(new RectangleMask(mask_size));
	} else if(parameters.find(MASK)->second == "ellipse") {
		mask = shared_ptr<Mask>(new EllipseMask(mask_size));
	} else if(parameters.find(MASK)->second == "gaussian") {
		const char *STD_DEV = "std_dev";
		const char *MEAN = "mean";
		
		//		REQUIRE_ATTRIBUTES(parameters, 
		//						   STD_DEV,
		//						   MEAN);
		
		shared_ptr <Variable> std_dev = reg->getVariable(parameters[STD_DEV], "1");	
		shared_ptr <Variable> mean = reg->getVariable(parameters[MEAN], "0");	
		checkAttribute(std_dev,
					   parameters.find("reference_id")->second, 
					   STD_DEV, 
					   parameters.find(STD_DEV)->second);                                              
		checkAttribute(mean,
					   parameters.find("reference_id")->second, 
					   MEAN, 
					   parameters.find(MEAN)->second);                                              
		
		mask = shared_ptr<Mask>(new GaussianMask(mask_size,
													 mean,
													 std_dev));
	} else {
		throw SimpleException("illegal mask: " + parameters.find(MASK)->second);				
	}
	
	shared_ptr<DriftingGratingStimulus> new_drifting_grating=shared_ptr<DriftingGratingStimulus>(new DriftingGratingStimulus(tagname, 
																																frames_per_second,
																																x_position,
																																y_position,
																																x_size,
																																y_size,
																																rotation,
																																alpha_multiplier,
																																direction_in_degrees,
																																spatial_frequency,
																																speed,
																																starting_phase,
																																mask,
																																grating));
	

	new_drifting_grating->load(StimulusDisplay::getCurrentStimulusDisplay());
	shared_ptr <StimulusNode> thisStimNode = shared_ptr<StimulusNode>(new StimulusNode(new_drifting_grating));
	reg->registerStimulusNode(tagname, thisStimNode);
	
	return new_drifting_grating;
}

