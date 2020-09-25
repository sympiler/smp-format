# SMP format

This repository is for converting different formats of QP problems.
The different formats that are supported are explained in the SMP repository in
[NASOQ docs](https://nasoq.github.io/docs/repository/). 

For how to use the API, please look at `ie2smp.cpp`, `bounded2smp.cpp`
and `smp_convertor.cpp`. 

For ease of use, ie2smp has a command-line interface. One input example is shown below:
```
./build/ie2smp -p quadratic.mtx -q linear.mtx -a EqualityMatrix.mtx -b EqualityBounds.mtx -c InequalityMatrix.mtx -d InequalityBounds.mtx -o OutputSMP.yml
```


Similarly bounded2smp also has a command line interface:
```
./build/ie2smp -p quadratic.mtx -q linear.mtx -a ConstraintMatrix.mtx -l LowerBounds.mtx -u UpperBounds.mtx -d -o OutputSMP.yml
``` 

