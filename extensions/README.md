# ep-oc-mcu extensions

Extensions build upon existing libraries and APIs.

## CMSIS-DSP Extensions

To enable the use of CMSIS-DSP extensions, remove the corresponding line in the `.mbedignore` file in this directory.

If you enable the CMSIS-DSP extensions, you must add the CMSIS repository to the build. For example, `cd` to the root directory of your project and execute `mbed add https://github.com/ARM-software/CMSIS_5.git`. You will have to exclude some directories of the CMSIS repository from the build using a .mbedignore in your root directory. To only include CMSIS-DSP from the CMSIS_5 repo, your .mbedignore may look something like this:

```
CMSIS_5/Device/*
CMSIS_5/CMSIS/Core/*
CMSIS_5/CMSIS/Core_A/*
CMSIS_5/CMSIS/CoreValidation/*
CMSIS_5/CMSIS/DAP/*
CMSIS_5/CMSIS/Documentation/*
CMSIS_5/CMSIS/DoxyGen/*
CMSIS_5/CMSIS/Driver/*
CMSIS_5/CMSIS/NN/*
CMSIS_5/CMSIS/Pack/*
CMSIS_5/CMSIS/RTOS/*
CMSIS_5/CMSIS/RTOS2/*
CMSIS_5/CMSIS/Utilities/*
CMSIS_5/CMSIS/DSP/ComputeLibrary/*
CMSIS_5/CMSIS/DSP/DSP_Lib_TestSuite/*
CMSIS_5/CMSIS/DSP/Examples/*
CMSIS_5/CMSIS/DSP/Lib/*
CMSIS_5/CMSIS/DSP/Platforms/*
CMSIS_5/CMSIS/DSP/Projects/*
CMSIS_5/CMSIS/DSP/PythonWrapper/*
CMSIS_5/CMSIS/DSP/Scripts/*
CMSIS_5/CMSIS/DSP/Testing/*
CMSIS_5/CMSIS/DSP/Toolchain/*
CMSIS_5/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c
CMSIS_5/CMSIS/DSP/Source/CommonTables/CommonTables.c
CMSIS_5/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c
CMSIS_5/CMSIS/DSP/Source/ControllerFunctions/ControllerFunctions.c
CMSIS_5/CMSIS/DSP/Source/FastMathFunctions/FastMathFunctions.c
CMSIS_5/CMSIS/DSP/Source/FilteringFunctions/FilteringFunctions.c
CMSIS_5/CMSIS/DSP/Source/MatrixFunctions/MatrixFunctions.c
CMSIS_5/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctions.c
CMSIS_5/CMSIS/DSP/Source/SupportFunctions/SupportFunctions.c
CMSIS_5/CMSIS/DSP/Source/TransformFunctions/TransformFunctions.c
CMSIS_5/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.S
```
