all:
	@echo "Write this Makefile by your self."

sim:main.cpp top.v
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	verilator --cc --exe --build -j 0 -Wall --trace-fst mani.cpp top.v 
include ../Makefile
