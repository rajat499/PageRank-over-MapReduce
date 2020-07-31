# PageRank using MapReduce

In this project, we have implemented two concepts, both coming from Google: the [PageRank](http://www.ams.org/publicoutreach/feature-column/fcarc-pagerank) algorithm and the [MapReduce](https://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf) paradigm for problem solving. We have combined these two concepts and have solved PageRank as a MapReduce problem.

* **mapreduce_cpp:** Used existing [MapReduce C++ library](https://github.com/cdmh/mapreduce).
* **mapreduce_own:** Implemented our own MapReduce library with MPI
* **mapreduce_mpi:** Used existing [MapReduce MPI library](https://mapreduce.sandia.gov/).
* **serial:** Solved PageRank using serialized code without any distributed computing 
