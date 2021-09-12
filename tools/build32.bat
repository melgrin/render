@echo off
set RENDERDEV_WIN32_ARCH_TARGET=x86
set RENDERDEV_WIN32_ARCH_LINK_OPTS=-subsystem:windows,5.1
call build_common.bat %*
