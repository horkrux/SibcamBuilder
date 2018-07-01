# Dark Souls 1 SibcamBuilder
## Description
This tool reads out animation data from a supplied .damnhavok file (created with SSFADF from .hkx) and uses it to build a camera animation file (.sibcam) to be used in custom-made or existing cutscenes.
Changing the FOV is currently not supported, but fixed to a default value.

## How to Use
To get the correct camera orientation, export an existing cutscene havok animation as .fbx with Z up with Noesis and use that a base. 
Coordinates need to be 100 times larger than they are supposed to be ingame. When exporting the camera animation with the Havok Tools, the X axis needs to be mirrored.
Be sure to only export the camera animation. This binary file will then have to be converted to the readable damnhavok format with SSFADF.

./SibcamBuilder [-f] "path_to_file"

-f - change camera type from default "Targeting Cam" to "Free Cam"