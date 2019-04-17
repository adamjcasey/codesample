# Sample Code
This sample code is a selection of code from a medical device that ran using embedded Linux.  The code focuses on the implementation, test, and development tools used to create and simulate the graphical user interface.

It is designed using dependency injection to allow for easy and thorough unit testing.  

The `src` directory contains the code that was compiled to run on the target.
The `test` directory contains the Google Test unit test files.  These were run as part of a continuous integration system.
The `tools` directory contains tools used to simulate the code on a Win32 platform, both to speed development, and to aid in demonstration and testing of functionality with clients and users.
