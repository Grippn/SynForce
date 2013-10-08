#include <cassert>
#include <stdexcept>

#include "forcepad.h"

using namespace std;

forcepad_base::forcepad_base()
{
    connect();
}

forcepad_base::~forcepad_base()
{
    disconnect();
}

void forcepad_base::connect()
{
    //xlog << "Connecting to ForcePad" << endl;
    HRESULT hr;
    hr = SynCreateAPI(m_api);
    if (hr != SYN_OK)
        //xlog << "Could not initialize Synaptics COM API" << endl;
    if (hr == SYN_OK)
    {
         m_api->SetSynchronousNotification(this) ;

        long lHandle = -1; // Request any device matching criteria
        // Assumes only one Synaptics USB TouchPad is present
        hr = m_api->FindDevice(SE_ConnectionUSB, SE_DeviceTouchPad, &lHandle);
        if (hr != SYN_OK)
            //xlog << "Could not find USB TouchPad" << endl;
        if (hr == SYN_OK)
        {
            hr = m_api->CreateDevice(lHandle, m_device);
            if (hr != SYN_OK)
                //xlog << "Could not create USB TouchPad device" << endl;
            if (hr == SYN_OK)
            {
                m_device->SetSynchronousNotification(this);

                // Validate that the touchpad supports multiple fingers and 
                // group reporting

                m_device->SetProperty(SP_IsMultiFingerReportEnabled, 1);

                long HasMultiFingerPacketsGrouped;
                m_device->GetProperty(SP_HasMultiFingerPacketsGrouped, &HasMultiFingerPacketsGrouped);
                //xlog << "HasMultiFingerPacketsGrouped = " << HasMultiFingerPacketsGrouped << endl;
    
                long HasPacketGroupProcessing;
               m_device->GetProperty(SP_HasPacketGroupProcessing, &HasPacketGroupProcessing);
                //xlog << "HasPacketGroupProcessing = " << HasPacketGroupProcessing << endl;

                m_device->SetProperty(SP_IsGroupReportEnabled, 1);

                m_device->GetProperty(SP_NumMaxReportedFingers, &MAX_GROUP_SIZE);

               // XSHOW(MAX_GROUP_SIZE);

                 m_device->CreateGroup(m_group) ;
                 m_device->CreatePacket(m_packet);

                 m_device->Acquire(0) ;

                //xlog << "Connected to ForcePad" << endl;
            }
        }
    }
}

const bool forcepad_base::connected() const
{
    return m_device;
}

// Called when a packet or a group is ready
HRESULT forcepad_base::OnSynDevicePacket(LONG lSeq)
{
    m_device->LoadGroup(m_group);
    return SYN_OK;
}

long forcepad_base::get_device_property(long specifier) const
{
    long value;
    m_device->GetProperty(specifier, &value);
    return value;
}

string forcepad_base::get_device_string_property(long specifier) const
{ 
    const int N = 256;
    unsigned char buffer[N];
    long length = N;
    m_device->GetStringProperty(specifier, buffer, &length);
    return std::string((char*) buffer);

}

long forcepad_base::get_group_property(long specifier) const
{
    long value;
    m_group->GetProperty(specifier, &value);
    return value;
}

long forcepad_base::get_group_property_indexed(long specifier, int index) const
{
    long value;
    m_group->GetPropertyByIndex(specifier, index, &value);
    return value;
}

long forcepad_base::get_packet_property(int i, long specifier) const
{
    m_group->GetPacketByIndex(i, m_packet);
    long value;
    m_packet->GetProperty(specifier, &value);
    return value;
}

// forcepad

forcepad::forcepad() 
    : forcepad_base()
{
    XLoRim = YLoRim = XHiRim = YHiRim = ZMaximum = 0;
    cornerForce.resize(4, 0);
    update_device();
}

forcepad::~forcepad()
{
}

void forcepad::update_device()
{
    if (connected())
    {
        XLoRim = get_device_property(SP_XLoRim);
        YLoRim = get_device_property(SP_YLoRim);
        XHiRim = get_device_property(SP_XHiRim);
        YHiRim = get_device_property(SP_YHiRim);
        ZMaximum = get_device_property(SP_ZMaximum);
        X.resize(MAX_GROUP_SIZE, 0);
        Y.resize(MAX_GROUP_SIZE, 0);
        Z.resize(MAX_GROUP_SIZE, 0);
        F.resize(MAX_GROUP_SIZE, 0);
        FingerState.resize(MAX_GROUP_SIZE, 0);
        filteredF.resize(MAX_GROUP_SIZE, 0.0);
    }
}

HRESULT forcepad::OnSynAPINotify(LONG lReason)
{
    HRESULT hr = forcepad_base::OnSynAPINotify(lReason);
    update_device();
    return hr;
}

HRESULT forcepad::OnSynDevicePacket(LONG lSeq)
{
    HRESULT hr = forcepad_base::OnSynDevicePacket(lSeq);
    for (int i = 0; i != 4; ++i)
    {
        cornerForce[i] = get_group_property_indexed(SP_ForceRaw, i);
    }
    for (int i = 0; i != MAX_GROUP_SIZE; ++i)
    {
        X[i] = get_packet_property(i, SP_X);
        Y[i] = get_packet_property(i, SP_Y);
        Z[i] = get_packet_property(i, SP_Z);
        F[i] = get_packet_property(i, SP_ZForce);
        FingerState[i] = get_packet_property(i, SP_FingerState);
        filteredF[i] = filteredF[i] * 0.6 + F[i] * 0.4;
    }
    return hr;
}


