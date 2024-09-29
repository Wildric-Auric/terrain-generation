mkdir build
mkdir .\build\bin

glslc src\shdrs\mrt.vert  -o build/bin/mrt.vert.spv
glslc src\shdrs\mrt.frag  -o build/bin/mrt.frag.spv
glslc src\shdrs\def.comp  -o build/bin/def.comp.spv
glslc src\shdrs\def.vert  -o build/bin/def.vert.spv
glslc src\shdrs\def.frag  -o build/bin/def.frag.spv
glslc src\shdrs\shadow.vert  -o build/bin/shadow.vert.spv
glslc src\shdrs\shadow.frag  -o build/bin/shadow.frag.spv

cd ./build
rm -f ALL_BUILD*
msbuild.exe -p:Configuration=Release
cd ..

cd ./build
mv  ./bin/vkUtilExample/Release/vkUtilExample.exe vkUtilExample.exe
cd ..
