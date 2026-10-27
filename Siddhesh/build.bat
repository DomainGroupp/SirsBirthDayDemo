cl /c /EHsc OGL.c
rc OGL.rc
link OGL.obj OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS