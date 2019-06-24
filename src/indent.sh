#!/bin/bash

INDENT_FLAGS="--style=1tbs --indent=spaces --add-braces --break-return-type --convert-tabs --max-code-length=80 "

if ! type astyle 2>/dev/null; then
    echo "Please run the following command to install astyle:"
    echo "sudo apt install astyle"
    exit 1
fi


find . -name *.cpp -exec astyle $INDENT_FLAGS {} \;
