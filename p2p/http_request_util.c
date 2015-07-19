//
//  http_request_util.c
//  p2p
//
//  Created by septimus on 15/6/28.
//  Copyright (c) 2015年 SEPTIMUS. All rights reserved.
//

#include "http_request_util.h"



/** Construct Function **/
void *http_request_new(struct event_base* base, const char *url, int req_get_flag, \
                       const char *content_type, const char* data)
{
    int len = 0;
    if (req_get_flag == REQUEST_GET_FLAG) {
        len = sizeof(struct http_request_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        len = sizeof(struct http_request_post);
    }
    
    struct http_request_get *http_req_get = calloc(1, len);
    http_req_get->uri = evhttp_uri_parse(url);
    //    print_uri_parts_info(http_req_get->uri);
    
    http_req_get->base = base;
    
    if (req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post *)http_req_get;
        if (content_type == NULL) {
            content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
        }
        http_req_post->content_type = strdup(content_type);
        
        if (data == NULL) {
            http_req_post->post_data = NULL;
        } else {
            http_req_post->post_data = strdup(data);
        }
    }
    
    return http_req_get;
}


void http_request_free(struct http_request_get *http_req_get, int req_get_flag)
{
    evhttp_connection_free(http_req_get->cn);
    evhttp_uri_free(http_req_get->uri);
    if (req_get_flag == REQUEST_GET_FLAG) {
        free(http_req_get);
    } else if(req_get_flag == REQUEST_POST_FLAG) {
        struct http_request_post *http_req_post = (struct http_request_post*)http_req_get;
        if (http_req_post->content_type) {
            free(http_req_post->content_type);
        }
        if (http_req_post->post_data) {
            free(http_req_post->post_data);
        }
        free(http_req_post);
    }
    http_req_get = NULL;
}

/* Post Request Function */
void http_requset_post_cb(struct evhttp_request *req, void *arg)
{
    struct http_request_post *http_req_post = (struct http_request_post *)arg;
    switch(req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            printf("print the head info:\n");
            print_request_head_info(req->output_headers);
            
            printf("len:%zu  body size:%zu\n", len, req->body_size);
            char *tmp = malloc(len+1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            printf("print the body:\n");
            printf("HTML BODY:%s\n", tmp);
            free(tmp);
            
            event_base_loopexit(http_req_post->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            printf("%s", "the uri moved permanently\n");
            break;
        case HTTP_MOVETEMP:
        {
            const char *new_location = evhttp_find_header(req->input_headers, "Location");
            struct evhttp_uri *new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_post->uri);
            http_req_post->uri = new_uri;
            start_url_request((struct http_request_get *)http_req_post, REQUEST_POST_FLAG);
            return;
        }
            
        default:
            event_base_loopexit(http_req_post->base, 0);
            return;
    }
}