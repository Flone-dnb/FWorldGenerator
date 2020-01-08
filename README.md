# Parameters
You can configure the generator's parameters in the details tab.<br>
<br>
<b>Preview</b><br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Complex Preview</em> - generates one part of the world (every time a parameter is changed) as an example of how it's gonna look.<br>
<b>Chunks</b><br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Chunk Piece Row Count</em> - determines how much there is gonna be chunk pieces in a row of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Chunk Piece Column Count</em> - determines how much there is gonna be chunk pieces in a column of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Chunk Piece Size X</em> - determines the size of one chunk piece by the x-axis.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Chunk Piece Size Y</em> - determines the size of one chunk piece by the y-axis.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>View Distance</em> - determines how much chunks will be always loaded. For example: ViewDistance == 0  ---  Always loaded only one chunk, ViewDistance == 1  ---  Always loaded chunks: 3x3, ViewDistance == 2  ---  Always loaded chunks: 5x5. The player is always in the central chunk.<br>
<b>Generation</b><br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Generation Frequency</em> - frequency parameter of the Perlin noise which is used to generate the terrain.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Generation Octaves</em> - octaves parameter of the Perlin noise which is used to generate the terrain.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Generation Seed</em> - the seed which is determines the base for the generation. 0 - the seed will be generated randomly on every GenerateWorld() call (also in the Editor, if Complex Preview is enabled).<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Generation Max Z From Actor Z</em> - determines the top point of the generated world relative to the actor's Z.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Invert World</em> - inverts height in the terrain and now mountains become pits and vice versa. If you have Complex Preview enabled please set the seed to the non zero value to see better how the world is inverting.<br>
<b>Ground</b><br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Ground Material</em> - determines the material which the terrain will have.<br>
<b>Water</b><br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Create Water</em> - determines is the world should be spawned in the water.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Z Water Level in World</em> - determines the water height from actor's Z, from 0.0 to 1.0, where 0.0 - actor's Z and 1.0 - Generation Max Z From Actor Z.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Water Size</em> - determines the size of the water in chunks.<br>
&nbsp;&nbsp;&nbsp;&nbsp;<em>Water Material</em> - determines the material which the water will have.<br>

# How to use it
1. Download plugin from the releases tab or just clone the master branch.<br>
2. Unzip the downloaded archive and move the unzipped folder to your project's "Plugins" folder (create one if it doesn't exist).<br>
So your project's root directory will have the "Plugins" folder and the "FWorldGenerator" folder inside it, containing the .uplugin in there.<br>
3. If you have a Blueprint project you need to convert it to the C++ project by just doing "Add New C++ Class" inside the engine which will set up everything needed.<br>
4. If you have code in your project: Right-click your .uproject file and click "Generate Visual Studio project files".<br>
5. Run the project (it should build the plugin if necessary).
6. To access the plugin in the C++ code:<br>
In your project's .Build.cs file you must add "FWorldGenerator" to PublicDependencyModuleNames.<br>
Recompile.<br>
And now you can include "FWGen.h".<br>
<br>
To use the generator just place an object of the FWGen class (search in the Modes tab in UE) in your level and configure its details.<br>
P.S. If you've added the FWGen object to the level and you can see it in the World Outliner but not on the screen then you should probably look up because the spawn point of this actor is the bottom of the generated world.<br>
To generate the world in the runtime you need to call the GenerateWorld() function from this object.