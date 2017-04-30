#!/bin/bash
set -e

# change to the directory where the script is stored
cd `dirname $0`

if [ -e skia ]; then
	echo "Skipping Skia download because it's been downloaded already."
else
	# TODO: use a fixed (verified-working) revision
	git clone https://skia.googlesource.com/skia.git
	(
		cd skia
		tools/git-sync-deps
	)
fi
