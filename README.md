# showlnk project - 20170125 20140129

This is a simple Windows only application to explore a Windows [MS-SHLLINK](https://msdn.microsoft.com/en-us/library/dd871305.aspx) Shell Link (.LNK) binary file format.

It is in no way `official`. All the structures are decoded after reading lots of Microsoft documentation, as simple personal exploration only... put here in the hope others may find it useful...

#### Building

This project uses [CMake](https://cmake.org/) to configure and generate build files... so is as simple as -

```
git clone the source
cd showlnk\build
cmake .. [options]
cmake --build . --config Release
```

Note, the install, if used, is to my `C:\MDOS`, which is in my **PATH** environment variable. You may want to change this in CMakeLists.txt to suit your environment.

#### Usage

Use the command `-?` to show the brief help. The simplest usage is `showlnk "file name.lnk"`, and it will show the command argument, if there is one. Increasing verbosity `-v[1-9]` will show more information... but sometimes only as a hexified dump.

Enjoy, Geoff. 20170125

; eof
