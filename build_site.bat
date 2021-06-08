@echo off

call build.bat

echo *** Building site...
rmdir /Q /S docs
website.com
rem mkdir docs\images
rem copy _images docs\images
echo *** Site built!
