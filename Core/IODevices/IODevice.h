#ifndef _IODEVICE_H
#define _IODEVICE_H

#include "Lockable.h"
#include "Component.h"
#include "ComponentRegistry.h"
#include "boost/enable_shared_from_this.hpp"


namespace mw {
	
	class IODevice : public Lockable, public mw::Component, public enable_shared_from_this<IODevice> {
		
    public:
        //
        // finalize()
        //
        // Overrides Component::finalize.  IODevice subclasses should not need to override this, but if they
        // do, they must call the base class method.
        //
        // Note that this is *not* the place to put shutdown/cleanup code for a device.  (That should go in the
        // destructor.)  This method is invoked when an experiment is first loaded and, in fact, is responsible
        // for calling the IODevice's initialize() method.
        //
        void finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg);

        //
        // initialize()
        //
        // Called during the "finalize" pass of the parser (after any channels or other IODevices have been created).
        // This is where subclasses should do any one-time startup/initialization tasks.  If device startup fails or
        // the experiment's demands are impossible to meet, this method should return false.
        //
        // Any tasks that need to be performed *before* channels or (potentially) other IODevices have been created
        // should go in the constructor, not initialize().
        //
        virtual bool initialize() {
            return true;
        }

        //
        // startDeviceIO()
        //
        // Called in response to a "Start IO Device" action.  It should start actual device I/O.
        //
        virtual bool startDeviceIO() {
            return true;
        }

        //
        // stopDeviceIO()
        //
        // Called in response to a "Stop IO Device" action.  It should stop device I/O but should *not* shut down
        // the device or undo any setup/initialization performed in initialize().  Any shutdown/cleanup code should
        // go in the destructor.
        //
        virtual bool stopDeviceIO() {
            return true;
        }
		
	};
	
}

#endif
