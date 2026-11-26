#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

// Functions from booking.c
int getPrice(int movie);
int isBooked(int movie, int row, int col);
int bookSeat(int movie, int row, int col);
void resetBookings();

#define PORT 8080

void sendResponse(SOCKET client, const char *body)
{
    char response[4096];

    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n%s",
        (int)strlen(body), body);

    send(client, response, strlen(response), 0);
}

int main()
{
    WSADATA wsa;
    SOCKET server, client;
    struct sockaddr_in address;
    int addressLength = sizeof(address);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&address,
             sizeof(address)) == SOCKET_ERROR)
    {
        printf("Port binding failed.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    listen(server, 5);

    printf("Theatre Booking Server running!\n");
    printf("Address: http://localhost:8080\n");

    while (1)
    {
        client = accept(server,
            (struct sockaddr *)&address, &addressLength);

        if (client == INVALID_SOCKET)
            continue;

        char request[2048] = {0};

        recv(client, request, sizeof(request) - 1, 0);

        int movie, row, col;
        char response[256];

        if (sscanf(request,
            "GET /price?movie=%d", &movie) == 1)
        {
            sprintf(response,
                "{\"price\":%d}", getPrice(movie));
        }
        else if (sscanf(request,
            "GET /check?movie=%d&row=%d&col=%d",
            &movie, &row, &col) == 3)
        {
            sprintf(response,
                "{\"booked\":%d}",
                isBooked(movie, row, col));
        }
        else if (sscanf(request,
            "GET /book?movie=%d&row=%d&col=%d",
            &movie, &row, &col) == 3)
        {
            int result = bookSeat(movie, row, col);

            sprintf(response,
                "{\"success\":%d}", result);
        }
        else if (strncmp(request, "GET /reset", 10) == 0)
        {
            resetBookings();
            sprintf(response, "{\"success\":1}");
        }
        else
        {
            sprintf(response,
                "{\"message\":\"Invalid request\"}");
        }

        sendResponse(client, response);
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();

    return 0;
}