@echo off

echo Building site...

call build_site.bat &&^
echo Pushing site... &&^
git add . && git commit -m "Site update: %time% on %date%"^
git push origin main
echo Site's been pushed: https://judah-caruso.github.io/