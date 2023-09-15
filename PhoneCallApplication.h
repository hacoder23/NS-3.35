#ifndef PHONE_CALL_APPLICATION_H
#define PHONE_CALL_APPLICATION_H

#include "ns3/application.h"

class PhoneCallApplication : public Application
{
public:
    PhoneCallApplication(uint32_t &priorityFlag);
    virtual ~PhoneCallApplication();

    void SetTrafficData(/* Add parameters for setting up traffic data */);

    // Method to set the priority flag
    void SetPriorityFlag(uint32_t &priorityFlag);

private:
    virtual void StartApplication();
    virtual void StopApplication();

    // Add private members as needed
    uint32_t &priorityFlag_; // Reference to the priority flag
};

#endif // PHONE_CALL_APPLICATION_H
