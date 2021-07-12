#!/usr/bin/env bash

./build_site.sh && \
echo judahcaruso.com > docs/CNAME && \

echo *** Pushing site... && \
git add . && git commit -m "Site update: $(date)" && \
git push origin main

echo *** Site pushed! https://judah-caruso.github.io/
