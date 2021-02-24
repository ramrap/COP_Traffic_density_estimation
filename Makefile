all:
	g++ main.cpp -o main -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
clean:
	rm *.out

# For testing on one device	
temp:
	sudo g++ main.cpp -o main -pthread -std=c++11 `pkg-config --cflags --libs opencv4`

#change empty to image name
build:
	./main empty
