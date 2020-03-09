rm callgrind.out.*
valgrind --tool=callgrind ./test
kcachegrind
