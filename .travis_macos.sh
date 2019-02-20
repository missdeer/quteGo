#!/bin/bash
project_dir=$(pwd)

# Complain when not in Travis environment
if [ -z ${TRAVIS_COMMIT+x} ]; then
    echo "This script is intended to be used only in Travis CI environment."
    exit 1
fi

brew update > /dev/null
brew install qt

QTDIR="/usr/local/opt/qt"
PATH="$QTDIR/bin:$PATH"
LDFLAGS=-L$QTDIR/lib
CPPFLAGS=-I$QTDIR/include

# Build your app
cd ${project_dir}
mkdir build
cd build
qmake -v
qmake CONFIG-=debug CONFIG+=release ../src/q5go.pro
if [ $? -ne 0 ]; then 
    exit 1; 
fi

make -j2
if [ $? -ne 0 ]; then 
    exit 1; 
fi

git clone https://github.com/aurelien-rainone/macdeployqtfix.git

$QTDIR/bin/macdeployqt q5go.app
python macdeployqtfix/macdeployqtfix.py q5go.app/Contents/MacOS/q5go $QTDIR

mkdir -p distrib/q5Go
cd distrib/q5go
mv ../../q5go.app ./
cp "${project_dir}/README.md" "README.md"
echo "${version}" > version
echo "${TRAVIS_COMMIT}" >> version

ln -s /Applications ./Applications

cd ..
hdiutil create -srcfolder ./q5Go -format UDBZ ./q5Go.dmg
mv q5Go.dmg q5Go-${version}-x64.dmg
cd ..

exit 0
