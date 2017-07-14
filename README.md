# packetSensor
An example in QT / C++ of a packet sensor that leverages a webhook to inject JSON into Stackstorm / Workflow Composer

This example is intended to compile and run on Linux. The code was tested on Ubuntu 16.0.4 LTS and compiled using QT Creator version 4.1.0 based upon QT libraries version 5.7.0 and GCC version 4.9.1

The example code 'packetSensor' is intended to demonstrate a cyber awareness capability. 

Purpose:
On some networks, it is easy to define what source and destination IP address should exist. Seeing other IP addresses could raise an alert that someting strange is happening and at a minimum the operatior / security team should get an alert. 

Operation:
This code threads off tcpdump configured with a filter dsescribed in config.josn. When a packet matching the filter is detected, a webhook is triggered to workflow composer. A simple workflow could be to trigger an e-mail alert with the informaiton about the packet source and destination addresses, port and protocol and sensor IP and test description of the sensor that saw the packet. 

When a packet is detected a single webhook is triggered and the sensor is disarmed. The code then waits 5 minutes before re-arming. This feature is intened to preform hysteresis so that we don't trigger an e-mail storm.

Considerations:
This code is dynamically linked and requires the QT libraries to be present on the target system. This constraint is imposed to comply with the QT license. 

For more information:
See the README.pdf file for more details.
