﻿# FWorldGenerator

<p align="center">
  <img width="1000" height="600" src="pics/FWorldGenerator.jpg">
</p>
Example of the infinite world generated with the default settings and custom user objects by the FWorldGenerator (note that the world is loading by chunks).
<br>
(far away objects were culled for performance reasons)
<br>
<br>
If you would like to use the FWorldGenerator, please, read this fully to avoid any mistakes.

# Parameters

You can configure the generator's parameters in the details tab (or through the "set" functions of the generator - see below).
<br>

<h3>Preview</h3>
<ul>
    <li><b>Complex Preview (unstable, may cause crashes)</b> - generates one part of the world (every time a parameter is changed) as an example of how it's gonna look.</li>
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
    <li><b>Max Offset By X</b> (valid values range: [0.0, 1.0]) - determines the random offset (in location) by X-axis on spawn, where 1.0 means that the maximum offset can be cell size by X, and 0.5 - half of the cell size by X.</li>
    <li><b>Max Offset By Y</b> (valid values range: [0.0, 1.0]) - determines the random offset (in location) by Y-axis on spawn, where 1.0 means that the maximum offset can be cell size by Y, and 0.5 - half of the cell size by Y.</li>
    <li><b>Max Rotation</b> (valid values range: [0.0, ...]) - determines the maximum random rotation by Z-axis in the range [minus this value; this value].</li>
    <li><b>Max Z Diff In Cell</b> - if the difference of the Z coordinates on the bound of the cell will be more than this value, then the cell will not be used (and nothing will be spawn in it).</li>
</ul>
</ul>

# Terrain Material

To set up Ground Material correctly you can just add your material but it's just gonna cover all the terrain. If you want your materials to change depending on the terrain height, for example, if you want to have grass on low terrain height and snow on the high terrain height then you should create the material which is similar to the one below:
<p align="center">
  <img width="700" height="400" src="pics/Example Ground Material.jpg">
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
<br>
To generate the world in the runtime you need to call the <b>GenerateWorld()</b> function from this object. Passed Actor (character) will be teleported to the central chunk (Z-coord will be the same).<br>
<br>
You need to be sure that you are creating your character in the center (central chunk) of the world. You can check chunk bounds by enabling the Draw Chunk Bounds option.<br>
<br>
BE AWARE that generating a world (probably with your custom actors (see below)) can take a while! Don't think that it stuck somewhere or froze, just take your time. For example, generating a world with the default parameters without spawning objects may take up to a minute (PIE).<br>
<br>
Do not worry, you will not fall from the generated terrain, there are blocking volumes at the borders.

# Load/unload chunks logic

Chunks of the world will be loaded and unloaded depending on where the player is going. But before calling GenerateWorld() you need to call AddOverlapToActorClass() function and add a class that will be considered as a player. You may add more than one class. Use RemoveOverlapToActorClass() to remove classes from considered as a player.

# How to spawn custom objects in world randomly

If you want FWorldGenerator to spawn your custom objects in the world such as trees, then you need to call BindFunctionToSpawn function and pass a name of the function which will be called on GenerateWorld() and the parameter of this function will be position in which you will need to spawn an object by yourself.
<br>BindFunctionToSpawn takes 3 arguments:
<ul>
    <li><b>Function Owner</b> - class in which the function is defined.</li>
    <li><b>Function Name</b> - name of the function which will be called. In this function, you will spawn your object in the world.</li>
    <li><b>Layer</b> - Just like in Materials, the material layer can be 0.0f (grass, for example), 0.5f (rock, for example), 1.0f (snow, for example). If you want to spawn something in water (underwater) then this value should be -0.5f. If you want to spawn a grass mesh then you should probably set this value to 0.0f.</li>
    <li><b>Probability To Spawn</b> - probability from 0.0 to 1.0 with which this function will be called (on every chunk cell). See "Spawning Objects" above.</li>
    <li><b>Is Blocking</b> - should the objects of this class "block" the cell on which they are spawn so that no other object will be spawn on this cell? For example, a grass object usually should not block the cell.</li>
</ul>
"Function Name" should be a function that accepts Transform, Integer64 and Integer64 (which determine the coordinates of the chunk).<br>
Here is the example of how you want to spawn many grass meshes on the terrain:<br>
(click to see in full size)
<p align="center">
  <img width="800" height="200" src="pics/Example BindFunctionToSpawn 1.png">
</p>
And Spawn Grass function:
<br>
<p align="center">
  <img width="600" height="250" src="pics/Example BindFunctionToSpawn 2.jpg">
</p>
<br>
And don't forget to set the culling in the Hierarchical Instanced Static Mesh Component if you don't want to melt your PC.
<br>
If you want to spawn something which is not like grass or trees (i.e. not through Hierarchical Instanced Static Mesh) then be sure to set the "Always Spawn, Ignore Collisions" option in the Collision Handling Override param in the SpawnActor node OR spawn your actors a little higher than the given transform.<br>
<br>
And if you want to get the location of the free cell, then you can call the GetFreeCellLocation function. The SetBlocking param, if set to true, will make this cell block, so on the next GetFreeCellLocation call, you will not get this cell.
<br>
Please note that if you want to call GenerateWorld() again you need to delete all actors that you've spawned.<br>
<br>
If your character moves then the generator will load new chunks and unload far ones. So, you will need to despawn all actors on those far chunks.
Using the BindFunctionToDespawnActors() function you can bind a function to despawn actors.
The function should accept 2 Integer64 values as an input (x and y of chunk that gets unloaded).
It's up to you to decide on how to despawn them. You also can you GetChunkXByLocation() and GetChunkYByLocation() functions to see if any of your non-static objects are on the chunk that is despawning.<br>
<br>
Also, you need to control the probabilities of each layer manually so that the probabilities of one layer add up to 1.0. And so you will have a probability of 4.0 in total (for 4 layers).<br>
<br>
Note, if you are using Hierarchical Instanced Static Mesh Component (like shown in the picture above) and your actors don't have a material, then you should check the Used with Instanced Static Meshes option in the Usage category inside your material.

# Save/Load world params to/from file

Inside the Editor in the "Save / Load" section of the FWGen Details tab, you can find a field for the path of the save file: "Path to Save File". Type in a path, for example: "D:/Home/save" and then click on the "Save Params to File Right Now" to make this value "true". After that, your file will be saved with the .fwgs extension. If you want to read params from this file you just need to add the ".fwgs" extension to your "D:/Home/save" to make it "D:/Home/save.fwgs" and then click on the "Read Params from File Right Now". If the path is correct and this save file has a supported save version, then the "Last Save Load Operation Status" value will be set to "true" automatically. For now, save/load operations is done like this, I didn't spend too much time on this, but in the future updates maybe this operation will have their own windows.<br>
<br>
You can visit our site fwg-save-share.xyz where you can share your world params save file and download others'.

# Save/Load world

Although there are still a lot of things to do, FWorldGenerator has functions to save/load the world. GenerateWorld() function has 3 params which are X, Y and a pointer to a character. You can remember your X and Y by calling the GetCentralChunkX() GetCentralChunkY() functions and then continue playing from this chunk by entering those coordinates to the GenerateWorld() function and passing the pointer to a character which will be teleported to the center of this world.

# Functions

You can set the parameters of the generator in the Editor but you also can do this in the Blueprints (or C++ code) by calling functions from the "FWorldGenerator" category. Functions usually called like this: "Set(parameter name)". Functions that return bool will return true if your passed value was incorrect or out of valid range. See Parameters (above) to see their valid values range.
