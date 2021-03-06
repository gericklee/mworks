/*
 *  Selectable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 5/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "Selectable.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"


BEGIN_NAMESPACE_MW


Selectable::Selectable(){
	selection = shared_ptr<Selection>();
}

//DAVE TODO DEFINE
Selectable::~Selectable() { }


void Selectable::attachSelection(shared_ptr<Selection> newselect) { 
	selection = newselect; 
	selection->setSelectable(this);
	selection->reset();
}

shared_ptr<Selection> Selectable::getSelectionClone() {
	if (selection != 0) {
		return selection;
//		return selection->clone();
	} else {
		return shared_ptr<Selection>();
	}
}


void Selectable::acceptSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"accept\" on an empty selection object");
	}
	selection->acceptSelections();
}

void Selectable::rejectSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"reject\" on an empty selection object");
	}
	selection->rejectSelections();
}

void Selectable::resetSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"reset\" on an empty selection object");
	}
	selection->reset();
}

int Selectable::getNLeft() {
    if (selection) {
        return selection->getNLeft();
    }
    return 0;
}

int Selectable::getNAccepted() {
    if (selection) {
        return selection->getNAccepted();
    }
    return 0;
}

int Selectable::getNDone() {
    if (selection) {
        return selection->getNDone();
    }
    return 0;
}


END_NAMESPACE_MW
