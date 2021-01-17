#include <cmath> // rand()
#include <sstream>

#include <net/http/request.hpp>
#include <net/http/response.hpp>
#include <net/http/server.hpp>
#include <net/iana.hpp>
#include <net/interfaces>
#include <os>
#include <timers>

#include <hal/machine.hpp>

#include "../bitvisor/bitvisor.hpp"

#define SEND_BUF_LEN 100
#define SERVER_TEST_LEN 10
#define PACKETS_PER_INTERVAL 2
#define NCAT_RECEIVE_PORT 9000
#define HTTP_SERVE_PORT 80

uint64_t data_len{0};
std::unique_ptr<http::Server> server;

using namespace std::chrono;

std::string HTML_RESPONSE() {
  const int color = rand();

  // Generate some HTML
  std::stringstream stream;
  stream << "<!DOCTYPE html><html><head>"
         << "<link href='https://fonts.googleapis.com/css?family=Ubuntu:500,300'"
         << " rel='stylesheet' type='text/css'>"
         << "<title>IncludeOS Demo Service</title></head><body>"
         << "<h1 style='color: #" << std::hex << ((color >> 8) | 0x020202)
         << "; font-family: \"Arial\", sans-serif'>"
         << "Include<span style='font-weight: lighter'>OS</span></h1>"
         << "<h2>The C++ Unikernel</h2>"
         << "<p>You have successfully booted an IncludeOS TCP service with simple http. "
         << "For a more sophisticated example, take a look at "
         << "<a href='https://github.com/hioa-cs/IncludeOS/tree/master/examples/acorn'>Acorn</a>.</p>"
         << "<footer><hr/>&copy; 2017 IncludeOS </footer></body></html>";

  return stream.str();
}

void root_handler(http::Request_ptr req, http::Response_writer_ptr writer) {
  // auto res = handle_request(*req);
  // set content type
  writer->header().set_field(http::header::Server, "IncludeOS/0.10");

  // GET /
  if (req->method() == http::GET && req->uri().to_string() == "/") {
    // add HTML response
    writer->write(HTML_RESPONSE());

    // set Content type and length
    writer->header().set_field(http::header::Content_Type, "text/html; charset=UTF-8");
    // writer->header().set_content_length();
  } else {
    // Generate 404 response
    writer->write_header(http::Not_Found);
  }
  writer->header().set_field(http::header::Connection, "close");
}

http::Response handle_request(const http::Request &req) {
  printf("<Service> Request:\n%s\n", req.to_string().c_str());

  http::Response res;

  auto &header = res.header();

  header.set_field(http::header::Server, "IncludeOS/0.10");

  // GET /
  if (req.method() == http::GET && req.uri().to_string() == "/") {
    // add HTML response
    res.add_body(HTML_RESPONSE());

    // set Content type and length
    header.set_field(http::header::Content_Type, "text/html; charset=UTF-8");
    header.set_field(http::header::Content_Length, std::to_string(res.body().size()));
  } else {
    // Generate 404 response
    res.set_status_code(http::Not_Found);
  }

  header.set_field(http::header::Connection, "close");

  return res;
}

void send_cb() {
  data_len += SEND_BUF_LEN;
}

void send_data(net::udp::Socket &client, net::Inet &inet) {
  // const net::Addr destip = net::Addr(192,168,0,181);
  const net::Addr destip = net::Addr(172, 16, 189, 1);
  printf("in send data-------------------------------------\n");
  for (size_t i = 0; i < PACKETS_PER_INTERVAL; i++) {
    const char c = 'A' + (i % 26);
    std::string buff(SEND_BUF_LEN, c);
    // client.sendto(inet.gateway(), NCAT_RECEIVE_PORT, buff.data(), buff.size(), send_cb);
    client.sendto(destip, NCAT_RECEIVE_PORT, buff.data(), buff.size(), send_cb);
    printf("send UDP (%c) %d \n", c, i);
  }
}

void udp_test(net::Inet &inet) {
  auto &udp = inet.udp();
  auto &client = udp.bind(15000);
  printf("app send udp data 1 \n");
  send_data(client, inet);
}

void http_test(net::Inet &inet) {
  using namespace http;

  server = std::make_unique<Server>(inet.tcp());

  server->on_request(root_handler);

  server->listen(HTTP_SERVE_PORT);
}

void tcp_test(net::Inet &inet) {
  auto &server = inet.tcp().listen(HTTP_SERVE_PORT);
  printf("Start TCP Listen \n");

  // Add a TCP connection handler - here a hardcoded HTTP-service
  server.on_connect(
      [](net::tcp::Connection_ptr conn) {
        printf("<Service> @on_connect: Connection %s successfully established.\n",
               conn->remote().to_string().c_str());
        // read async with a buffer size of 1024 bytes
        // define what to do when data is read
        conn->on_read(1024,
        [conn](auto buf) {
          printf("<Service> @on_read: %lu bytes received.\n", buf->size());
          try {
            const std::string data((const char*) buf->data(), buf->size());            // try to parse the request
            http::Request req{data};

            // handle the request, getting a matching response
            auto res = handle_request(req);

            printf("<Service> Responding with %u %s.\n",
                    res.status_code(), http::code_description(res.status_code()).data());

            conn->write(res);
          } catch (const std::exception &e) {
            printf("<Service> Unable to parse request:\n%s\n", e.what());
          }
        });
        conn->on_write([](size_t written) {
          printf("<Service> @on_write: %lu bytes written.\n", written);
        });
      });
}
void Service::start() {

  printf("Service started \n");
  
  // Get the first IP stack
  // It should have configuration from config.json
  auto &inet = net::Interfaces::get(0);
  inet.network_config({172, 16, 189, 132}, {255, 255, 255, 0}, {172, 16, 189, 1});

  // udp_test(inet);
  // tcp_test(inet);
  http_test(inet);

  printf("*** Basic demo service started ***\n");
}
