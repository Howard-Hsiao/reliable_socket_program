server: main.o yun_function.o account.o accountManager.o threadPool.o
	g++-9 main.o yun_function.o account.o accountManager.o threadPool.o -pthread -o server -std=c++17 -lstdc++fs -l ssl -l crypto

main.o: main.cpp
	g++-9 main.cpp -c -std=c++17 -lstdc++fs

account.o: account.cpp account.h
	g++ account.cpp -c

accountManager.o: accountManager.cpp accountManager.h
	g++ accountManager.cpp -c

yun_function.o: ../yun_library/yun_function.cpp ../yun_library/yun_function.h
	g++-9 ../yun_library/yun_function.cpp -c -std=c++17 -lstdc++fs -l ssl -l crypto

threadPool.o: threadPool.cpp threadPool.h
	g++ threadPool.cpp -c

clean:
	rm *.o output *.out