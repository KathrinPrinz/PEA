# PEA: The Parallel Epsilon Algorithm for Tri-Objective Integer Optimization Problems

The software and data in this repository is used in the research reported on in the paper [[1]](#1).

## Cite

To cite the contents of this repository, please cite the paper [[1]](#1).

## Description

This repository contains folders for the source files (.cpp and .h), assignment problem test instances (n=55-100) generated according the scheme described in [[3]](#3) and the results of the computational study described in the paper [[1]](#1).

## Usage
The executable itself is called `PEA` and requires the instance file, the results directory and the number of threads. The instance file needs to be in the LP file format.

### Example 

Running `./PEA instances/AP_p-3_n-55_ins-2.lp results/PEA_tutulla/AP_p-3-n-55_ins-2.json 16` will solve the problem specified in the file `instances/AP_p-3_n-55_ins-2.lp` and use `16` threads. The output will be written to `results/PEA_tutulla/AP_p-3-n-55_ins-2.json`.


## Results
We compared the sequential algorithms QSM ([[2]](#2)), DPA ([[4]](#4)) and TamVan ([[5]](#5)) on knapsack instances and assignment problems provided by Kirlik and Sayin ([[3]](#3)).
Then, we compared the CPLEX parallelization of QSM ([[2]](#2)), parallel AIRA ([[6]](#6)) and PEA on knapsack instances and assignment problems provided by Kirlik and Sayin ([[3]](#3)). The experiments were performed on a machine with two 2.60 gigahertz Intel Xeon E5-2670 processors with 16 physical cores on two sockets and a RAM size of 188 GiB, CPLEX 22.1.1., OpenMP 4.5, using 1,2,4,6,8,16  threads and a runtime limit of 5000 seconds. The results can be found in the folders AIRA_AP, AIRA_KP, DPA_AP, DPA_KP, PEA_AP, PEA_KP, QSM_AP, QSM_KP, TamVan_AP and  TamVan_KP.

In the second round of experiments we further investigated the scaling of PEA with the number of threads. The experiments were performed on another machine with two 3.10 gigahertz AMDY EPYC 9554 64-Core processors, 128 cores on two sockets, a RAM size of 1511 GiB, CPLEX 22.1.1., OpenMP 4.5, using 16,32,64,128, threads and a runtime limit of 5000 seconds. We used the assignment problem instances provided by Kirlik and Sayin ([[3]](#3)) of size n=50 and newly generated instances according to their scheme of size n=55-100. The results can be found in the folder PEA_tutulla.


## References

<a id="1">[1]</a>
Prinz, K., Ruzika, S. (2024)
The Parallel Epsilon Algorithm for tri-objective integer optimization problems.

<a id="2">[2]</a>
Boland, N., Charkhgard, H., Savelsbergh, M. (2017)
The Quadrant Shrinking Method: A simple and efficient algorithm for solving tri-objective integer programs.
European Journal of Operational Research 260(3):873–885

<a id="3">[3]</a>
Kirlik, G., Sayın, S. (2014)
A new algorithm for generating all nondominated solutions of multiobjective discrete optimization problems.
European Journal of Operational Research 232(3):479–488

<a id="4">[4]</a>
Dächert, K., Fleuren, T., Klamroth, K. (2024)
A simple, efficient and versatile objective space algorithm for multiobjective integer programming.
Math. Methods Oper. Res. 

<a id="5">[5]</a>
Tamby, S., Vanderpooten, D. (2021)
Enumeration of the nondominated set of multiobjective discrete optimization problems.
INFORMS J. Comput. 33(1):72–85

<a id="6">[6]</a>
Petterson, W., Ozlen M. (2020)
Multiobjective integer programming: Synergistic parallel approaches
INFORMS J. Comput. 32(2):461–472
