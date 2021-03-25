
# Add the following 'help' target to your Makefile
# And add help text after each target name starting with '\#\#'
 
help:    ## Show this help.
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'
 
build:    ## TO BUILD COMMAND
	g++ main.cpp -o main.out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
clean:   ## Clean everything
	rm *.out
m1:	## build method 1
	g++ m1.cpp -o m1.out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
m2:	## build method 2
	g++ m2.cpp -o m2.out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
emp: ##change empty to image name
	./main.out empty
