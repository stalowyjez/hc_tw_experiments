CC=g++
FLAGS=-std=c++11 -O3 -Wall 

COMMON=main.cpp hc_solver.cpp hc_solver_common.cpp treewidth.cpp strange_io.cpp basic.cpp gaussian.cpp
COMMON_GD=main_gd.cpp hc_solver.cpp hc_solver_common.cpp treewidth.cpp strange_io.cpp basic.cpp gaussian.cpp
COMMON_D=main.cpp hc_solver.cpp hc_solver_common_d.cpp treewidth.cpp strange_io.cpp basic.cpp gaussian.cpp edge_selection.cpp
COMMON_D_GD=main_gd.cpp hc_solver.cpp hc_solver_common_d.cpp treewidth.cpp strange_io.cpp basic.cpp gaussian.cpp edge_selection.cpp
COMMON_CC=main.cpp hc_solver.cpp hc_solver_common_cc.cpp treewidth.cpp strange_io.cpp basic.cpp edge_selection.cpp poly.cpp
COMMON_CC_GD=main_gd.cpp hc_solver.cpp hc_solver_common_cc.cpp treewidth.cpp strange_io.cpp basic.cpp edge_selection.cpp poly.cpp

OBJECTS=hc_simple hc_gaussian_1 hc_gaussian_2 hc_simple_d hc_gaussian_1_d hc_gaussian_2_d hc_simple_cc hc_fj_cc \
	hc_simple_d_gd hc_gaussian_1_d_gd hc_gaussian_2_d_gd hc_cc_gd

all: $(OBJECTS)

clean:
	rm $(OBJECTS)

#
# Main targets
#

hc_simple: $(COMMON)
	$(CC) $(FLAGS) -o hc_simple -DUSE_MEMORY -DREDUCTION=0 $(COMMON)

hc_gaussian_1: $(COMMON)
	$(CC) $(FLAGS) -o hc_gaussian_1 -DUSE_MEMORY -DREDUCTION=1 $(COMMON)

hc_gaussian_2: $(COMMON)
	$(CC) $(FLAGS) -o hc_gaussian_2 -DUSE_MEMORY -DREDUCTION=2 $(COMMON)

hc_simple_d: $(COMMON_D)
	$(CC) $(FLAGS) -o hc_simple_d -DUSE_DECISIVE -DREDUCTION=0 -DFREQ=$(FREQ) $(COMMON_D)

hc_gaussian_1_d: $(COMMON_D)
	$(CC) $(FLAGS) -o hc_gaussian_1_d -DUSE_DECISIVE -DREDUCTION=1 -DFREQ=$(FREQ) $(COMMON_D)

hc_gaussian_2_d: $(COMMON_D)
	$(CC) $(FLAGS) -o hc_gaussian_2_d -DUSE_DECISIVE -DREDUCTION=2 -DFREQ=$(FREQ) $(COMMON_D)

hc_simple_cc: $(COMMON_CC)
	$(CC) $(FLAGS) -o hc_simple_cc -DREDUCTION=0 $(COMMON_CC)

hc_fj_cc: $(COMMON_CC)
	$(CC) $(FLAGS) -o hc_fj_cc -DREDUCTION=0 -DUSE_CC_FAST_JOIN $(COMMON_CC)

#
# Main targets - outer decomposition
#

hc_simple_d_gd: $(COMMON_D_GD)
	$(CC) $(FLAGS) -o hc_simple_d_gd -DUSE_DECISIVE -DREDUCTION=0 -DFREQ=$(FREQ) $(COMMON_D_GD)

hc_gaussian_1_d_gd: $(COMMON_D_GD)
	$(CC) $(FLAGS) -o hc_gaussian_1_d_gd -DUSE_DECISIVE -DREDUCTION=1 -DFREQ=$(FREQ) $(COMMON_D_GD)

hc_gaussian_2_d_gd: $(COMMON_D_GD)
	$(CC) $(FLAGS) -o hc_gaussian_2_d_gd -DUSE_DECISIVE -DREDUCTION=2 -DFREQ=$(FREQ) $(COMMON_D_GD)

hc_cc_gd: $(COMMON_CC_GD)
	$(CC) $(FLAGS) -o hc_cc_gd $(COMMON_CC_GD)

