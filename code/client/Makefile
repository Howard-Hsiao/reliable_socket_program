client: client.o yun_function.o cServerHandler.o cli_threadPool.o
	g++ client.o yun_function.o cServerHandler.o cli_threadPool.o -o client -l ssl -l crypto -pthread

client.o: client.cpp
	g++-9 client.cpp -c -std=c++17 -lstdc++fs -pthread

yun_function.o: ../yun_library/yun_function.cpp ../yun_library/yun_function.h
	g++-9 ../yun_library/yun_function.cpp -c -std=c++17 -lstdc++fs -l ssl -l crypto
	
cServerHandler.o: cServerHandler.cpp cServerHandler.h 
	g++-9 cServerHandler.cpp -c -std=c++17 -lstdc++fs -pthread

cli_threadPool.o: cli_threadPool.cpp cli_threadPool.h
	g++-9 cli_threadPool.cpp -c -pthread

clean:
	rm *.o output *.out