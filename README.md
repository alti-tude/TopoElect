```bash
mkdir build && cd build
cmake ..
make all
mpirun -n 2 main
```

### EXPECTED OUTPUT OF THE TEST PROGRAM FOR 10 PROCS
```bash
mpirun -n 10 main
```

```
1 0 2 1 
1 1 3 1 
1 2 4 1 
1 3 5 1 
1 4 6 1 
1 5 7 1 
1 6 8 1 
1 7 9 1 
1 8 10 1 
1 9 11 1
```

### IMPLEMENTATION
1. Copy the test.cpp file and rename to algo_name.cpp
2. Edit the CMakeLists.txt in the root folder
    * Instead of ```add_executable(main src/main.cpp src/test.cpp)``` change it to ```add_executable(main src/main.cpp src/algo_name.cpp)```

### TOPO NAMESPACE
1. Available graph functions (call at the beginning of the run function according to preference )
```
void topo::make_ring(): makes a ring of size numprocs
```

2. Constructed graphs will be available in ```topo::adjacency_list``` (vector of vectors of ints). **DO NOT USE THIS** in your code. Use only for debugging.
3. Neighbours will be available in ```topo::neighbours``` (vector of ints).
4. Other important variables
```c++
topo::numprocs  //number of processes
topo::rank      //rank of current process
topo::globals   // vector of vectors of ints containing the broadcasted global variables
```
5. Tag for send/recv operations should be greater than ```topo::TAGS_CUSTOM_BASE```
6. Utility functions
```c++
int topo::make_global(vector<long long int> data_buffer, bool is_root);
// broadcasts the data buffer variable following the topological graph. 
// returns the index in the globals vector, where the current buffer has been stored. This returned value is same in all the processes.
```

### DEBUGGING
1. In the CMakeLists.txt in the root folder, uncomment the last line, ```add_definitions(-DDEBUG)```.
2. Then do the following again
```
mkdir build && cd build
cmake ..
make all
mpirun -n 2 main
```
3. This time the code will get stuck and show the output line ```wating for debugger pid=x```.
4. Open gdb
```
gdb
```
5. Run the following commands inside the gdb shell
```bash
attach x #where x is the pid we got earlier
set variable i=1
finish

# set any break points you want
# basically use the debugger normally now. 
next # execute the next statement
print x #display value of x
break func #set breakpoint at function func
```
