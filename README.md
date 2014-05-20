#Ext2Reader#

##Description##

Authors: Kevin Navero

Reader for an ext2 filesystem

##How To Use##
```
$ cd Debug
$ make
$ ./ext2reader ../resources/testimage.ext2
```

```
Usage: 
   ext2reader <image.ext2> [path]
   ext2reader -l <image.ext2> <file_to_dump.txt>
   If [path] is not specified, '/' will be used

Options:
   -l    print to the screen the contents of <file_to_dump.txt>

Notes:
   All paths not prefixed with '/' are relative to the root directory
```

##Notes##

1. Running into a weird bug where in find_dir(), a name is matched to a 
directory entry, but the mode for that entry is 0x0000. However, when the
filesystem is mounted, that entry is seen as a directory. The logic 
in find_dir() doesn't seem to be completely off, since it is able to
match the name and the correct mode in "most" cases.


