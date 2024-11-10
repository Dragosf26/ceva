#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **headers, int headers_count, char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (headers != NULL) {
        strcat(message, "Authorization: Bearer ");
        for (int i = 0; i < headers_count; ++i) {
            compute_message(message, headers[i]);
        }
    }


    if (cookies != NULL) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; ++i) {
            strcat(message, cookies[i]);
            strcat(message, "; ");
        }
        compute_message(message, cookies[cookies_count - 1]);
    }

    // Step 4: add final new line
    compute_message(message, "");

    free(line);
    return message;
}


char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **headers, int headers_count, char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    
    // Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    
    int total_body_length = 0;
    for (int i = 0; i < body_data_fields_count; ++i) {
        total_body_length += strlen(body_data[i]);
    }
    if (total_body_length > 0) {
        total_body_length += body_data_fields_count - 1;
    }
    sprintf(line, "Content-Length: %d", total_body_length);
    compute_message(message, line);
    
    // Step 4 (optional): add cookies and headers
    if (headers != NULL) {
        strcat(message, "Authorization: Bearer ");
        for (int i = 0; i < headers_count; ++i) {
            compute_message(message, headers[i]);
        }
    }

    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; ++i) {
            compute_message(message, cookies[i]);
        }
    }
    
    // Step 5: add new line at end of header
    compute_message(message, "");
    
    // Step 6: add the actual payload data
    for (int i = 0; i < body_data_fields_count; ++i) {
        strcat(body_data_buffer, body_data[i]);
        if (i < body_data_fields_count - 1) {
            strcat(body_data_buffer, "&");
        }
    }
    strcat(message, body_data_buffer);

    free(line);
    free(body_data_buffer);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char **headers, int headers_count, char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (headers != NULL) {
        strcat(message, "Authorization: Bearer ");
        for (int i = 0; i < headers_count; ++i) {
            compute_message(message, headers[i]);
        }
    }

    if (cookies != NULL) {
        strcat(message, "Cookie: ");
        for (int i = 0; i < cookies_count - 1; ++i) {
            strcat(message, cookies[i]);
            strcat(message, "; ");
        }
        compute_message(message, cookies[cookies_count - 1]);
    }

    // Step 4: add final new line
    compute_message(message, "");
    
    free(line);
    return message;
}