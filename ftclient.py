"""
 * File Transfer Application - Client
 *
 * Programmer: Bobby Hines
 *
 * Description:
 * This is the "client" side of the file transfer program. It will
 * only run for the duration of the specified action.
 *
 * Input: The user must specify a host, host's control port, -l or 
 * -g depending on if this is a list or get command, a file name if
 * it is a get command, and if desired, a specific data port, other-
 * wise a default will be assigned. 
 *
 * Output: Various validation messages, the server's responses, and 
 * if the specified file is found on the server, a copy of that 
 * file in the same directory as this program. 
 *
"""

#!/usr/bin/python

import sys, socket, getpass, string, os, time

def errMsg(errType):
    if (errType == "BAD_CL_ARGS"):
        msg = "Usage: python ftclient.py hostname <ctrl port> <-l|-g> [filename] <data port>"
    elif (errType == "BAD_PORTNUM"):
        msg = "Error: Control port must be a number between 1024 and 65535."
    elif (errType == "BAD_CFLAG"):
        msg = "Error: Command flag not recognized."
    elif (errType == "BAD_DATAPORT"):
        msg = "Error: Invalid data port. Use 1024-65535, cannot match control port."
    elif (errType == "BAD_SERVER"):
        msg = "Error: Unable to connect with specified host:port."
    elif (errType == "DISCONNECTED"):
        msg = "Error: Disconnected from server."
    elif (errType == "BAD_CREDS"):
        msg = "\nAccess denied."
    elif (errType == "BAD_FILE"):
        msg = "\nThe file you requested could not be found."
    elif (errType == "DUPLICATE"):
        msg = "\nThe file you requested already exists in the current directory."
    print msg
    sys.exit()

if __name__ == "__main__":

    # Validate command-line args:
    if (len(sys.argv) < 5):
        errMsg("BAD_CL_ARGS")

    # Save the host name as a string
    host = sys.argv[1]

    # Save the port if it can be parsed to an integer
    try:
        port = int(sys.argv[2])
    except ValueError:
        errMsg("BAD_PORTNUM")

    # Check for correct port range
    if (port < 1024 or port > 65535):
        errMsg("BAD_PORTNUM")

    # Validate command flag
    cflag = sys.argv[3]
    if (cflag != "-l" and cflag != "-g"):
        errMsg("BAD_CFLAG")

    dataIndex = 4
    # If flag was -g, get the filename
    if (cflag == "-g"):
        filename = sys.argv[4]

        if (len(sys.argv) != 6):
            errMsg("BAD_CL_ARGS")
        else:
            dataIndex = dataIndex + 1

    # Validate data port
    try:
        dataPort = int(sys.argv[dataIndex])
    except ValueError:
        errMsg("BAD_DATAPORT")
    
    if (dataPort < 1024 or dataPort > 65535 or dataPort == port):
        errMsg("BAD_DATAPORT")

    # Open a TCP socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)

    # Establish TCP control connection with server at specified port
    try:
        s.connect((host, port))
    except:
        errMsg("BAD_SERVER")

    print "Establishing connection with " + host + ":" + str(port) + "..."

    # Get credentials
    username = raw_input("Username: ")
    # Add length header for C parsing
    username = str(10000 + len(username)) + username
    username = username[1:]
    username = username.rstrip()
    s.send(username)
    password = getpass.getpass()
    # Add length header for C parsing
    password = str(10000 + len(password)) + password
    password = password[1:]
    password = password.rstrip()
    s.send(password)
    
    response = s.recv(4096)
    if not response:
        errMsg("DISCONNECTED")

    # Check for permission
    if response == "ACCESS DENIED":
        errMsg("BAD_CREDS")
    else:
        print response

    if cflag == "-l":
        s.send(cflag)
        print "\nCurrent directory contents:"

        response = s.recv(4096)
        if not response:
            errMsg("DISCONNECTED")

        print response

    elif cflag == "-g":

        s.send(cflag)
        print "\nRequesting file..."
        filename = str(1000 + len(filename)) + filename
        filename = filename[1:]
        filename = filename.rstrip()
        s.send(filename)

        # Now change the name back to normal
        filename = filename[3:]

        response = s.recv(4096)
        if not response:
            errMsg("DISCONNECTED")

        if response == "NOT FOUND":
            errMsg("BAD_FILE")

        s.send(str(dataPort))

        # Connect over the data port
        s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s2.settimeout(2)

        time.sleep(1)

        try:
            s2.connect((host, dataPort))
        except:
            errMsg("BAD_SERVER")

        print "Establishing data connection over port " + str(dataPort) + "..."

        # Handle duplicates
        files = [f for f in os.listdir('.') if os.path.isfile(f)]
        for f in files:
            if f == filename:
                errMsg("DUPLICATE")

        # Create the empty file
        newFile = open(filename, 'w')

        # read chunks from socket and append to newFile
        response = s2.recv(4096)

        while(response):
            print "Receiving..."
            newFile.write(response)
            response = s2.recv(4096)

        print "Done Receiving."
        newFile.close()
        s2.shutdown(socket.SHUT_WR)

        sys.exit()
