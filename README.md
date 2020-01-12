# Parameters

You can configure the generator's parameters in the details tab.<br>
<br>

<h3>Preview</h3>
<ul>
    <li><b>Complex Preview</b> - generates one part of the world (every time a parameter is changed) as an example of how it's gonna look. Please, disable Complex Preview if you're gonna test world generation in-game. And use this only to quickly check how it all looks.</li>
</ul>

<h3>Chunks</h3>
<ul>
    <li><b>Chunk Piece Row Count</b> - determines how much there is gonna be chunk pieces in a row of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Column Count</b> - determines how much there is gonna be chunk pieces in a column of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Size X</b> - determines the size of one chunk piece by the x-axis.</li>
    <li><b>Chunk Piece Size Y</b> - determines the size of one chunk piece by the y-axis.</li>
    <li><b>View Distance</b> - determines how much chunks will be always loaded. For example: ViewDistance == 1  ---  Always loaded chunks: 3x3, ViewDistance == 2  ---  Always loaded chunks: 5x5. The player is always in the central chunk.</li>
</ul>

<h3>Generation</h3>
<ul>
    <li><b>Generation Frequency</b> - frequency parameter of the Perlin noise which is used to generate the terrain.</li>
    <li><b>Generation Octaves</b> - octaves parameter of the Perlin noise which is used to generate the terrain.</li>
    <li><b>Generation Seed</b> - the seed which is determines the base for the generation. 0 - the seed will be generated randomly on every GenerateWorld() call (also in the Editor, if Complex Preview is enabled).</li>
    <li><b>Generation Max Z From Actor Z</b> - determines the top point of the generated world relative to the actor's Z.</li>
    <li><b>Invert World</b> - inverts height in the terrain and now mountains become pits and vice versa. If you have Complex Preview enabled please set the seed to the non zero value to see better how the world is inverting.</li>
</ul>

<h3>World</h3>
<ul>
    <li><b>World Size</b> - when set to 0 the world will be infinite, when set to -1 the world will be 1 chunk size, when set to 1 the world will be 1 ViewDistance size, when set to 2 the world will be 2 ViewDistance sizes (for example if ViewDistance is 1, then the world will be 5x5).</li>
</ul>

<h3>Ground</h3>
<ul>
    <li><b>Ground Material</b> - determines the material which the terrain will have.</li>
</ul>

<h3>Water</h3>
<ul>
    <li><b>Create Water</b> - determines is the world should be spawned in the water.</li>
    <li><b>Z Water Level in World</b> - determines the water height from actor's Z, from 0.0 to 1.0, where 0.0 - actor's Z and 1.0 - Generation Max Z From Actor Z.</li>
    <li><b>Water Size</b> - determines the size of the water in chunks.</li>
    <li><b>Water Material</b> - determines the material which the water will have.</li>
</ul>

# How to use it
<ol>
    <li>Download plugin from the releases tab or just clone the master branch.</li>
    <li>Unzip the downloaded archive and move the unzipped folder to your project's "Plugins" folder (create one if it doesn't exist). So your project's root directory will have the "Plugins" folder and the "FWorldGenerator" folder inside it, containing the .uplugin in there.</li>
    <li>If you have a Blueprint project you need to convert it to the C++ project by just doing "Add New C++ Class" inside the engine which will set up everything needed.</li>
    <li>If you have code in your project: Right-click your .uproject file and click "Generate Visual Studio project files".</li>
    <li>Run the project (it should build the plugin if necessary).</li>
    <li>To access the plugin in the C++ code: In your project's .Build.cs file you must add "FWorldGenerator" to PublicDependencyModuleNames. Recompile. And now you can include "FWGen.h".</li>
</ol>

To use the generator just place an object of the <b>FWGen</b> class (search in the Modes tab in UE) in your level and configure its details. Don't forget to enable Complex Preview if you want to see how it look.<br><br>

To generate the world in the runtime you need to call the <b>GenerateWorld()</b> function from this object.