/*
 *  NE500PumpNetworkDevice.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	_NE500PUMPNETWORKDEVICE_H_
#define _NE500PUMPNETWORKDEVICE_H_

#include "NE500DeviceChannel.h"


BEGIN_NAMESPACE_MW


class NE500PumpNetworkDevice : public IODevice, boost::noncopyable {
    
private:
    const std::string address;
    const int port;
    const MWTime response_timeout;
    
    // the socket
    int s;
    
    bool connected;
    
    std::vector<boost::shared_ptr<NE500DeviceChannel>> pumps;
    
    bool active;
    boost::mutex active_mutex;
    using scoped_lock = boost::mutex::scoped_lock;
    
public:
    static const std::string ADDRESS;
    static const std::string PORT;
    static const std::string RESPONSE_TIMEOUT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit NE500PumpNetworkDevice(const ParameterValueMap &parameters);
    ~NE500PumpNetworkDevice();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  shared_ptr<Component> _child) override;
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    static constexpr char PUMP_SERIAL_DELIMITER_CHAR = 3;  // ETX
    
    static std::string formatFloat(double val);
    
    bool connectToDevice();
    void disconnectFromDevice();
    
    bool sendMessage(const std::string &pump_id, string message);
    bool dispense(const std::string &pump_id, double rate, Datum data);
    bool initializePump(const std::string &pump_id, double rate, double syringe_diameter);
    
    
    class NE500DeviceOutputNotification : public VariableNotification {
    private:
        const boost::weak_ptr<NE500PumpNetworkDevice> pump_network;
        const boost::weak_ptr<NE500DeviceChannel> channel;
        
    public:
        NE500DeviceOutputNotification(const boost::shared_ptr<NE500PumpNetworkDevice> &_pump_network,
                                      const boost::shared_ptr<NE500DeviceChannel> &_channel) :
            pump_network(_pump_network),
            channel(_channel)
        { }
        
        void notify(const Datum &data, MWTime timeUS) override;
    };
};


END_NAMESPACE_MW


#endif





