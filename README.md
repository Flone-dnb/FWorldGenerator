# Parameters

You can configure the generator's parameters in the details tab (or through the "set" functions of the generator - see below).<br>
<br>

<h3>Preview</h3>
<ul>
    <li><b>Complex Preview</b> - generates one part of the world (every time a parameter is changed) as an example of how it's gonna look.</li>
    <li><b>Draw Chunk Bounds</b> - draws bounds of the chunks if enabled. This could be useful to see triggers (see Load Unload Chunk Max Z parameter). Only works if the Complex Preview is enabled or if the game is started. Will not work if you have only 1 chunk.</li>
</ul>

<h3>Chunks</h3>
<ul>
    <li><b>Chunk Piece Row Count</b> (valid values range: [1, ...]) - determines how much there is gonna be chunk pieces in a row of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Column Count</b> (valid values range: [1, ...]) - determines how much there is gonna be chunk pieces in a column of one chunk. Because one chunk should be considered as a 2D matrix from the top view. You can change the View Mode in the Editor to Wireframe to see how this parameter changes the chunk.</li>
    <li><b>Chunk Piece Size X</b> (valid values range: [0.5, ...]) - determines the size of one chunk piece by the x-axis.</li>
    <li><b>Chunk Piece Size Y</b> (valid values range: [0.5, ...]) - determines the size of one chunk piece by the y-axis.</li>
    <li><b>View Distance</b> (valid values range: [1, ...]) - determines how much chunks will be always loaded. For example: ViewDistance == 1  ---  Always loaded chunks: 3x3, ViewDistance == 2  ---  Always loaded chunks: 5x5. And so on... The player is always in the central chunk.</li>
    <li><b>Load Unload Chunk Max Z</b> - every chunk has an invisible trigger box which determines if the player has entered another chunk, and if he did then we need to unload far chunks and load new ones. This parameter determines the max Z coordinate of this trigger box. If your player can fly very high make sure to set this value pretty high because at some point he can fly over this trigger box and new chunks will not load. You can see every chunk's trigger box if you use DrawChunkBounds option.</li>
</ul>

<h3>Generation</h3>
<ul>
    <li><b>Generation Frequency</b> (valid values range: [0.1, 64.0]) - frequency parameter of the Perlin noise which is used to generate the terrain.</li>
    <li><b>Generation Octaves</b> (valid values range: [1, 16]) - octaves parameter of the Perlin noise which is used to generate the terrain.</li>
    <li><b>Generation Seed</b> (valid values range: [0, ...]) - the seed which is determines the base for the generation. 0 - the seed will be generated randomly on every GenerateWorld() call (also in the Editor, if Complex Preview is enabled).</li>
    <li><b>Generation Max Z From Actor Z</b> (valid values range: [0.0, ...]) - determines the top point of the generated world relative to the actor's Z.</li>
    <li><b>Invert World</b> - inverts height in the terrain and now mountains become pits and vice versa. If you have Complex Preview enabled please set the seed to the non zero value to see better how the world is inverting.</li>
</ul>

<h3>World</h3>
<ul>
    <li><b>World Size</b> (valid values range: [-1, ...]) - when set to 0 the world will be infinite, when set to -1 the world will be 1 chunk size, when set to 1 the world will be 1 ViewDistance size, when set to 2 the world will be 2 ViewDistance sizes (for example if ViewDistance is 1, then the world will be 5x5). And so on...</li>
</ul>

<h3>Ground</h3>
<ul>
    <li><b>Ground Material</b> - determines the material which the terrain will have.</li>
    <li><b>First Material Max Relative Height</b> (valid values range: [0.0, 1.0] but always lower than Second Material Max Relative Height) - Ground Material must consist out of 3 materials (layers), for example, it can contain: grass, rock, and snow (3 layers). This parameter determines the maximum relative height (from the actor's Z coordinate to the Generation Max Z From Actor Z) in the range from 0.0 to 1.0, on which terrain will contain the first layer (grass for example). See Terrain Material below on how to set this material. Note, that terrain which is higher than the second material height will have a 3rd layer of material (snow for example).</li>
    <li><b>Second Material Max Relative Height</b> (valid values range: [0.0, 1.0] but always higher than First Material Max Relative Height) - Ground Material must consist out of 3 materials (layers), for example, it can contain: grass, rock, and snow (3 layers). This parameter determines the maximum relative height (from the actor's Z coordinate to the Generation Max Z From Actor Z) in the range from 0.0 to 1.0, on which terrain will contain the second layer (rock for example). See Terrain Material below on how to set this material. Note, that terrain which is higher than the second material height will have a 3rd layer of material (snow for example).</li>
    <li><b>Material Height Max Deviation</b> (valid values range: [0.0, 1.0]) - this is a possible deviation from values set to the "First Material Max Relative Height" and the "Second Material Max Relative Height". For example, if the "First Material Max Relative Height" is set to 0.5 and the "Material Height Max Deviation" is set to 0.1, then for every vertex of terrain value 0.5 can vary from 0.4 to 0.6.</li>
    <li><b>Terrain Cut Height From Actor Z</b> (valid values range: [0.0, 1.0]) - determines the height after which terrain will not be generated and will be flat.</li>
</ul>

<h3>Water</h3>
<ul>
    <li><b>Create Water</b> - determines is the world should be spawned in the water.</li>
    <li><b>Second Material Under Water</b> - determines if the underwater ground should have second material.</li>
    <li><b>Z Water Level in World</b> (valid values range: [0.0, 1.0]) - determines the water height from actor's Z, from 0.0 to 1.0, where 0.0 - actor's Z and 1.0 - Generation Max Z From Actor Z.</li>
    <li><b>Water Size</b> (valid values range: [1, ...]) - determines the size of the water in chunks.</li>
    <li><b>Water Material</b> - determines the material which the water will have.</li>
</ul>

<h3>Additional Steps</h3>
<ul>
    <li><b>Apply Ground Material Blend</b> - determines if the parameters from Ground Material Blend should be applied.</li>
    <li><b>Apply Slope Dependent Blend</b> - determines if the parameters from Slope Dependent Blend should be applied.</li>
</ul>

<h3>Ground Material Blend</h3>
<ul>
    <li><b>First Material On Second Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of first material appearance on the second material for a vertices.</li>
    <li><b>First Material On Third Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of first material appearance on the third material for a vertices.</li>
    <li><b>Second Material On First Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of second material appearance on the first material for a vertices.</li>
    <li><b>Second Material On Third Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of second material appearance on the third material for a vertices.</li>
    <li><b>Third Material On First Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of third material appearance on the first material for a vertices.</li>
    <li><b>Third Material On Second Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of third material appearance on the second material for a vertices.</li>
    <li><b>Increased Material Blend Probability</b> (valid values range: [0.0, 1.0]) - determines the probability of that materials on other materials will be bigger.</li>
</ul>

<h3>Slope Dependent Blend</h3>
<ul>
    <li><b>Min Slope Height Multiplier</b> (valid values range: [0.0, 1.0]) - defines the minimum height difference between two close vertices as (GenerationMaxZFromActorZ * this value) for which the second material will be applied. You usually should keep this value pretty low but not too low.</li>
</ul>

<h3>Spawning Objects</h3>
<ul>
    <li><b>Divide Chunk X Count</b> (valid values range: [1, ...]) - every chunk is divided into cells, any cell may have an object in it. This parameter determines the X size of the cell. For example: if your "Chunk Piece Size X" = 300 and "Chunk Piece Column Count" = 300, then your chunk X size will be 90000 and if you set "Divide Chunk X Count" to the 300 then your chunk will have 300 cells by X-axis.</li>
    <li><b>Divide Chunk Y Count</b> (valid values range: [1, ...]) - every chunk is divided into cells, any cell may have an object in it. This parameter determines the Y size of the cell. For example: if your "Chunk Piece Size Y" = 300 and "Chunk Piece Row Count" = 300, then your chunk Y size will be 90000 and if you set "Divide Chunk Y Count" to the 300 then your chunk will have 300 cells by Y-axis.</li>
</ul>

# Terrain Material

To set up Ground Material correctly you can just add your material but it's just gonna cover all the terrain. If you want your materials to change depending on the terrain height, for example, if you want to have grass on low terrain height and snow on the high terrain height then you should create the material which is similar to the one below:
<p align="center">
  <img width="650" height="400" src="pics/Example Ground Material.jpg">
</p>
Let's guess you have a material of grass, you have base color, metallic, specular, etc. You can just create simple material with the Lerp_3Color node and add the Vertex Color node. Then copy-paste your material into this new simple material and connect your base color output to one of then Lerp_3Color inputs. Then you can copy-paste the Lerp and Vertex Color nodes and connect them to the Normal input of the material, for example.<br>
<br>
If you want your terrain to have only one material then you can just connect your textures to every input of the Lerp node.

# How to use it
<ol>
    <li>Download the plugin from the releases tab or just clone the master branch.</li>
    <li>Unzip the downloaded archive and move the unzipped folder to your project's "Plugins" folder (create one if it doesn't exist). So your project's root directory will have the "Plugins" folder and the "FWorldGenerator" folder inside it, containing the .uplugin in there.</li>
    <li>If you have a Blueprint project you need to convert it to the C++ project by just doing "Add New C++ Class" inside the engine which will set up everything needed.</li>
    <li>If you have code in your project: Right-click your .uproject file and click "Generate Visual Studio project files".</li>
    <li>Run the project (it should build the plugin if necessary).</li>
    <li>To access the plugin in the C++ code: In your project's .Build.cs file you must add "FWorldGenerator" to PublicDependencyModuleNames. Recompile. And now you can include "FWGen.h".</li>
</ol>

To use the generator just place an object of the <b>FWGen</b> class (search in the Modes tab in UE) in your level and configure its details. Don't forget to enable Complex Preview if you want to see how it looks.<br>
To generate the world in the runtime you need to call the <b>GenerateWorld()</b> function from this object.

# How to spawn custom objects in world randomly

If you want FWorldGenerator to spawn your custom objects in the world such as trees, then you need to call BindFunctionToSpawn function and pass a name of the function which will be called on GenerateWorld() and the parameter of this function will be position in which you will need to spawn an object by yourself.
<br>BindFunctionToSpawn takes 3 arguments:
<ul>
    <li><b>Function Owner</b> - class in which the function is defined.</li>
    <li><b>Function Name</b> - name of the function which will be called. In this function, you will spawn your object in the world.</li>
    <li><b>Probability To Spawn</b> - probability from 0.0 to 1.0 with which this function will be called (on every chunk cell). See "Spawning Objects" above.</li>
</ul>
"Function Name" should be a function that accepts FTransform as the only parameter.
<p align="center">
  <img width="650" height="400" src="pics/Example BindFunctionToSpawn 1.jpg">
  <img width="650" height="400" src="pics/Example BindFunctionToSpawn 2.jpg">
</p>
<br>
You need to control your probabilities manually, so that the probabilities of different objects add up to 1.0.

# Functions

You can set the parameters of the generator in the Editor but you also can do this in the Blueprints (or C++ code) by calling functions from the "FWorldGenerator" category. Functions usually called like this: "Set(parameter name)". Functions that return bool will return true if your passed value was incorrect or out of valid range. See Parameters (above) to see their valid values range.
