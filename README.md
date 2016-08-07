# Multi-client-fileserver

Client Machine 
	CPU 	- 	i7 5500U
	RAM 	-    8 GB
Server Machine 
	CPU 	-	i5-3210M
	RAM	-	4 GB
	
Setup
The client and server are connected through a router via ethernet cables.

Running Instructions:

For Server:
	1) compile using gcc -w -pthread server-mp.c -o server
	2) run using ./server <portno>

For Client:
	1) compile using gcc -w -pthread multi-client.c -o client
	2) run using ./client <host> <portno> <threads> <runtime> <sleeptime> <filemode>