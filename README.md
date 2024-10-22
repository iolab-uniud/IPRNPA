<!---
I need to write a README.md that explains how to compile
-->
# IPRNPA Solver
## Overview

This repository contains the local search-based software developed to solve the **Integrated Patient-to-Room and Nurse-to-Patient Assignment (IPRNPA)** problem, as proposed by Brandt et al. in their paper ["Integrated patient-to-room and
nurse-to-patient assignment in hospital wards"](https://arxiv.org/abs/2309.10739).

It also includes the best solutions found by our **Multi-Neighborhood Simulated Annealing** solver for the instances provided in the dataset available at [IPRNPA Instance Generator](https://github.com/TLKT0M/IPRNPA_instance_generator).

## Prerequistes

- C++ (version >= 14)
- Boost (see download options [here](https://www.boost.org/))
- EasyLocal++ 3 (available [here](https://bitbucket.org/satt/easylocal-3))
- json.hpp (available [here](https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp)) should be included in the path.

## Solver

Once the EASYLOCAL/BOOST paths in the ```Makefile``` have been updated to point to the installation directories of the respective libraries, you can compile the code by running the following command ```make -j``` in the ```Software``` folder.

To run the Multi-Neighborhood Simulated Annealing solver presented in the paper, use the following command: 

    ./pna --main::method SA
    --main::instance <instance_path> 
    --IPRNPA_SA::max_evaluations <num_iterations> 
    --IPRNPA_SA::start_temperature <float_num> 
    --IPRNPA_SA::min_temperature <float_num>  
    --IPRNPA_SA::cooling_rate <float_num (0,1)> 
    --main::change_room_rate <float_num [0,1]>  
    --main::swap_nurses_rate <float_num [0,1]> 
    --main::swap_rooms_rate <float_num [0,1]>

<!--the list of QSA parameters can be found with 

    ./pna --main::inst <instance_path> --main::method QSA --help
--->

## Solutions
Solutions are presented as ```.json``` files

* Folder ```tables``` contains the best solutions found by the Simulated Annealing (SA) solver, as reported in Tables 8-12 of the Appendix in the paper.

* Additionally, folder ```long_runs``` contains solutions that improve upon the best-reported solutions by Brandt et al.









