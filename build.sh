OPTIONS="$@"
fan fmake core/fmake.props $OPTIONS
fan fmake modules/fmake.props $OPTIONS

fan fmake example/gltf/fmake.props $OPTIONS

fan fmake example/triangle/fmake.props $OPTIONS
fan fmake example/ui/fmake.props $OPTIONS

