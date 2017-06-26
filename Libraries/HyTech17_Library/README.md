![HyTech logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)

# HyTech17 Library
This library serves as a central location for ECU state IDs, CAN message IDs, and CAN message parsers.
## Getting started
You will need to add this directory to your Arduino libraries directory. The preferred way to do this is with a symlink to this directory's location in the cloned repository. This way it will be updated whenever you pull updates from this repository.
## CAN message parsing
Each message type that is sent on the vehicle CAN bus is implemented as a struct and a class in this library. Some messages are only sent and others are only read by team-designed ECUs. Each class for messages sent by team-designed ECUs has functions to create the message, while all other classes only have functions to read the message.
## CAN message data formats
To avoid issues with floating point numbers, HyTech's CAN messages instead use scaled integers, predefined based on the data type. For consistency, we use the RMS CAN Protocol Data Formats when possible which can be referenced in [their documentation](https://app.box.com/s/vf9259qlaadhzxqiqrt5cco8xpsn84hk/1/2822171827/27334613044/1) (page 11). This library does not attempt to convert these values into floating point numbers.
