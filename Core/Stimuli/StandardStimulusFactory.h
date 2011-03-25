/*
 *  StandardStimulusFactory.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_STANDARD_STIMULUS_FACTORY_H
#define _MW_STANDARD_STIMULUS_FACTORY_H

#include "StandardComponentFactory.h"
#include "StimulusNode.h"
#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


template<typename StimulusType>
class StandardStimulusFactory : public StandardComponentFactory<StimulusType> {
    
public:
    virtual boost::shared_ptr<mw::Component> createObject(ComponentFactory::StdStringMap parameters,
                                                          ComponentRegistryPtr reg)
    {
        ParameterValueMap values;
        boost::shared_ptr<StimulusType> stim(StandardComponentFactory<StimulusType>::createObject(parameters,
                                                                                                  reg,
                                                                                                  values));
        
        stim->load(StimulusDisplay::getCurrentStimulusDisplay());
        boost::shared_ptr<StimulusNode> node(new StimulusNode(stim));
        reg->registerStimulusNode(values.at("tag"), node);
        
        return stim;
    }
    
};


END_NAMESPACE_MW


#endif
