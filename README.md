# Dark Souls 1/Demon's Souls SibcamBuilder
## Description
This tool reads out animation data from a supplied .damnhavok file (created with SSFADF from .hkx) and optionally FoV animation data from an .xaf (created with 3DS Max), and uses both to build a camera animation file (.sibcam) to be used in custom-made or existing cutscenes. It was only tested for the PC version for DS1, but should now work on all versions and also Demon's Souls. 

## How to Use
To get the correct camera orientation, export an existing cutscene havok animation as .fbx with Z up with Noesis and use that a base. 
Coordinates need to be 100 times larger than they are supposed to be ingame. When exporting the camera animation with the Havok Tools, the X axis needs to be mirrored.
Be sure to only export the camera animation. This binary file will then have to be converted to the readable damnhavok format with SSFADF.
FoV changes will automically be imported from an .xaf (should it be present) sharing the same name as the damnhavok file. To create a suitable .xaf, use "Save Animation" for the desired camera in 3DS Max.

./SibcamBuilder [-c] "path_to_file"

-c - change output to big-endian, used by console versions of Dark Souls, as well as Demon's Souls
