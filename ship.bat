@echo off

call build.bat

echo *** Creating site...
rmdir /Q /S site
website.com
echo *** Done!
