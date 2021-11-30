[![](glTF.png)](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)

# Animation strip to glTF 2.0 converter

clip2gltf2 is a command line tool for converting an animation strip to glTF 2.0.  
[![](pngegg.png)](https://www.codeandweb.com/texturepacker/tutorials/how-to-create-a-sprite-sheet)  
It uses the optional feature to morph texture coordinates in glTF 2.0: 
[3.7.2.2. Morph Targets](https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#morph-targets)  

Usage: `clip2gltf2.exe [-r 1 -c 6 -f 15.0 -i pngegg.png]`

`-r 1.0` Number of rows of the animation strip.  
`-c 6.0` Number of columns of the animation strip.  
`-f 15.0` Frames per second to be used.  
`-i pngegg.png` Use another image beside the included animation strip.  


## Software Requirements

* C/C++ 17 compiler e.g. gcc or Visual C++
* [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2021-03/r/eclipse-ide-cc-developers) or  
* [CMake](https://cmake.org/)  


## Import the generated glTF

Import the generated glTF in e.g. [Gestaltor](https://gestaltor.io/) and reuse in your scene.  

