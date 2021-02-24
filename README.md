## Using Makefile
```make all```

To compile your .cpp files

```make clean```

To remove the generated .out files

## Using Command Line

```g++ main.cpp -o main -pthread -std=c++11 `pkg-config --cflags --libs opencv4` ```
```./main {image_name}```

Example:
```./main traffic```

image_name argument is to be given at command line without the .jpg extension.
In case argument is missing it will give output " Error loading the image"

It will make user able to select 4 pts on the image according to whichc image will be warped and finally cropped
Both the images will get saved in the same folder.
