# PiEye
PiEye is a C++11 library providing fast and easy access to the Raspberry Pi camera using OpenCV. The library uses the camera's MMAL interface to communicate.

Credits go to [RaspiCam] and the Raspberry Pi [raspivid and raspistill] programs, serving as an inspiration for this library.

This library is released under the MIT license.

## Compilation and installation
CMake is required to build and install the library. To compile the library:

```sh
$ mkdir build
$ cd build
$ cmake ../
$ make
```

To install/uninstall the library, headers and CMake package, simply run these commands from the build directory:

```sh
$ sudo make install
$ sudo make uninstall
```

## Usage
After instalation, the library can be added as a target in your CMake project as follows:

```cmake
project(MyProject)
...
find_package(PiEye REQUIRED)
target_link_libraries (<MyTarget> PiEye)
...
```

## Example - capturing a still image

```c++
#include <opencv2/opencv.hpp>
#include <PiEye.h>

int main(int argc, char* argv[]) {
    PiEye camera;
    cv::Mat image;
    
    cam.grabStill(image);
    cv::imwrite("still.jpg", image);
    
    return 0;
}
```

Check the included PiEyeTest program for a more detailed example.

[RaspiCam]: <https://github.com/cedricve/raspicam>
[raspivid and raspistill]: <https://github.com/raspberrypi/userland/tree/master/host_applications/linux/apps/raspicam>
