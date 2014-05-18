#Ext2Reader#

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


