Fromea Dragos-Florin
322 CC

In this project, I implemented a web client that communicates with a REST API. I created functions to manage a virtual library, including registering and logging into the server, getting information about books, adding and deleting books.

The project consists on the following files:
    -client.c: The implementation of the client for the server, where the application flow is managed by functions such as registering, logging in, getting books, and more.
    -requests.c/requests.h: Functions that handles requests such as GET, POST and DELETE.
    -parson.c/parson.h: Library used for parsing data by creating JSON Objects, facilitating data parsing for requests, optimal for a C implementation.
    -helpers.c/helpers.h: Functions that facilitate the communication with the server such as opening and closing connections, sending and receiving messages to/from the server, etc.
    -buffer.c/buffer.h: Functions that simplify working with buffers.
    -Makefile
    -README.md: Explanation of the application.

In the client.c file, I managed the application flow by creating the following functions:
    -register_user: Reads the username and password from stdin, checks the validity, eliminates '\n' caracters from input. It sends the corresponding message to the server and check if the response validates the registration.
    -login: Similar to the registration_user function, it receives, checks validity, and shapes data from stdin to remove unwanted characters, then sends a POST request to the server. It checks if the response is a succesful one and extract the cookie from it.
    -enter_library: Generates the GET request, verifies the user's access to the library, and extract the JWT token for subsequent function usage.
    -get_books: verifies the user's acces, which is validated by the response generated from the GET request containing the token, and displays all the available books from the library in a JSON Array format.
    -get_book: Reads the ID of the book the user wants to see information for, checks validity, then computes a GET request with the JWT token. If the book with the specific ID exists, it displays detailed information about the book, otherwise, it displays an error message saying that no book was found.
    -add_book: To add a book in the library it reads the information about the book, checks if the information is valid, and create a POST request with those details in a JSON object format. Checking the user's acces we also include the token in the request. The message received from the server confirms whether the book was added successfully.
    -delete_book: Reads the ID of the book to be deleted, verifies its validity, generates a DELETE request, and sends it to the server. Verifies in the response from the server whether the book was deleted successfully.
    -logout: Sends a GET message, including the cookie, to verify if the user was logged in. It verifies the success of the command, and then frees the cookie session and JWT token to revoke the user's access to the server functions.
    -auxiliary functions such as checkValidity, hasSpaces, checkNum were built to simplify the process of verifying the input, while my_atoi function is responsible for converting a string to an integer.

For parsing objects I used the Parson library, which assisted me with its functionalities for creating JSON objects and converting them back to JSON-formatted strings, being optimal for a C implementation.

To create this project I worked on HTTP Protocol laboratory(laboratory 9: https://gitlab.cs.pub.ro/pcom/pcom-laboratoare-public/-/tree/master/lab9), and I used Parson library(https://github.com/kgabis/parson).