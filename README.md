# wlx_csv
Total Commander CSV Lister Plugin Sample

### Premake

[Premake](https://github.com/premake/premake-core) is a build configuration tool.

usage:
```
premake5 vs2010
cd build
msbuild build\wlx_csv.sln /t:rebuild /p:configuration=release
```
or
```
premake5 gmake
cd build
make
```

### GNU Makefile
I also write a simple Makefile for gcc.

### COMMANDER_PATH
COMMANDER_PATH is a environment value for output dir.
if you launch cmd.exe or MSVC from Total Commander, COMMANDER_PATH value add by Total Commander.
otherwise, you must define COMMANDER_PATH in Makefile or Premake file.

