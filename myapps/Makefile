main: main.o vertex_relabel.o kernel_maps.o cpl_conn.o profile.o helper.o
	g++ -o  main main.o vertex_relabel.o kernel_maps.o cpl_conn.o profile.o helper.o -lcpl -lcpl-odbc -lpthread -fopenmp -lz

main.o: main.cpp
	g++ -c -O3 -I/usr/local/include/ -I../src/ -Wall -Wno-strict-aliasing  main.cpp

vertex_relabel.o: vertex_relabeling/vertex_relabel.cpp vertex_relabeling/vertex_relabel.hpp
	g++ -c vertex_relabeling/vertex_relabel.cpp -I../src/

kernel_maps.o: kernel_maps/kernel_maps.cpp kernel_maps/kernel_maps.hpp
	g++ -c kernel_maps/kernel_maps.cpp -I../src/

cpl_conn.o: cpl/cpl_conn.cpp cpl/cpl_conn.hpp
	g++ -c cpl/cpl_conn.cpp

profile.o: profile/profile.cpp profile/profile.hpp
	g++ -c profile/profile.cpp

helper.o: profile/helper.cpp profile/helper.hpp
	g++ -c profile/helper.cpp

clean:
	rm -f main *.o