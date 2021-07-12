@echo off

call build.bat

echo *** Building site...

rmdir /Q /S docs
website.com &&^
copy CNAME docs\

echo *** Site built!
