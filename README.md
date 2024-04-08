
## MGP

MGP is a lightweight 3D game engine.


#### Features

- Rendering: OpenGL ES rendering backends with PBR, CSM
- PostEffect: SSAO, bloom, FXAA
- Assets: Importing GLTF
- UI: Build-in Declarative UI system
- Text: Dynamic load TTF fonts with unicode supporting
- Animation: Animation system with skeletal character animation
- Terrain: Height map based terrains with LOD
- Physics: Powered by Bullet
- Audio: 3D audio system with WAV and OGG support
- Lua like script (unmaintained)

#### Build

1. install JVM and [Fanx](https://github.com/fanx-dev/fanx/releases) 

2. build and install [fmake](https://github.com/)

3. build [third-party dependencies](https://gitee.com/chunquedong/third-party):
```
sh build.sh -debug
```
4. build [jsonc](https://github.com/chunquedong/jsonc):
```
fan fmake libjsonc.props -debug
```
5. build sources
```
fan fmake core/fmake.props -debug
fan fmake modules/fmake.props -debug
```
6. generate IDE project file
```
sh build.sh -G -debug
```

#### WebAssembly

fanx/etc/fmake/config.props:
```
compiler=gcc
gcc.home=/C:/soft/emsdk/upstream/emscripten/
gcc.name@{cpp}=emcc.bat @{cppflags} -pthread
gcc.name@{c}=emcc.bat @{cflags} -pthread
gcc.ar=emar.bat
gcc.link=emcc.bat -pthread
gcc.exe=@{gcc.link} @{linkflags} -o @{outFile}.js @{gcc.objList} @{gcc.libDirs} @{gcc.libNames}
```

```
fan fmake core/fmake_wasm.props
fan fmake modules/fmake_wasm.props
```

## License
Licensed under the [GNU LESSER GENERAL PUBLIC LICENSE](https://www.gnu.org/licenses/lgpl.html)
