//
//  http_request_util.h
//  p2p
//
//  Created by septimus on 15/6/28.
//  Copyright (c) 2015年 SEPTIMUS. All rights reserved.
//

#ifndef __p2p__http_request_util__
#define __p2p__http_request_util__

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"
// (use for files: picture, mp3, tar-file etc.)
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"

#define SESSION_ID "565F0542-EC0B-453E-A63B-E48B078604C4"

#define REQUEST_POST_FLAG               2
#define REQUEST_GET_FLAG                3

struct http_request_get {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
};

struct http_request_post {
    struct evhttp_uri *uri;
    struct event_base *base;
    struct evhttp_connection *cn;
    struct evhttp_request *req;
    char *content_type;
    char *post_data;
};

/** Tools Function **/
static inline void print_request_head_info(struct evkeyvalq *header)
{
    struct evkeyval *first_node = header->tqh_first;
    while (first_node) {
        printf("key:%s  value:%s\n", first_node->key, first_node->value);
        first_node = first_node->next.tqe_next;
    }
}

static inline void print_uri_parts_info(const struct evhttp_uri * http_uri)
{
    printf("scheme:%s\n", evhttp_uri_get_scheme(http_uri));
    printf("host:%s\n", evhttp_uri_get_host(http_uri));
    printf("path:%s\n", evhttp_uri_get_path(http_uri));
    printf("port:%d\n", evhttp_uri_get_port(http_uri));
    printf("query:%s\n", evhttp_uri_get_query(http_uri));
    printf("userinfo:%s\n", evhttp_uri_get_userinfo(http_uri));
    printf("fragment:%s\n", evhttp_uri_get_fragment(http_uri));
}

void http_requset_post_cb(struct evhttp_request *req, void *arg);
void http_requset_get_cb(struct evhttp_request *req, void *arg);
int start_url_request(struct http_request_get *http_req, int req_get_flag);
int start_chunklist_request(struct http_request_get *http_req);
int start_media_request(struct http_request_get *http_req);
void *http_request_new(struct event_base* base, const char *url, int req_get_flag, \
                       const char *content_type, const char* data);
void *start_http_requset(struct event_base* base, const char *url, int req_get_flag, \
                         const char *content_type, const char* data);

#endif /* defined(__p2p__http_request_util__) */
