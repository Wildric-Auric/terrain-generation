mkdir build
mkdir .\build\bin

cd ./build
rm -f ALL_BUILD*
msbuild.exe -p:Configuration=Release
cd ..

cd ./build
mv  ./bin/vkUtilExample/Release/vkUtilExample.exe vkUtilExample.exe
cd ..
