#!/bin/bash
otool -L src/q5go.app/Contents/MacOS/q5go
if [ ! -d src/q5go.app/Contents/Libs ]; then mkdir src/q5go.app/Contents/Libs; fi
cp -R src/thirdparty/QtRAR/src/libQtRAR.*dylib src/q5go.app/Contents/Libs/
if [ ! -d src/q5go.app/Contents/Resources/translations ]; then mkdir src/q5go.app/Contents/Resources/translations; fi
cp ../src/translations/*.qm src/q5go.app/Contents/Resources/translations/
install_name_tool -change libQtRAR.1.dylib @executable_path/../Libs/libQtRAR.1.dylib src/q5go.app/Contents/MacOS/q5go
otool -L src/q5go.app/Contents/MacOS/q5go

