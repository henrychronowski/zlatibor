# animal3D-SDK-202101SP
animal3D SDK and related course frameworks for spring 2021.

# GPR 430: Final Project
In order to use this project, clone the repository and check out the final_project branch. Navigate into the GPRO Net SDK/ directory and run the GPRO-NET-VSLAUNCH-REMOTE batch file to launch the server solution and the GPRO-NET-VSLAUNCH batch file to launch the client solution. Before running either solution check the IP address for the server machine and make sure that it is listed as SERVER_IP on line 37 of gpro-net-RakNet-Client.cpp in the gpro-net-Client project. Also ensure that gpro-net-Client-Windowed is set as the startup project for the client and gpro-net-Server-Windowed is set as the startup project for the server.

Once this is done, the server can be built and run, then up to 10 (a limit configurable in the client project) clients may be started and connected to the server.
