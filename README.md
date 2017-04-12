This expects to run from the Samples folder within NITE-Bin-Dev-MacOSX-v1.5.2.21

Setup instructions for Kinect on El Capitan:
https://creativevreality.wordpress.com/2016/01/26/setting-up-the-kinect-on-osx-el-capitan/


libboost_system.dylib had to be built for both i386 and x86_64 architectures and those merged into a universal dylib.

Build it this way:
```
rm -rf i386 x86_64 universal
./bootstrap.sh --with-toolset=clang --with-libraries=filesystem
./b2 toolset=darwin -j8 address-model=32 architecture=x86 -a
mkdir -p i386 && cp stage/lib/*.dylib i386
./b2 toolset=clang -j8 cxxflags="-arch i386 -arch x86_64" -a
mkdir x86_64 && cp stage/lib/*.dylib x86_64
mkdir universal
for dylib in i386/*; do 
  lipo -create -arch i386 $dylib -arch x86_64 x86_64/$(basename $dylib) -output universal/$(basename $dylib); 
done
```


The NiteSameplMakefile must use this line: 

CFLAGS += -std=c++14
