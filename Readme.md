This is the base level setup of using OpenGL,Glad,GLFW. 
For rendering a 2d Triangle.

# from root run the command - 
```bash
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
```
This will generate the build files in the build folder.

Then to do the linking with - 
```bash
cmake --build build
```
This will build the project and create an executable in the build folder. (statically linked), easily portable.


# Check the dependency of the exe with the command - 
```bash
dumpbin /dependents Minecraft_Clone.exe
```
THis command only works in Visual Studio Installed , if installed and not working, search dumbpin and open folder and add to PATH.
or easily copy the dumpbin.exe to the exe folder , instead of adding to PATH.
)

# all the git submodules - 

For glad dowloaded from the official site

[submodule "external/stb"]
	path = external/stb
	url = https://github.com/nothings/stb.git
[submodule "external/glm"]
	path = external/glm
	url = https://github.com/g-truc/glm.git
[submodule "external/glfw"]
	path = external/glfw
	url = https://github.com/glfw/glfw.git
