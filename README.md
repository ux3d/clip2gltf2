[![](glTF.png)](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)

# Animation strip to glTF 2.0 converter

clip2gltf2 is a command line tool for converting an animation strip to glTF 2.0.  
[![](RunningGirl.png)](https://www.codeandweb.com/texturepacker/tutorials/how-to-create-a-sprite-sheet)  
The animation strip image has to be organized into a grid with rows and columns.  

Usage: `clip2gltf2.exe [-r 1 -c 6 -d 1.0 -i RunningGirl.png -m 0 -e 0.001 -g 10000.0]`  

`-r 1.0` Number of rows of the animation strip.  
`-c 6.0` Number of columns of the animation strip.  
`-d 1.0` Duration in seconds.  
`-i RunningGirl.png` Use another image beside the included animation strip.  
`-m 0` Used animation mode. `0` is the scale, `1` the translation, `2` the morph mode.  
`-e 0.001` Epsilon used for small scale number. Only used in mode `0`. `0.0` is not allowed to be used.  
`-g 10000.0` Googol used for large translation number. Only used in mode `1`.  

## Modes

### Scale

The default mode and only uses glTF 2.0 core features.  

### Translation

An alternative mode and also only uses glTF 2.0 core features.  

### Morph

This mode uses the optional feature to morph texture coordinates in glTF 2.0: 
[3.7.2.2. Morph Targets](https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#morph-targets)  

## Software Requirements

* C/C++ 17 compiler e.g. gcc or Visual C++
* [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2022-03/r/eclipse-ide-cc-developers) or  
* [CMake](https://cmake.org/)  

## Viewing

Try out the generated glTF in [Gestaltor](https://gestaltor.io/).  
