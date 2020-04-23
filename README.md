```bash
mkdir build && cd build
cmake ..
make all
mpirun -n 2 <algoname> # yoyo, shout, bully
```

### IMPLEMENTATION
1. Copy the test.cpp file and rename to algo_name.cpp
2. Edit the CMakeLists.txt in the root folder
    * Add at the end (change algo_name to name of your algo)
    ```
    add_executable(algo_name src/main.cpp src/algo_name.cpp)
    target_link_libraries(algo_name ${MPI_CXX_LIBRARIES} topo)
    ```
3. From now on, to run your code, the final executable will be inside the build folder and it will be called ```algo_name```

### TOPO NAMESPACE
1. Available graph functions (call at the beginning of the run function according to preference )
```
void topo::make_ring(): makes a ring of size numprocs
```

2. Other important variables
```c++
topo::numprocs  //number of processes
topo::rank      //rank of current process
topo::globals   // vector of vectors of ints containing the broadcasted global variables
```
3. Tag for send/recv operations should be greater than ```topo::TAGS_CUSTOM_BASE```
4. **Utility functions**
```c++
int topo::make_global(vector<long long int> data_buffer, bool is_root);
// broadcasts the data buffer variable following the topological graph. 
// returns the index in the globals vector, where the current buffer has been stored. This returned value is same in all the processes.
```
## THE AUTHOR COULD NOT BE BOTHERED TO COMPLETE THE DOCUMENTATION. HELP YOURSELF TO ```topologies.h```

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
