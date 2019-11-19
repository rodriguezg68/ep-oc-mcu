# Unit Testing for ep-oc-mcu

This document describes how to write and use unit tests for ep-oc-mcu.

### Introduction

Embedded Planet uses the same testing framework as Mbed-OS for consistency and ensuring good code functionality.


>Unit tests test code in small sections on a host machine. Unlike other testing tools, unit testing doesn't require embedded hardware or need to build a full operating system. Because of this, unit testing can result in faster tests than other tools. Unit testing happens in a build environment where you test each C or C++ class or module in isolation. Build test suites into separate test binaries and stub all access outside to remove dependencies on any specific embedded hardware or software combination. This allows you to complete tests using native compilers on the build machine.

Any differences in workflow will be highlighted below.

See the official Mbed-OS documentation for writing and running unit tests for more information: https://github.com/ARMmbed/mbed-os/blob/master/UNITTESTS/README.md

### Differences in Workflow

Since many stub classes and dependencies exist in the Mbed-OS source repository, Mbed-OS is required to build the unit tests in this repository properly. The Mbed-OS repository should be clone **beside** this repository, __not__ inside of it. For example, your directory structure should look like this when building and running ep-oc-mcu unit tests:

--> my-current-directory

----> ep-oc-mcu

----> mbed-os

### Building and Testing

You can build unit tests for ep-oc-mcu the same way you would build unit tests for mbed-os using cmake:


>>>Build tests directly with CMake.

>>>1.) Create a build directory mkdir UNITTESTS/build.
>>>2.) Move to the build directory cd UNITTESTS/build.
>>>3.) Run CMake using a relative path to UNITTESTS folder as the argument. So from UNITTESTS/build use cmake ..:
>>>   a.) Add -DCMAKE_BUILD_TYPE=Debug for a debug build.
>>>   b.) Add -DCOVERAGE=True to add coverage compiler flags.
>>>4.) Run a Make program to build tests.
