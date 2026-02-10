
### Build

#### Build Dependencies
1. install JVM

2. build and install [fmake](https://github.com/)

3. build [third-party dependencies](https://gitee.com/chunquedong/third-party):
```
sh build.sh -debug
```
1. build [jsonc](https://github.com/chunquedong/jsonc):
```
fmake libjsonc.props -debug
```
1. build sric and wase

build [Wase](https://github.com/sric-language/wase) or remove WASE_UI in modules/fmake.props

#### Build MGP
1. build
```
fmake core/fmake.props -debug
fmake modules/fmake.props -debug
```
2. generate IDE project file
```
sh build.sh -G -debug
```

### WebAssembly

```
fmake core/fmake_wasm.props
fmake modules/fmake_wasm.props
```
