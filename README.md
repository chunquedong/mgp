
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

1. install JVM, Fanx and build [fmake](https://github.com/chunquedong/fmake).

2. build [third-party dependencies](https://gitee.com/chunquedong/third-party):
```
sh build.sh -debug
```
3. build [jsonc](https://github.com/chunquedong/jsonc):
```
fan fmake libjsonc.props -debug
```
4. build src
```
fan fmake engine/fmake.props -debug
fan fmake engine/fmake.props -debug
```
5. generate IDE project file
```
sh build.sh -G -debug
```


## License
Licensed under the [GNU LESSER GENERAL PUBLIC LICENSE](https://www.gnu.org/licenses/lgpl.html)
