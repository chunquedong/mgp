
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
