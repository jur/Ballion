#!/bin/bash
PND="${1:-ballion}"
WORKDIR="$PWD"
cd "`dirname \"$0\"`" || exit -1
SCRIPTDIR="$PWD"
cd "$WORKDIR" || exit -1
cp "$SCRIPTDIR/icon.png" "$PND/" || exit -1
cp "$SCRIPTDIR/PXML.xml" "$PND/" || exit -1
cp "$SCRIPTDIR/license.txt" "$PND/" || exit -1
cp "$SCRIPTDIR/levelformat.txt" "$PND/" || exit -1
cp "$SCRIPTDIR/screenshot.png" "$PND/" || exit -1
xmllint --noout --schema "$SCRIPTDIR/PXML_schema.xsd" "$PND/PXML.xml"
if [ -e "$PND.tmp" ]; then
	rm "$PND.tmp"
fi
if [ -e "$PND.tmp" ]; then
	rm "$PND.pnd"
fi
mksquashfs "$PND/" "$PND.tmp" || exit -1
cat "$PND.tmp" "$PND/PXML.xml" "$PND/icon.png" > "$PND.pnd" || exit -1
rm "$PND.tmp"
