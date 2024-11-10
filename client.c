#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define HOST "34.246.184.49"
#define PORT 8080
#define MAXCOMMANDLEN 100

// function that checks if the input exists
int checkValidity(char *input) {
    if ((strlen(input) == 1 && input[0] == '\n') || strlen(input) == 0)
    {
        return 0;
    }
    return 1;
}

// function that checks if the credentials have spaces
int hasSpaces(char *input) {
    for (int i = 0; i < strlen(input); i++)
    {
        if (input[i] == ' ')
        {
            return 0;
        }
    }
    return 1;
}


// function that converts a string to an integer
int my_atoi(char *str) {
    int result = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        result = result * 10 + str[i] - '0';
    }
    return result;
}

// function that checks if a string is a number
int checkNum(char *str) {
    for (int i = 0; i < strlen(str); i++)
    {
        if(str[i] >= '0' && str[i] <= '9')
        {
            continue;
        } else {
            return 0;
        }
    }
    return 1;
}

// function called when the user wants to create a new account
void register_user(int socketfd) {

    // getting the credentials from the user
    char *username = calloc(100, sizeof(char));
    printf("username=");
    fgets(username, 100, stdin);

    char *password = calloc(100, sizeof(char));
    printf("password=");
    fgets(password, 100, stdin);

    // checking if the credentials are valid
    if (checkValidity(username) == 0 || checkValidity(password) == 0 ||
        hasSpaces(username) == 0 || hasSpaces(password) == 0)
    {
        printf("ERROR - Invalid credentials\n");
        free(username);
        free(password);
        return;
    }

    // removing the newline character from the end of the strings
    if (username[strlen(username) - 1] == '\n')
    {
        username[strlen(username) - 1] = '\0';
    }
    if (password[strlen(password) - 1] == '\n')
    {
        password[strlen(password) - 1] = '\0';
    }

    // creating the JSON object with the credentials to create the request
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *body_data[] = {serialized_string};

    char* message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", body_data, 1, NULL, 0, NULL, 0);
                
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);
    
    // checking the response to see if the registration was successful
    if (strstr(response, "201 Created") != NULL)
    {
        printf("SUCCES - Registration complete\n");
    } else {
        printf("ERROR - Username already taken\n");
    }
    
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    free(username);
    free(password);
    free(response);
    free(message);
}

// function called when the user wants to log into the account
char* login(int socketfd) {
    // getting the credentials from the user
    char *username = calloc(100, sizeof(char));
    printf("username=");
    fgets(username, 100, stdin);

    char *password = calloc(100, sizeof(char));
    printf("password=");
    fgets(password, 100, stdin);

    // checking if the credentials are valid
    if (checkValidity(username) == 0 || checkValidity(password) == 0 ||
        hasSpaces(username) == 0 || hasSpaces(password) == 0)
    {
        printf("ERROR - Invalid credentials\n");
        free(username);
        free(password);
        return NULL;
    }

    // removing the newline character from the end of the strings
    if (username[strlen(username) - 1] == '\n')
    {
        username[strlen(username) - 1] = '\0';
    }
    if (password[strlen(password) - 1] == '\n')
    {
        password[strlen(password) - 1] = '\0';
    }

    // creating the JSON object with the credentials to create the request
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *body_data[] = {serialized_string};

    char* message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", body_data, 1, NULL, 0, NULL, 0);

    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the login was successful
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES - Logged in successfully\n");
    } else {
        printf("ERROR - Invalid username or password\n");
        free(username);
        free(password);
        free(message);
        free(response);
        return NULL;
    }

    // getting the cookie from the response
    char *cookie = NULL;
    char *cookie_start = strstr(response, "Set-Cookie: ");
    if (cookie_start != NULL) {
        cookie_start += strlen("Set-Cookie: ");
        char *cookie_end = strstr(cookie_start, ";");
        if (cookie_end != NULL) {
            *cookie_end = '\0';
        }
        cookie = strdup(cookie_start);
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    free(username);
    free(password);
    free(message);
    free(response);

    // returning the cookie to check if the user is logged in
    return cookie;
}

//function called when the user wants to gain access to the library
char* enter_library(int socketfd, char* cookie) {
    char* jwt = NULL;
    
    // creating the request with the cookie to check if user is logged in
    // and to get access to the library
    char **cookies = calloc(1, sizeof(char*));
    cookies[0] = cookie;

    char* message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, NULL, 0, cookies, 1);
    free(cookies);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the user has access to the library
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES - Welcome to library\n");
    } else {
        printf("ERROR - No access for library");
        return NULL;
    }

    // getting the JWT token from the response
    char *jwt_start = strstr(response, "{\"token\":\"");
    if (jwt_start != NULL) {
        jwt_start += strlen("{\"token\":\"");
        char *jwt_end = strstr(jwt_start, "\"");
        if (jwt_end != NULL) {
            *jwt_end = '\0';
        }
        jwt = strdup(jwt_start);
    }

    free(message);
    free(response);

    // returning the JWT token to check if the user has access to the library
    return jwt;
}

// function called when the user wants to see all the books in the library
void get_books(int socketfd, char* jwt) {

    // creating the request with the JWT token to check if
    // the user has access to the library and to get the books
    char **headers = calloc(1, sizeof(char*));
    headers[0] = jwt;

    char* message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, headers, 1, NULL, 0);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the user got the books
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES\n");

        char* books = strstr(response, "[");
        printf("%s\n", books);
    } else {
        printf("ERROR - No access for library\n");
    }

    free(message);
}

// function called when the user wants to get the details of a book
void get_book(int socketfd, char* jwt) {

    // creating the request with the JWT token to check if
    // the user has access to the library and to get the book
    char **headers = calloc(1, sizeof(char*));
    headers[0] = jwt;

    // getting the book id from the user
    printf("id = ");
    char* book_id = calloc(100, sizeof(char));
    fgets(book_id, 100, stdin);

    // removing the newline character from the end of the string
    if (book_id[strlen(book_id) - 1] == '\n')
    {
        book_id[strlen(book_id) - 1] = '\0';
    }

    // checking if the book id is valid
    if (checkValidity(book_id) == 0)
    {
        printf("ERROR - Invalid book id\n");
        free(book_id);
        return;
    }

    // checking if the book id is a number
    if (checkNum(book_id) == 0)
    {
        printf("ERROR - Invalid book id\n");
        free(book_id);
        return;
    }

    // computing the request with the book id to get the book information
    char* my_url = calloc(100, sizeof(char));
    strcpy(my_url, "/api/v1/tema/library/books/");

    strcat(my_url, book_id);
    if (my_url[strlen(my_url) - 1] == '\n')
    {
        my_url[strlen(my_url) - 1] = '\0';
    }
    
    char* message = compute_get_request(HOST, my_url, NULL, headers, 1, NULL, 0);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if any book was found
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES\n");
        char* books = strstr(response, "{\"id\"");
        printf("%s\n", books);
    } else {
        printf("ERROR - No book was found\n");
    }

    free(book_id);
    free(my_url);
    free(message);
}

// function called when the user wants to add a book in the library
void add_book(int socketfd, char* jwt) {
    char **headers = calloc(1, sizeof(char*));
    headers[0] = jwt;

    // getting the book details from the user
    printf("title=");
    char* title = calloc(100, sizeof(char));
    fgets(title, 100, stdin);

    printf("author=");
    char* author = calloc(100, sizeof(char));
    fgets(author, 100, stdin);

    printf("genre=");
    char* genre = calloc(100, sizeof(char));
    fgets(genre, 100, stdin);

    printf("publisher=");
    char* publisher = calloc(100, sizeof(char));
    fgets(publisher, 100, stdin);

    printf("page_count=");
    char* page_count = calloc(100, sizeof(char));
    fgets(page_count, 100, stdin);

    // checking if the book details are valid
    if (checkValidity(title) == 0 || checkValidity(author) == 0 ||
        checkValidity(genre) == 0 || checkValidity(publisher) == 0)
    {
        printf("ERROR - Invalid book information\n");
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        return;
    }

    // removing the newline character from the end of the strings
    if (title[strlen(title) - 1] == '\n')
    {
        title[strlen(title) - 1] = '\0';
    }
    if (author[strlen(author) - 1] == '\n')
    {
        author[strlen(author) - 1] = '\0';
    }
    if (genre[strlen(genre) - 1] == '\n')
    {
        genre[strlen(genre) - 1] = '\0';
    }
    if (publisher[strlen(publisher) - 1] == '\n')
    {
        publisher[strlen(publisher) - 1] = '\0';
    }
    if (page_count[strlen(page_count) - 1] == '\n')
    {
        page_count[strlen(page_count) - 1] = '\0';
    }

    // checking if the page count is a number
    if (checkNum(page_count) == 0)
    {
        printf("ERROR - Invalid page count\n");
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        return;
    }

    // creating the JSON object with the book details to create the request
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", my_atoi(page_count));
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *body_data[] = {serialized_string};
    char *message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", body_data, 1, headers, 1, NULL, 0);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the book was added
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES - Book added\n");
    } else {
        printf("ERROR - Invalid book information\n");
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    free(title);
    free(author);
    free(genre);
    free(publisher);
    free(page_count);
    free(message);
}

// function called when the user wants to delete a specific book
void delete_book(int socketfd, char* jwt) {
    // creating the request with the JWT token to check if
    // the user has access to the library and to delete the book
    char **headers = calloc(1, sizeof(char*));
    headers[0] = jwt;

    printf("id = ");
    char* book_id = calloc(100, sizeof(char));
    fgets(book_id, 100, stdin);

    char* my_url = calloc(100, sizeof(char));
    strcpy(my_url, "/api/v1/tema/library/books/");

    if (book_id[strlen(book_id) - 1] == '\n')
    {
        book_id[strlen(book_id) - 1] = '\0';
    }

    if (checkNum(book_id) == 0)
    {
        printf("ERROR - Invalid book id\n");
        free(book_id);
        free(my_url);
        return;
    }

    strcat(my_url, book_id);
    if (my_url[strlen(my_url) - 1] == '\n')
    {
        my_url[strlen(my_url) - 1] = '\0';
    }

    char* message = compute_delete_request(HOST, my_url, NULL, headers, 1, NULL, 0);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the book was deleted
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES - Book deleted\n");
    } else {
        printf("ERROR - No book was found\n");
    }

    free(book_id);
    free(my_url);
    free(message);
}

// function called when the user wants to log out of the account
void logout(int socketfd, char** cookie, char** jwt) {
    // creating the request with the cookie to log out of the account
    char **cookies = calloc(1, sizeof(char*));
    cookies[0] = *cookie;

    char* message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, NULL, 0, cookies, 1);
    send_to_server(socketfd, message);
    char *response = receive_from_server(socketfd);

    // checking the response to see if the user was logged out
    if (strstr(response, "200 OK") != NULL)
    {
        printf("SUCCES - Logged out\n");
    } else {
        printf("ERROR - Something went wrong\n");
    }

    // freeing the cookie and JWT token to make sure
    // the user is logged out and doesn't have access to the library anymore
    *cookie = NULL;
    *jwt = NULL;

    free(message);
    free(response);
}

int main(int argc, char *argv[])
{
    int sockfd;
    char *command = calloc(MAXCOMMANDLEN, sizeof(char));
    char *cookie = NULL;
    char *jwt = NULL;

    // opening and closing connection for each command
    // to avoid the server closing the connection;
    // read the command and call the appropriate function
    while (1)
    {
        memset(command, 0, MAXCOMMANDLEN);
        fgets(command, MAXCOMMANDLEN, stdin);

        if (command[strlen(command) - 1] == '\n')
        {
            command[strlen(command) - 1] = '\0';
        }

        if(strcmp(command, "exit") == 0) 
        {
            break;
        } else {
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                perror("Error opening connection");
                return 1;
            }
            // for register and login commands check if the user is already logged in
            // if not, call the appropriate function
            if (strcmp(command, "register") == 0)
            {
                if (cookie != NULL)
                {
                    printf("ERROR - You must logout before registering\n");
                } else {
                    register_user(sockfd);
                }
            } else if (strcmp(command, "login") == 0)
            {
                if (cookie != NULL)
                {
                    printf("ERROR - You must logout before logging in\n");
                } else {
                    cookie = login(sockfd);
                }
                // for enter_library and logout commands check only if the user is logged in
                // if yes, call the appropriate function
            } else if (strcmp(command, "enter_library") == 0)
            { 
                if (cookie == NULL)
                {
                    printf("ERROR - You must login before entering the library\n");
                } else {
                    jwt = enter_library(sockfd, cookie);
                }
                // for the rest of the commands check if the user is logged in and has access to the library
            } else if (strcmp(command, "get_books") == 0)
            {
                if (cookie == NULL && jwt == NULL) 
                {
                    printf("ERROR - You must login and enter library before getting the books\n");
                } else if (jwt == NULL)
                {
                    printf("ERROR - You must enter the library before getting the books\n");
                } else {
                    get_books(sockfd, jwt);
                }
            } else if (strcmp(command, "get_book") == 0)
            {
                if (cookie == NULL && jwt == NULL) 
                {
                    printf("ERROR - You must login and enter library before getting a book\n");
                } else if (jwt == NULL)
                {
                    printf("ERROR - You must enter the library before getting a book\n");
                } else {
                    get_book(sockfd, jwt);
                }
                
            } else if (strcmp(command, "add_book") == 0)
            {                
                if (cookie == NULL && jwt == NULL) 
                {
                    printf("ERROR - You must login and enter library before adding a book\n");
                } else if (jwt == NULL)
                {
                    printf("ERROR - You must enter the library before adding a book\n");
                } else {
                    add_book(sockfd, jwt);
                }
            } else if (strcmp(command, "delete_book") == 0)
            {
                if (cookie == NULL && jwt == NULL) 
                {
                    printf("ERROR - You must login and enter library before deleting a book\n");
                } else if (jwt == NULL)
                {
                    printf("ERROR - You must enter the library before deleting a book\n");
                } else {
                    delete_book(sockfd, jwt);
                }
            } else if (strcmp(command, "logout") == 0)
            {
                if (cookie == NULL)
                {
                    printf("ERROR - You must login before logging out\n");
                } else {
                    logout(sockfd, &cookie, &jwt);
                }
            } else
            {
                printf("Invalid command\n");
            }
            close_connection(sockfd);
        }

    }

    free(command);

    return 0;
}
