/*
     This file is part of libhttpserver
     Copyright (C) 2011, 2012, 2013, 2014, 2015 Sebastiano Merlino

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
     USA
*/

#include <iostream>

#include <httpserver.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <inference.hpp>



class hello_world_resource : public httpserver::http_resource {
 public:
     std::shared_ptr<httpserver::http_response> render(const httpserver::http_request&);

};

class inference_resource : public httpserver::http_resource {
 public:
     std::shared_ptr<httpserver::http_response> render(const httpserver::http_request&);

};

std::string cpu_bound_function() {
    const int NUM_ITERATIONS = 500000000;
    double result = 0.0;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        result += i * i;
    }
//    std::cout << "Result: " << result +rand()%10 << std::endl;
    return std::to_string(result +rand()%10);
}
// Using the render method you are able to catch each type of request you receive
std::shared_ptr<httpserver::http_response> hello_world_resource::render(const httpserver::http_request& req) {
    std::string result = cpu_bound_function();
    return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(result, 200));
}

// Using the render method you are able to catch each type of request you receive
std::shared_ptr<httpserver::http_response> inference_resource::render(const httpserver::http_request& req) {
    std:: string data(req.get_content());
    std:: string result = infer(data);
    return std::shared_ptr<httpserver::http_response>(new httpserver::string_response(result, 200));
}

int main() {
    // It is possible to create a webserver passing a great number of parameters. In this case we are just passing the port and the number of thread running.
    at::init_num_threads();
    // std::cout<<at::get_num_threads<<std::endl;
    at::set_num_threads(1);
    at::set_num_interop_threads(1);
    struct sockaddr_in bind_address;
    memset(&bind_address, 0, sizeof(bind_address));
    bind_address.sin_family = AF_INET;
    bind_address.sin_addr.s_addr = INADDR_ANY;
    bind_address.sin_port = htons(7000);
    unsigned int num_threads = std::thread::hardware_concurrency();
    // std::cout << "Number of threads: " << num_threads << std::endl;
    httpserver::webserver ws = httpserver::create_webserver(7000)
        .bind_address(reinterpret_cast<const struct sockaddr*>(&bind_address))
        // .start_method(httpserver::http::http_utils::INTERNAL_SELECT)
        // .max_threads(2);
        .start_method(httpserver::http::http_utils::THREAD_PER_CONNECTION);

    hello_world_resource hwr;
    inference_resource ir;
    // This way we are registering the hello_world_resource to answer for the endpoint
    // "/hello". The requested method is called (if the request is a GET we call the render_GET
    // method. In case that the specific render method is not implemented, the generic "render"
    // method is called.
    ws.register_resource("/cpu", &hwr, true);
    ws.register_resource("/infer", &ir, true);

    // This way we are putting the created webserver in listen. We pass true in order to have
    // a blocking call; if we want the call to be non-blocking we can just pass false to the method.
    ws.start(true);
    return 0;
}
