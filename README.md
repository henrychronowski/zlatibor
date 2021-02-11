# animal3D-SDK-202101SP
animal3D SDK and related course frameworks for spring 2021.

# Project 1: Server/Client Chat Application
In order to use this project, clone the repository and check out the networking_proj1 branch. Navigate into the GPRO Net SDK/ directory and run both the remote and server bat files as an administrator. This will open both projects. In order for the server to run you must have another accessible instance with remote debugging set up and have the address of that instance in the first line of the configuration file in GPRO Net SDK/resource/. If this is set up, launch the server without debugging, then launch as many clients as you want without debugging. 

The clients use the service by first typing in their username, then typing and pressing enter to send a message to the public chat. If they wish to request a list of users from the server, this can be done by entering the command !users. If the client wishes to send a direct message to a specific user, they must type the @ character, press enter, and respond to the prompts by entering the user's username then the message they wish to send. The text output is not buffered so the client must send a message/press enter in order for their local chat display to update.

The server simply prints traffic to the console and stores a log by default in c:/Users/Public/. In order to shut down the server send an interrupt signal (ctrl+c) in the console.
