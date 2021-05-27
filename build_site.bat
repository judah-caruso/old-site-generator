@echo off

call build.bat

echo *** Building site...
rmdir /Q /S docs
website.com
echo *** Site built!
