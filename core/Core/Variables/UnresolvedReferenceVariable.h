/*
 *  UnresolvedReferenceVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 1/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _UNRESOLVED_REFERENCE_VARIABLE
#define _UNRESOLVED_REFERENCE_VARIABLE

#include "GenericVariable.h"
#include "ExpressionVariable.h"
#include "ComponentRegistry.h"
#include <string>
#include <sstream>


BEGIN_NAMESPACE_MW


class UnresolvedVariableString {

	protected:
		std::vector <shared_ptr<Variable> >stringFragments;

	public:
	
		UnresolvedVariableString(const std::string &unresolved_string) {
		
			std::string outStr(unresolved_string);	
			
			stringFragments.clear();
			
			while(outStr.find_first_of("$") != std::string::npos) {
				std::string stringSegment;
				std::string varName;
				std::istringstream parser(outStr);
				
				getline(parser, stringSegment, '$');
				shared_ptr<ConstantVariable> c(new ConstantVariable(Datum(stringSegment)));
				stringFragments.push_back(c);
				
				getline(parser, varName, ' ');
				shared_ptr<Variable> var = global_variable_registry->getVariable(varName);
				if(!var) {
					var = shared_ptr<ConstantVariable>(new ConstantVariable(Datum(std::string("UNKNOWNVAR"))));			
				}
				
				stringFragments.push_back(var);
				
				outStr.erase(0, stringSegment.size() + varName.size() + 1);
			}
			
			// add any remainder
			shared_ptr<ConstantVariable> remainder(new ConstantVariable(Datum(outStr)));
			stringFragments.push_back(remainder);
		}


		string resolve() {
			std::string outStr("");
			
			for(std::vector<shared_ptr<Variable> >::iterator i=stringFragments.begin();
				i != stringFragments.end();
				++i) {
				
				shared_ptr<Variable> var = *i;
				
				outStr.append(var->getValue().toString());		
			}
			
			return outStr;
		}


};

class UnresolvedReferenceVariable : public ReadOnlyVariable {

	protected:
	
		weak_ptr<ComponentRegistry> registry;
		string expression;
		UnresolvedVariableString unresolved_string;
		
	
	public:

		UnresolvedReferenceVariable (string _expression, shared_ptr<ComponentRegistry> _registry) :
										expression(_expression),
										unresolved_string(_expression){
										
			registry = weak_ptr<ComponentRegistry>(_registry);
		}
		
		
        Datum getValue() override {
			
			string resolved_string = unresolved_string.resolve();
			
            shared_ptr<ComponentRegistry> registry_shared = registry.lock();
			if(registry_shared){
				shared_ptr<Variable> resolved_variable = registry_shared->getVariable(resolved_string);
				
				if(resolved_variable != NULL){
					return resolved_variable->getValue();
				}
			}
			
			return Datum();
		}

};


END_NAMESPACE_MW


#endif
