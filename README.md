# Finding Hamiltonian Cycle in Graphs of Bounded Treewidth: Experimental Evaluation

The following repository is attachment to our article with the same title. It contains some helpful to reproduce our experiments described in that paper, in particular our implementations, description of the data sets and tree decompositions for instances with greater treewidth.

The instances on which experiments were performed are part of the set used in FHCP Challenge (http://www.flinders.edu.au/science_engineering/csem/research/programs/flinders-hamiltonian-cycle-project/fhcpcs.cfm).

For computing tree decompositions we used heuristic written by Ben Strasser for PACE Challenge 2017. It can be found here: https://github.com/kit-algo/flow-cutter-pace17

## Naming

For naive and rank based approaches their names are following:

+ hc_simple - naive algorithm
+ hc_gaussian_1 - slower rank-based approach
+ hc_gaussian_2 - faster rank-based approach

Additionally "\_d" can be added for the decisive version (without it algorithm keeps the solutions during the computation) and "\_gd" can be added for version accepting outer decomposition. If both are used "\_d" goes first.

For the cut-and-count approach names are following: 

+ hc\_simple\_cc - theoretically slower version
+ hc\_fj\_cc - theoretically faster version, but with large overhead (useless in practice)
+ hc\_cc\_gd - first version, but accepting outer decomposition

## Compiling and running

For compiling a certain version the following command should be used
```
make solver_name
```
where solver\_name is the name of preferred version, as listed above.

For using the built-in fill-in heuristic for finding tree decomposition one should run:
```
./solver_name < instance
```
and for using an outer decomposition (only for "\_gd" versions):
```
./solver_name instance < tree_decomposition
```
where instance is a HCP file (like in FHCP Challenge set) and tree\_decomposition is a TD file (the description can be found here: https://pacechallenge.wordpress.com/pace-2017/track-a-treewidth/).
