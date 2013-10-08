#pragma once

#include <string>
#include <vector>

#include "SynKit.h"

#include "com_ptr.h"

class forcepad_base 
    : public _ISynAPIEvents, public _ISynDeviceEvents // Can receive Synaptics event notifications
{
public:
    forcepad_base();
    virtual ~forcepad_base();
    const bool connected() const; // Connected to a physical ForcePad?
    // Callbacks
    virtual HRESULT STDMETHODCALLTYPE OnSynAPINotify(LONG lReason);
    virtual HRESULT STDMETHODCALLTYPE OnSynDevicePacket(LONG lSeq);
    // Supported number of fingers
    long MAX_GROUP_SIZE;
    // Accessors
    long get_device_property(long specifier) const;
    std::string get_device_string_property(long specifier) const;
    long get_group_property(long specifier) const;
    long get_group_property_indexed(long specifier, int index) const;
    long get_packet_property(int i, long specifier) const;
private:
    void connect();
    void disconnect();
    // Synaptics COM interface
    com_ptr<ISynAPI> m_api;
    com_ptr<ISynDevice> m_device;
    com_ptr<ISynGroup> m_group;
    com_ptr<ISynPacket> m_packet;
};


class forcepad : public forcepad_base
{
public:
    forcepad();
    virtual ~forcepad();
    // Callbacks
    virtual HRESULT STDMETHODCALLTYPE OnSynAPINotify(LONG lReason);
    virtual HRESULT STDMETHODCALLTYPE OnSynDevicePacket(LONG lSeq);
    // Device properties
    long XLoRim;
    long YLoRim;
    long XHiRim;
    long YHiRim;
    long ZMaximum;
    // Touchpad corner forces
    std::vector<long> cornerForce;
    // Per-finger properties X, Y, Z(touch size), F(force), 
    std::vector<long> X, Y, Z, F, FingerState;
    std::vector<double> filteredF;
private:
    void update_device();
};
