cls
cl.exe /c /EHsc OGL.c 

rc.exe OGL.rc

link.exe OGL.obj OGL.res USER32.LIB GDI32.LIB /SUBSYSTEM:WINDOWS
