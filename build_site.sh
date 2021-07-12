#!/usr/bin/env sh

./build.sh

echo *** Building site...
rm -rf ./docs

./website.com.dbg && \
cp CNAME ./docs/

echo *** Site built!
