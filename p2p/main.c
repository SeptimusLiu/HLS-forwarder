//
//  main.c
//  p2p
//
//  Created by septimus on 15/3/9.
//  Copyright (c) 2015年 SEPTIMUS. All rights reserved.
//

#include <stdio.h>
#include "http_request_util.h"
#include "http_server.h"
#include "utils.h"
#include <event2/buffer.h>
#include <sys/stat.h>

#define PORT 8081
#define HOST "127.0.0.1"

#define SOURCE_HOST "http://10.3.220.152/hls/cctv5hd.m3u8"
#define SERVER_NAME "http://10.3.220.152/hls/"
#define FILE_NAME_CHUNKLIST "chunklist.m3u8"
#define FILE_NAME_MEDIA "media.ts"
#define DEFAULT_FILE "chunklist.m3u8"
#define TIMEOUT 5

struct event_base* base;
struct event *ev;
struct event *ev_file;
struct timeval tv;
struct timeval tv_file;
struct http_request_get *http_req_get;

char *filedata;
time_t lasttime = 0;
char filename[80];
int counter = 0;
struct evhttp * http_server;
unsigned long size = 0;

pthread_mutex_t mutex;

/** Http Callback Function **/

void* httpserver_dispatch(void *arg);

void chunklist_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_get *http_req_get = (struct http_request_get *)arg;
    
    
    switch(req->response_code)
    {
        case HTTP_OK:
        {
//            pthread_mutex_lock (&mutex);

            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            printf("print the head info:\n");
            print_request_head_info(req->output_headers);
            
            printf("len:%zu  body size:%zu\n", len, req->body_size);
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            
            FILE *storage;
            storage = fopen(FILE_NAME_CHUNKLIST, "wt");
            if (storage == NULL)
            {
                printf("Cannot open file!");
                return;
            }
            printf("File opened successfully\n");
            
            char *chunklist = malloc(len + 1);
            strcpy(chunklist, tmp);
            char* blob = chunklist;
            
            if (chunklist)
            {
                while (1) {
                    char* media_prefix = strstr(blob, "cctv");
                    char* chunklist_new = realloc(chunklist, strlen(chunklist) + strlen(SERVER_NAME) + 1);
                    if (!media_prefix)
                    {
                        if (!chunklist_new)
                        {
                            free(chunklist_new);
                            chunklist_new = NULL;
                        }
                        break;
                    }
                    
                    if (!chunklist_new)
                    {
                        printf("realloc error!\n");
                        return;
                    }
                    media_prefix += chunklist_new - chunklist;
                    chunklist = chunklist_new;
                    insert_substring(chunklist, SERVER_NAME, (int)(media_prefix - chunklist) + 1);
                    blob = media_prefix + strlen(SERVER_NAME) + 4;
                }
            }
            
            
            printf("media file: \n%s", chunklist);
            
            
//            char *media1_start = strstr(tmp, "cctv");
//            char *media1_end = strstr(media1_start, "\n");
//            char *media1 = malloc(media1_end - media1_start);
//            memcpy(media1, media1_start, media1_end - media1_start);
//            media1[media1_end - media1_start] = '\0';
//            printf("media segment: %s\n", media1);
//            char *server_prefix = malloc(strlen(SERVER_NAME) + strlen(media1));
//            memset(server_prefix, 0, strlen(server_prefix));
//            memcpy(server_prefix, SERVER_NAME, strlen(SERVER_NAME));
//            server_prefix[strlen(SERVER_NAME)] = '\0';
//            char *media_dir = strcat(server_prefix, media1);
//            media_dir[strlen(SERVER_NAME) + strlen(media1)] = '\0';
            
            fprintf(storage, "%s\n", chunklist);
            
            fclose(storage);
            
//            struct http_request_get *http_req_get_new = http_request_new(base, media_dir, REQUEST_GET_FLAG, NULL, NULL);
//            start_media_request(http_req_get_new);

            
            if (!chunklist)
            {
                free(chunklist);
                chunklist = NULL;
            }
            
            if (!tmp)
            {
                free(tmp);
                tmp = NULL;
            }
//            free(media1);
//            free(server_prefix);
//            pthread_mutex_unlock (&mutex);


            break;
        }
        case HTTP_MOVEPERM:
            printf("%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_get->uri);
            http_req_get->uri = new_uri;
            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }
            
        default:
            event_base_loopexit(http_req_get->base, 0);
            return;
    }
    
}

//void download_media(char* uri, struct evbuffer* buf)
//{
//    FILE *storage;
//    storage = fopen(FILE_NAME_MEDIA, "wt");
//    if (storage == NULL)
//    {
//        printf("Cannot open download!");
//        return;
//    }
//    printf("Downloading %s\n", uri);
//    
//    size_t len = evbuffer_get_length(buf);
//    char *tmp = malloc(len + 1);
//    fwrite(evbuffer_pullup(buf, -1), len, 1, storage);
//    free(tmp);
//    
//    fclose(storage);
//}

void download_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_get *http_req_get = (struct http_request_get *)arg;
    FILE *storage;
    storage = fopen(FILE_NAME_MEDIA, "wt");
    if (storage == NULL)
    {
        printf("Cannot open download!");
        return;
    }
    printf("Downloading %s\n", (char*)http_req_get->uri);
    
    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            
            printf("len:%zu  media size:%zu\n", len, req->body_size);
            
//            process_func((char*)http_req_get->uri, buf);
            char *tmp = malloc(len+1);
            fwrite(evbuffer_pullup(buf, -1), len, 1, storage);
            free(tmp);
            
            fclose(storage);
            break;
        }
        case HTTP_MOVEPERM:
            printf("%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_get->uri);
            http_req_get->uri = new_uri;
            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }
            
        default:
            event_base_loopexit(http_req_get->base, 0);
            return;
    }
    
}

/** Http Request Function **/

int start_url_request(struct http_request_get *http_req, int req_get_flag)
{
    if (http_req->cn)
        evhttp_connection_free(http_req->cn);
    
    int port = evhttp_uri_get_port(http_req->uri);
    http_req->cn = evhttp_connection_base_new(http_req->base,
                                              NULL,
                                              evhttp_uri_get_host(http_req->uri),
                                              (port == -1 ? 80 : port));
    
    /**
     * Request will be released by evhttp connection
     * See info of evhttp_make_request()
     */
    if (req_get_flag == REQUEST_POST_FLAG) {
        http_req->req = evhttp_request_new(http_requset_post_cb, http_req);
    } else if (req_get_flag ==  REQUEST_GET_FLAG) {
        http_req->req = evhttp_request_new(chunklist_cb, http_req);
    }
    
    if (req_get_flag == REQUEST_POST_FLAG) {
        const char *path = evhttp_uri_get_path(http_req->uri);
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_POST,
                            path ? path : "/");
        /** Set the post data */
        struct http_request_post *http_req_post = (struct http_request_post *)http_req;
        evbuffer_add(http_req_post->req->output_buffer, http_req_post->post_data, strlen(http_req_post->post_data));
        evhttp_add_header(http_req_post->req->output_headers, "Content-Type", http_req_post->content_type);
    } else if (req_get_flag == REQUEST_GET_FLAG) {
        const char *query = evhttp_uri_get_query(http_req->uri);
        const char *path = evhttp_uri_get_path(http_req->uri);
        size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
        char *path_query = NULL;
        if (len > 1) {
            path_query = calloc(len, sizeof(char));
            sprintf(path_query, "%s?%s", path, query);
        }
//        evhttp_request_set_chunked_cb(http_req->req, http_requset_download_cb);
        evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                            path_query ? path_query: "/");
    }
    /** Set the header properties */
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));
    evhttp_add_header(http_req->req->output_headers, "X-Playback-Session-Id", SESSION_ID);
    
    return 0;
}

int start_chunklist_request(struct http_request_get *http_req)
{
    if (http_req->cn)
        evhttp_connection_free(http_req->cn);
    
    int port = evhttp_uri_get_port(http_req->uri);
    http_req->cn = evhttp_connection_base_new(http_req->base,
                                              NULL,
                                              evhttp_uri_get_host(http_req->uri),
                                              (port == -1 ? 80 : port));
    
    /**
     * Request will be released by evhttp connection
     * See info of evhttp_make_request()
     */
    http_req->req = evhttp_request_new(chunklist_cb, http_req);

    const char *query = evhttp_uri_get_query(http_req->uri);
    const char *path = evhttp_uri_get_path(http_req->uri);
    size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
    char *path_query = NULL;
    if (len > 1) {
        path_query = calloc(len, sizeof(char));
        sprintf(path_query, "%s?%s", path, query);
    }
    //        evhttp_request_set_chunked_cb(http_req->req, http_requset_download_cb);
    evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                            path_query ? path_query: "/");
    /** Set the header properties */
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));
    evhttp_add_header(http_req->req->output_headers, "X-Playback-Session-Id", SESSION_ID);
    
    return 0;
}

int start_media_request(struct http_request_get *http_req)
{
    if (http_req->cn)
        evhttp_connection_free(http_req->cn);
    
    int port = evhttp_uri_get_port(http_req->uri);
    http_req->cn = evhttp_connection_base_new(http_req->base,
                                              NULL,
                                              evhttp_uri_get_host(http_req->uri),
                                              (port == -1 ? 80 : port));

    http_req->req = evhttp_request_new(download_cb, http_req);
    
    const char *query = evhttp_uri_get_query(http_req->uri);
    const char *path = evhttp_uri_get_path(http_req->uri);
    size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
    char *path_query = NULL;
    if (len > 1) {
        path_query = calloc(len, sizeof(char));
        sprintf(path_query, "%s?%s", path, query);
    }
    //        evhttp_request_set_chunked_cb(http_req->req, http_requset_download_cb);
    evhttp_make_request(http_req->cn, http_req->req, EVHTTP_REQ_GET,
                        path_query ? path_query: "/");
    /** Set the header properties */
    evhttp_add_header(http_req->req->output_headers, "Host", evhttp_uri_get_host(http_req->uri));
    evhttp_add_header(http_req->req->output_headers, "X-Playback-Session-Id", SESSION_ID);
    
    return 0;
}


void request_timer_cb(evutil_socket_t listener, short event, void *arg)
{
    if (!evtimer_pending(ev, NULL))
    {
        event_del(ev);
        evtimer_add(ev, &tv);
    }
    
    http_req_get = start_http_requset(base, SOURCE_HOST, REQUEST_GET_FLAG, NULL, NULL);
}

/************************** Start POST/GET Function ******************************/
/**
 * @param content_type: refer HTTP_CONTENT_TYPE_*
 */
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, \
                         const char *content_type, const char* data)
{
    struct http_request_get *http_req_get = http_request_new(base, url, req_get_flag, content_type, data);
    start_url_request(http_req_get, req_get_flag);
    
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    
    ev = evtimer_new(base, request_timer_cb, NULL);
    
    evtimer_add(ev, &tv);
    
    return http_req_get;
}

/** File Operating Function **/
void read_file()
{
    
    struct stat buf;
    
    if (stat(filename, &buf) < 0)
    {
        printf("Read file error \n");
        return;
    }
    
    if (buf.st_mtime > lasttime)
    {
        if (counter++)
            fprintf(stderr, "Reloading file: %s", filename);
        else
            fprintf(stderr, "Loading file: %s", filename);
        
        FILE *f = fopen(filename, "rb");
        if (f == NULL)
        {
            fprintf(stderr, "Couldn't open file\n");
            return;
        }
        
        size = buf.st_size;
        filedata = (char *)malloc(size + 1);
        memset(filedata, 0, size + 1);
        fread(filedata, sizeof(char), size, f);
        fclose(f);
        
        fprintf(stderr, " (%ld bytes)\n", size);
        lasttime = buf.st_mtime;
    }
}

void read_file_timer_cb(evutil_socket_t listener, short event, void *arg)
{
    if (!evtimer_pending(ev_file, NULL))
    {
        event_del(ev_file);
        evtimer_add(ev_file, &tv_file);
    }
    
    read_file();
}

void load_file(struct event_base *base) {
    tv_file.tv_sec = 5;
    tv_file.tv_usec = 0;
    
    ev_file = evtimer_new(base, read_file_timer_cb, NULL);
    
    evtimer_add(ev_file, &tv_file);
}

void generic_handler(struct evhttp_request *req, void *arg)
{
    struct evbuffer *buf = evbuffer_new();
    char size_str[16];
    sprintf(size_str, "%d", size);
    if(!buf)
    {
        puts("failed to create response buffer \n");
        return;
    }
    if (strstr(req->uri, ".ts"))
    {
        evhttp_add_header(req->output_headers, "Content-Type", "video/mp2t");
    } else {
        evhttp_add_header(req->output_headers, "Content-Type", "application/vnd.apple.mpegurl");
    }
    
//    evbuffer_add_printf(buf, "%s", filedata);
    evbuffer_add(buf, filedata, size);
    
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

int start_http_server()
{
    short http_port = PORT;
    char *http_addr = HOST;
    strcpy(filename, DEFAULT_FILE);
    
    http_server = evhttp_new(base);
    if(!http_server)
    {
        return 0;
    }
    
    int ret = evhttp_bind_socket(http_server,http_addr,http_port);
    if(ret!=0)
    {
        return 0;
    }
    
    evhttp_set_gencb(http_server, generic_handler, NULL);
    
    read_file();
    load_file(base);
    
    printf("http server start OK! \n");
    return 1;
}

int httpserver_start(int port, int nthreads, int backlog)
{
    int r, i;
    int nfd = httpserver_bindsocket(port, backlog);
    if (nfd < 0)
        return -1;
    pthread_t ths[nthreads];
    for (i = 0; i < nthreads; i++) {
        struct event_base *base_new = event_base_new();
        if (base_new == NULL) return -1;
        
        if (i == 0)
        {
            strcpy(filename, DEFAULT_FILE);
            read_file();
            load_file(base_new);
        }
        if (i == 1)
        {
            base = base_new;
            http_req_get = start_http_requset(base_new, SOURCE_HOST, REQUEST_GET_FLAG, NULL, NULL);
        } else {
            struct evhttp *httpd = evhttp_new(base_new);
            if (httpd == NULL) return -1;
            r = evhttp_accept_socket(httpd, nfd);
            if (r != 0) return -1;
            evhttp_set_gencb(httpd, generic_handler, NULL);
        }
        r = pthread_create(&ths[i], NULL, httpserver_dispatch, base_new);
        if (r != 0) return -1;
    }
    for (i = 0; i < nthreads; i++) {
        pthread_join(ths[i], NULL);
    }
    return 0;
}

void* httpserver_dispatch(void *arg) {
    event_base_dispatch((struct event_base*)arg);
    return NULL;
}

void gc()
{
    http_request_free(http_req_get, REQUEST_GET_FLAG);
    //    http_request_free((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);
    event_base_free(base);
    evhttp_free(http_server);
}

int main(int argc, char *argv[])
{
//    base = event_base_new();
    
//    if (!start_http_server())
//    {
//        printf("http server start failed! \n");
//        return -1;
//    }
    pthread_mutex_init(&mutex, NULL);

    httpserver_start(PORT, 2, 10240);
    
//    struct http_request_post *http_req_post = start_http_requset(base,
//                                                                 "http://172.16.239.93:8899/base/truck/delete",
//                                                                 REQUEST_POST_FLAG,
//                                                                 HTTP_CONTENT_TYPE_URL_ENCODED,
//                                                                 "name=winlin&code=1234");
//    http_req_get = start_http_requset(base, SOURCE_HOST, REQUEST_GET_FLAG, NULL, NULL);
////
//    event_base_dispatch(base);
    gc();
    
    return 0;
}
