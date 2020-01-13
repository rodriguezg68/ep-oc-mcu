# ep-oc-mcu extensions

Extensions build upon existing libraries and APIs.

## CMSIS-DSP Extensions

To enable the use of CMSIS-DSP extensions, remove the corresponding line in the `.mbedignore` file in this directory.

If you enable the CMSIS-DSP extensions, you must add the CMSIS repository to the build. For example, `cd` to the root directory of your project and execute `mbed add https://github.com/ARM-software/CMSIS_5.git`
