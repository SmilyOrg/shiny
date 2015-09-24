/*
* Copyright (c) 2013-2015 the CivetWeb developers
* Copyright (c) 2013 No Face Press, LLC
* License http://opensource.org/licenses/mit-license.php MIT License
*/

/* Simple example program on how to use Embedded C interface. */


#include <string.h>
#include "civetweb/civetweb.h"

#include <ostream>

#define DOCUMENT_ROOT "vis"
#define PORT "8511"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

#define ASSERT(cond) assert(cond)

int WebSocketConnectHandler(const struct mg_connection * conn, void *cbdata);
void WebSocketReadyHandler(struct mg_connection * conn, void *cbdata);
int WebsocketDataHandler(struct mg_connection * conn, int bits, char * data, size_t len, void *cbdata);
void WebSocketCloseHandler(const struct mg_connection * conn, void *cbdata);

int ExitHandler(struct mg_connection *conn, void *cbdata);

class Server {
    
    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    
public:
    bool exit;

    void start() {
        exit = false;
        
        const char * options[] = {
            "document_root", DOCUMENT_ROOT,
            "listening_ports", PORT,
            "request_timeout_ms", "10000",
            "websocket_timeout_ms", "3600000",
            0
        };
        
        
        memset(&callbacks, 0, sizeof(callbacks));
        ctx = mg_start(&callbacks, 0, options);
        
        /* Handler EXAMPLE_URI to explain the example */
        mg_set_request_handler(ctx, EXIT_URI, ExitHandler, this);
        
        /* HTTP site to open a websocket connection */
        //mg_set_request_handler(ctx, "/websocket", WebSocketStartHandler, 0);
        
        /* WS site for the websocket connection */
        mg_set_websocket_handler(ctx, "/websocket", WebSocketConnectHandler, WebSocketReadyHandler, WebsocketDataHandler, WebSocketCloseHandler, 0);
        
        cout << "Server running at http://localhost:" << PORT << "/" << endl;
    }
    
    void stop() {
        if (ctx == NULL) return;
        mg_stop(ctx);
        ctx = NULL;
        cout << "Server stopped" << endl;
    }
};

int ExitHandler(struct mg_connection *conn, void *cbdata)
{
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    mg_printf(conn, "Bye!\n");
    Server* server = (Server*) cbdata;
    server->exit = true;
    return 1;
}

/*
int WebSocketStartHandler(struct mg_connection *conn, void *cbdata)
{
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

    mg_printf(conn, "<!DOCTYPE html>\n");
    mg_printf(conn, "<html>\n<head>\n");
    mg_printf(conn, "<meta charset=\"UTF-8\">\n");
    mg_printf(conn, "<title>Embedded websocket example</title>\n");
#ifdef USE_WEBSOCKET
   mg_printf(conn, "<script>\n");
    mg_printf(conn,
    "function load() {\n"
    "  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
    "  connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');\n"
    "  websock_text_field = document.getElementById('websock_text_field');\n"
    "  connection.onmessage = function (e) {\n"
    "    websock_text_field.innerHTML=e.data;\n"
    "  }\n"
    "  connection.onerror = function (error) {\n"
    "    alert('WebSocket error');\n"
    "    connection.close();\n"
    "  }\n"
    "}\n"
    );
    mg_printf(conn, "</script>\n");
    mg_printf(conn, "</head>\n<body onload=\"load()\">\n");
    mg_printf(conn, "<div id='websock_text_field'>No websocket connection yet</div>\n");
#else
    mg_printf(conn, "</head>\n<body>\n");
    mg_printf(conn, "Example not compiled with USE_WEBSOCKET\n");
#endif
    mg_printf(conn, "</body>\n</html>\n");
    return 1;
}
*/

#define MAX_WS_CLIENTS 5

struct t_ws_client {
    struct mg_connection * conn;
    int state;
} static ws_clients[MAX_WS_CLIENTS];

int WebSocketConnectHandler(const struct mg_connection * conn, void *cbdata)
{
    struct mg_context *ctx = mg_get_context(conn);
    int reject = 1;
    int i;

    mg_lock_context(ctx);
    for (i=0; i<MAX_WS_CLIENTS; i++) {
        if (ws_clients[i].conn == NULL) {
            ws_clients[i].conn = (struct mg_connection *) conn;
            ws_clients[i].state = 1;
            mg_set_user_connection_data(conn, (void*) (ws_clients+i));
            reject = 0;
            break;
        }
    }
    mg_unlock_context(ctx);

    fprintf(stdout, "Websocket client %s\r\n\r\n", (reject ? "rejected" : "accepted"));
    return reject;
}

void WebSocketReadyHandler(struct mg_connection * conn, void *cbdata)
{
    const char * text = "Hello from the websocket ready handler";
    struct t_ws_client * client = (t_ws_client*) mg_get_user_connection_data(conn);

    mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
    fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
    ASSERT(client->conn == conn);
    ASSERT(client->state == 1);

    client->state = 2;
}

int WebsocketDataHandler(struct mg_connection * conn, int bits, char * data, size_t len, void *cbdata)
{
    struct t_ws_client * client = (t_ws_client*) mg_get_user_connection_data(conn);
    ASSERT(client->conn == conn);
    ASSERT(client->state >= 1);

    fprintf(stdout, "Websocket got data:\r\n");
    fwrite(data, len, 1, stdout);
    fprintf(stdout, "\r\n\r\n");

    return 1;
}

void WebSocketCloseHandler(const struct mg_connection * conn, void *cbdata)
{
    struct mg_context *ctx = mg_get_context(conn);
    struct t_ws_client * client = (t_ws_client*) mg_get_user_connection_data(conn);
    ASSERT(client->conn == conn);
    ASSERT(client->state >= 1);

    mg_lock_context(ctx);
    client->state = 0;
    client->conn = NULL;
    mg_unlock_context(ctx);

    fprintf(stdout, "Client droped from the set of webserver connections\r\n\r\n");
}

/*
void InformWebsockets(struct mg_context *ctx)
{
    char text[32];
    int i;

    sprintf(text, "%lu", ++cnt);

    mg_lock_context(ctx);
    for (i=0; i<MAX_WS_CLIENTS; i++) {
        if (ws_clients[i].state == 2) {
            mg_websocket_write(ws_clients[i].conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
        }
    }
    mg_unlock_context(ctx);
}
#endif
*/
