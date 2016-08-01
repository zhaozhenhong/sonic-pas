PAS - Platform Adaptation Service
--------------------------------

Description
-----------
This is the implementation of the PAS for the SONiC project.
The PAS provides the following features and more:
* Monitors the status of the SDI devices  (via the SDI API)
* Initializes the platform devices
* Initializes the leds 
* Monitors insertion/removal of the optics and sends sonic-object-library events (use the cps_event_trace.py script to display all events in the system).

The interface provided from the PAS is via the sonic-object-library through registrations for the led objects, inventory objects and the other required device objects. 

Look in the src/models for the yang model that describes the hardware.
Look at the unit tests from example uses in C++.
Look at the paltform configuration repo to see a example of monitoring PAS events via pythin.

The YANG model for the CPS API PAS reference is : dell-base-pas.yang

Building
--------
Please see the instructions in the sonic-nas-manifest repo for more details on the common build tools.  [Sonic-nas-manifest](https://github.com/Azure/sonic-nas-manifest)

Development Dependencies:
 - sonic-logging
 - sonic-common
 - sonic-sdi-api
 - sonic-sdi-framework
 - sonic-sdi-sys
 - sonic-object-library
 - sonic-base-model

Dependent Packages:

 - libsonic-logging1 libsonic-logging-dev libsonic-common1 libsonic-common-dev  libsonic-model1 libsonic-model-dev sonic-sdi-api-dev libsonic-sdi-framework1  libsonic-sdi-framework-dev libsonic-sdi-sys1 libsonic-sdi-sys-dev libsonic-object-library1 libsonic-object-library-dev


BUILD CMD: sonic_build --dpkg libsonic-logging1 libsonic-logging-dev libsonic-common1 libsonic-common-dev  libsonic-model1 libsonic-model-dev sonic-sdi-api-dev libsonic-sdi-framework1  libsonic-sdi-framework-dev libsonic-sdi-sys1 libsonic-sdi-sys-dev libsonic-object-library1 libsonic-object-library-dev -- clean binary 


(c) Dell 2016

