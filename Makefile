
# Add the following 'help' target to your Makefile
# And add help text after each target name starting with '\#\#'
 
help:           ## Show this help.
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'
 
build:    ## TO BUILD COMMAND
	g++ main.cpp -o main.out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`
clean:   ## Clean everything
	rm *.out

emp: ##change empty to image name
	./main.out empty
