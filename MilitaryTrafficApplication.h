#ifndef MILITARY_TRAFFIC_APPLICATION_H
#define MILITARY_TRAFFIC_APPLICATION_H

#include "ns3/application.h"

class MilitaryTrafficApplication : public Application
{
public:
    MilitaryTrafficApplication(uint32_t &priorityFlag);
    virtual ~MilitaryTrafficApplication();

    void SetTrafficData(/* Add parameters for setting up traffic data */);

    // Method to set the priority flag
    void SetPriorityFlag(uint32_t &priorityFlag);

private:
    virtual void StartApplication();
    virtual void StopApplication();

    // Add private members as needed
    uint32_t &priorityFlag_; // Reference to the priority flag
};

#endif // MILITARY_TRAFFIC_APPLICATION_H
