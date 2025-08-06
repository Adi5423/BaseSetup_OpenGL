This is the base level setup of using OpenGL,Glad,GLFW. 
For rendering a 2d Triangle.

from root run the command - 
```bash
cmake -S . -Build -G "Viusal Studio 17 2022"
```

this will generate the build files in the build folder.
Then open the build folder in Visual Studio and run the project.


then open the folder in visual studio or click the solution inside the build.
then build usib ctrl+shift+ b

then run using ctrl + f5
*if run not working manually run it from the folder dir. *


how to ignore submodules in git
```bash
git submodule deinit -f external/glfw
git rm -f external/glfw
rm -rf .git/modules/external/glfw
```
