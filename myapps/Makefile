main: main.o cpl_conn.o kernel_maps.o
	g++ -o  main main.o cpl_conn.o kernel_maps.o -lcpl -lcpl-odbc -lpthread -lz

main.o: main.cpp
	g++ -c -O3 -I/usr/local/include/ -I../src/  -fopenmp -Wall -Wno-strict-aliasing  main.cpp

kernel_maps.o: kernel_maps/kernel_maps.cpp kernel_maps/kernel_maps.hpp
	g++ -c kernel_maps/kernel_maps.cpp -I../src/

cpl_conn.o: cpl/cpl_conn.cpp cpl/cpl_conn.hpp
	g++ -c cpl/cpl_conn.cpp

clean:
	rm -f main *.o