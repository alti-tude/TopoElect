```bash
mkdir build && cd build
cmake ..
make all
mpirun -n 2 main
```

You have to implement a function called run. Please edit the test.cpp file.  

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