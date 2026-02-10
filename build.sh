OPTIONS="$@"
fmake core/fmake.props $OPTIONS
fmake modules/fmake.props $OPTIONS

fmake example/gltf/fmake.props $OPTIONS

fmake example/triangle/fmake.props $OPTIONS
fmake example/ui/fmake.props $OPTIONS

