#!/bin/bash
if [ ! -d src/qutego.app/Contents/Resources/translations ]; then mkdir src/qutego.app/Contents/Resources/translations; fi
cp ../src/translations/*.qm src/qutego.app/Contents/Resources/translations/
