@echo off

call build.bat

echo *** Creating site...
rmdir /Q /S docs
website.com
echo *** Done!
