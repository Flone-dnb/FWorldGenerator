# How to use it
1. Download plugin from the releases tab or just clone the master branch.<br>
2. Unzip the downloaded archive and move the unzipped folder to your project's "Plugins" folder (create one if it doesn't exist).<br>
So your project's root directory will have the "Plugins" folder and the "FWorldGenerator" folder inside it, containing the .uplugin in there.<br>
3. If you have a Blueprint project you need to convert it to the C++ project by just doing "Add New C++ Class" inside the engine which will set up everything needed.<br>
4. Right-click your .uproject file and click "Generate Visual Studio project files".<br>
5. Run the project (it should build the plugin if necessary).
6. To access the plugin in the C++ code:<br>
In your project's .Build.cs file you must add "FWorldGenerator" to PublicDependencyModuleNames.<br>
Recompile.<br>
<br>
To use the generator just place an object of the FWGen class (search in the Modes tab in UE) in your level and configure its details.<br>
P.S. If you've added the FWGen object to the level and you can see it in the World Outliner but not on the screen then you should probably look up because the spawn point of this actor is the bottom of the generated world.<br>
To generate the world in the runtime you need to call the GenerateWorld() function from this object.