### Program Instructions

#### HOW TO COMPILE

* "make"

The server program is written in C and can be compiled using the included makefile.
Type "make" and press enter. This generates the needed executable.

The client program is written in Python, and does not need to be compiled.


#### HOW TO EXECUTE

* "./ftserver <port>" e.g. "./ftserver 7777"

The server must be started first. To start the compiled program, type 
"./ftserver <port>". This starts the program on the current host at the desired
port number. I made this program to only use ports 1024-65535, since 0-1024 are 
often reserved for other processes. 

* "python ftclient.py hostname <ctrl port> <[-l|-g <filename>] <data port>" 
e.g. "python ftclient.py localhost 8888 -g long.txt 8889"

Once the server is running you can run the client program in a new terminal. If the 
programs are both running on the same host, use "localhost" for the host name. The 
control port needs to be the same port that the server program was started on, 
otherwise a connection can't be established. 


#### HOW TO CONTROL

The command flag can either be "-l" to list the items in the server's current 
directory, or "-g" plus an additional argument of a filename to request a file 
transfer. The last argument is the user specification for a data port, which 
has the same constraints as the control port, and must be unique from it. 
NOTE: The server will continue running after serving requests through the data
port, but due to OS limitations the data port cannot be immediately resused. 
Access the server with the same control port but make sure to update the data
port with subsequent requests over a short period. 
