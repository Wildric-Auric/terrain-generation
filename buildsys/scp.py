import os
import time

i = time.time()

srcDir = "src/shdrs"
binDir = "build/bin"

err = 0
num = 0
for filename in os.listdir(srcDir):
    n = srcDir + "/" + filename
    d = binDir + "/" + filename 
    num = num + 1
    if os.system("glslc " + n + " -o " +  d + ".spv"):
        print("SHADER COMPILATION ERROR!")
        err = 1

if not err and num:
    print("{} shaders compiled successfully in  ".format(num) + str(int((time.time() - i)*1000)) + " ms" )

