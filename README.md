DAT2C
=====
The project is a Telecommunications Substation in Distribution Automation, 
Including IEC60870-101, IEC60870-103, IEC60870-104, MODBUS, CDT and some individual power communication protocol. 
Power Communication via TCP/UDP, Serial Port and other channel. 
Including Feeder Automation and some other power automation. 

The project used some of the feature of boost and openssl. Need to add the include and lib path for both to compile.
The latest test was with Boost1.53.0 and OpenSSL1.0.1c on Windows with VS2012 and Linux with GCC4.6.

There is two executable files in binary/ , they are compiled used static link for using conveniently. 
The size of executable file using dymanic link is less than 1M fitting for Embedded system.

The executable file need a xml file called "sub.xml" as the main config file to run, there is a sample called "sub-demo.xml" in cfg/ .
