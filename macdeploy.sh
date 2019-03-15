#!/bin/bash
if [ ! -d src/q5go.app/Contents/Resources/translations ]; then mkdir src/q5go.app/Contents/Resources/translations; fi
cp ../src/translations/*.qm src/q5go.app/Contents/Resources/translations/
