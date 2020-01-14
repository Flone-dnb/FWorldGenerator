# Parameters

You can configure the generator's parameters in the details tab.<br>
<br>

<h3>Preview</h3>
<ul>
    <li><b>Complex Preview</b> - generates one part of the world (every time a parameter is changed) as an example of how it's gonna look.</li>
    <li><b>Water Preview</b> - shows water plane (see Water parameters below) and its material in the Editor as an example of how it's gonna look.</li>
</ul>

<h3>Chunks</h3>
<ul>
    <li><b>Chunk Piece Row Count</b> - determines how much there is gonna be chunk pieces in a row of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Column Count</b> - determines how much there is gonna be chunk pieces in a column of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Size X</b> - determines the size of one chunk piece by the x-axis.</li>
    <li><b>Chunk Piece Size Y</b> - determines the size of one chunk piece by the y-axis.</li>
    <li><b>View Distance</b> - determines how much chunks will be always loaded. For example: ViewDistance == 1  ---  Always loaded chunks: 3x3, ViewDistance == 2  ---  Always loaded chunks: 5x5. And so on... The player is always in the central chunk.</li>
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
    <li><b>World Size</b> - when set to 0 the world will be infinite, when set to -1 the world will be 1 chunk size, when set to 1 the world will be 1 ViewDistance size, when set to 2 the world will be 2 ViewDistance sizes (for example if ViewDistance is 1, then the world will be 5x5). And so on...</li>
</ul>

<h3>Ground</h3>
<ul>
    <li><b>Ground Material</b> - determines the material which the terrain will have.</li>
    <li><b>First Material Max Relative Height</b> - Ground Material must consist out of 3 materials (layers), for example, it can contain: grass, rock, and snow (3 layers). This parameter determines the maximum relative height (from the actor's Z coordinate to the Generation Max Z From Actor Z) in the range from 0.0 to 1.0, on which terrain will contain the first layer (grass for example). See Terrain Material below on how to set this material. Note, that terrain which is higher than the second material height will have a 3rd layer of material (snow for example).</li>
    <li><b>Second Material Max Relative Heightl</b> - Ground Material must consist out of 3 materials (layers), for example, it can contain: grass, rock, and snow (3 layers). This parameter determines the maximum relative height (from the actor's Z coordinate to the Generation Max Z From Actor Z) in the range from 0.0 to 1.0, on which terrain will contain the second layer (rock for example). See Terrain Material below on how to set this material. Note, that terrain which is higher than the second material height will have a 3rd layer of material (snow for example).</li>
    <li><b>Material Height Max Deviation</b> - this is a possible deviation from values set to the "First Material Max Relative Height" and the "Second Material Max Relative Height". For example, if the "First Material Max Relative Height" is set to 0.5 and the "Material Height Max Deviation" is set to 0.1, then for every vertex of terrain value 0.5 can vary from 0.4 to 0.6.</li>
</ul>

<h3>Water</h3>
<ul>
    <li><b>Create Water</b> - determines is the world should be spawned in the water.</li>
    <li><b>Z Water Level in World</b> - determines the water height from actor's Z, from 0.0 to 1.0, where 0.0 - actor's Z and 1.0 - Generation Max Z From Actor Z.</li>
    <li><b>Water Size</b> - determines the size of the water in chunks.</li>
    <li><b>Water Material</b> - determines the material which the water will have.</li>
</ul>

# Terrain Material

To set up Ground Material correctly use can just add your material but it's just gonna cover all the terrain. If you want your materials to change depending on the terrain height, for example, if you want to have grass on low terrain height and snow on the high terrain height then you should create the material which is similar to the one below:
<p align="center">
  <img width="650" height="400" src="pics/Example Ground Material.jpg">
</p>
Let's guess you have a material of grass, you have base color, metallic, specular, etc. You can just create simple material with the Lerp_3Color node and add the Vertex Color node. Then copy-paste your material into this new simple material and connect your base color output to one of then Lerp_3Color inputs. Then you can copy-paste the Lerp and Vertex Color nodes and connect them to the Normal input of the material, for example.<br>
<br>
If you want your terrain to have only one material then you can just connect your textures to every input of the Lerp node.

# How to use it
<ol>
    <li>Download plugin from the releases tab or just clone the master branch.</li>
    <li>Unzip the downloaded archive and move the unzipped folder to your project's "Plugins" folder (create one if it doesn't exist). So your project's root directory will have the "Plugins" folder and the "FWorldGenerator" folder inside it, containing the .uplugin in there.</li>
    <li>If you have a Blueprint project you need to convert it to the C++ project by just doing "Add New C++ Class" inside the engine which will set up everything needed.</li>
    <li>If you have code in your project: Right-click your .uproject file and click "Generate Visual Studio project files".</li>
    <li>Run the project (it should build the plugin if necessary).</li>
    <li>To access the plugin in the C++ code: In your project's .Build.cs file you must add "FWorldGenerator" to PublicDependencyModuleNames. Recompile. And now you can include "FWGen.h".</li>
</ol>

To use the generator just place an object of the <b>FWGen</b> class (search in the Modes tab in UE) in your level and configure its details. Don't forget to enable Complex Preview if you want to see how it look.<br>
To generate the world in the runtime you need to call the <b>GenerateWorld()</b> function from this object.