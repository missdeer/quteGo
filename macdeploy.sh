#!/bin/bash
otool -L src/q5go.app/Contents/MacOS/q5go
mkdir src/q5go.app/Contents/Libs
cp -R src/thirdparty/QtRAR/src/libQtRAR.*dylib src/q5go.app/Contents/Libs/
install_name_tool -change libQtRAR.1.dylib @executable_path/../Libs/libQtRAR.1.dylib src/q5go.app/Contents/MacOS/q5go

