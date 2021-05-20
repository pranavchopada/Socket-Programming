# Socket-Programming
Server and Client Communication

Project Description: This is a simple program that can send/receive chat messages and files between two devices. This is executed by the client and the server. 
They communicate with each other by having the client socket reach out to the server (listener socket) to form a connection using the IP address of the server. 
The client can then send a chat message to the server or request a file from the server. The server will then reply to the message or find and send the requested
file respectively. TCP was used to send and receive data.


Chat messaging between the Client and the Server is default and does not need a keyword

Client Keyword Requests:
1. request file - to request server to send a file
2. close connection - to close the connection with the server
