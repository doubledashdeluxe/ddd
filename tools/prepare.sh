#!/bin/bash

set -e

if [[ $# -ne 1 ]] ; then
    echo 'Usage: prepare.sh <CW55xx_v2_10_SE.exe>'
    exit 1
fi

dir=$(mktemp -d)
echo $dir
7z x -o$dir $1 /Data1.cab
7z x -o$dir $dir/Data1.cab _44CD8EA541F44E56801091477F3DC9AA
7z x -o$dir $dir/Data1.cab _274B47372A7D4391B92F01E51B171A58
7z x -o$dir $dir/Data1.cab _4C8ADA37887647F5955B4FB0F716277F
mkdir -p tools/cw
mv $dir/_44CD8EA541F44E56801091477F3DC9AA tools/cw/license.dat
mv $dir/_274B47372A7D4391B92F01E51B171A58 tools/cw/lmgr11.dll
mv $dir/_4C8ADA37887647F5955B4FB0F716277F tools/cw/mwcceppc.exe
rm $dir/Data1.cab
rmdir $dir
