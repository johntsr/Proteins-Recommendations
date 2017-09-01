Instructions for JNI

From Java perspective:
  1. Declare the methods as "native" in the Java code.
  2. Load the dynamic library with the "System.loadLibrary('XXX')" function. In UNIX systems, a file named "libXXX.so" is expected.
  3. The native methods can be called through the object returned by the above call.
  4. Run the java code as: java -Djava.library.path=/path/to/libXXX.so
  
From C++ perspective:
  1. Run the Java file as : "javah XXX" (without the .java extension) to create the apropriate C/C++ header file.
  2. Implement the methods in the above header file in C++.
  3. Compile the C++ code with the apropriate flags and name it as expected (libXXX.so).
  
Files of interest:
  1. Example.java (the Java code)
  2. Example.h (the auto-generated header file)
  3. Main.cpp (the implementation of the native methods)
  4. Makefile (the apropriate g++ flags)
  
  See the example!
