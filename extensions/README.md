# ep-oc-mcu extensions

Extensions build upon existing libraries and APIs.

## CMSIS-DSP Extensions

To enable the use of CMSIS-DSP extensions, remove the corresponding line in the `.mbedignore` file in this directory.

If you enable the CMSIS-DSP extensions, you must add the CMSIS repository to the build. For example, `cd` to the root directory of your project and execute `mbed add https://github.com/ARM-software/CMSIS_5.git`. You will have to exclude some directories of the CMSIS repository from the build using a .mbedignore in your root directory. To only include CMSIS-DSP from the CMSIS_5 repo, your .mbedignore may look something like this:

```
CMSIS_5/Device/*
CMSIS_5/Core/*
CMSIS_5/Core_A/*
CMSIS_5/CoreValidation/*
CMSIS_5/DAP/*
CMSIS_5/Documentation/*
CMSIS_5/DoxyGen/*
CMSIS_5/Driver/*
CMSIS_5/NN/*
CMSIS_5/Pack/*
CMSIS_5/RTOS/*
CMSIS_5/RTOS2/*
CMSIS_5/Utilities/*
```
