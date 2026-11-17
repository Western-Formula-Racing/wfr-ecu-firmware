# wfr-ecu-firmware
Firmware for the WFR27 ECU (and beyond, hopefully)

To make any changes, create a branch with your code changes are create a pull request, which needs at least 1 reviewer to merge to main.

To create a new component, create a folder as such:

-ComponentName
|-ComponentName.h
|-ComponentName.c
|-CMakeLists.txt

Your CMakeLists.txt file should contain the following:
```
idf_component_register( SRCS "ComponentName.c"
                        INCLUDE_DIRS "." "../../include"
                        REQUIRES <any components your component depends on>
)
```
