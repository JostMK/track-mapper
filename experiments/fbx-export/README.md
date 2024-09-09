This module requires the official [FBX SDK from Autodesk](https://aps.autodesk.com/developer/overview/fbx-sdk).

The files however are proprietary and can not be included in projects under an open source licence.
Therefor for compiling this module it is required to manually download and install the FBX SDK, so cmake can find the required files.

After successfully installing the FBX SDK make sure to set the cmake variable 
``FBX_ROOT="path/to/sdk"`` when compiling.