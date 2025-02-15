ReadMe for CMake version of lab 1
To start program:
- Print "cmake -S . -B build" in your console
- Then print "cd build"
- "make"
- "./lab1"
- If you want to switch from double version to float, 
write flag "-D IS_FLOAT=true" when printing cmake
(cmake -S . -B build -D IS_FLOAT=True)

Results:
Double: -6.77e-10
Float: -0.213894
