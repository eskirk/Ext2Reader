#Ext2Reader#

##Description##

Authors: Kevin Navero

Reader for an ext2 filesystem

##How To Use##
```
$ make
$ ./program4 <image.ext2>
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

##TODO##

1. Alphabetical ordering of listing
