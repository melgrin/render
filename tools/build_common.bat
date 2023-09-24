@echo off
if not "%VSCMD_ARG_TGT_ARCH%" == "%RENDERDEV_WIN32_ARCH_TARGET%" (
    call vcvarsall %RENDERDEV_WIN32_ARCH_TARGET%
)

if not exist ..\build mkdir ..\build
pushd ..\build

@echo on
cl -nologo -MT -Gm- -GR- -EHa- -Od -Oi -W4 -WX -wd4201 -wd4189 -Z7 -Fm:win32 -DRENDERDEV_ASSERT -DRENDERDEV_DEBUG %* ..\source\win32.cpp /link -opt:ref %RENDERDEV_WIN32_ARCH_LINK_OPTS% user32.lib gdi32.lib winmm.lib
@echo off

popd

rem warning C4189: local variable is initialized but not referenced
