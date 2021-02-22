#include <cmath> // rand()
#include <sstream>

#include <net/http/request.hpp>
#include <net/http/response.hpp>
#include <net/http/server.hpp>
#include <net/iana.hpp>
#include <net/interfaces>
#include <os>
#include <timers>
#include <memdisk>
#include <hal/machine.hpp>

#include "../bitvisor/bitvisor.hpp"

// #define SEND_BUF_LEN 32768 // 32KB
#define SEND_BUF_LEN 1024
// #define SEND_BUF_LEN 100
#define SERVER_TEST_LEN 10
#define PACKETS_PER_INTERVAL 10
#define NCAT_RECEIVE_PORT 12000
#define HTTP_SERVE_PORT 80

uint64_t data_len{0};
std::unique_ptr<http::Server> server;

using namespace std::chrono;

struct file {
  using Buf = net::tcp::buffer_t;
  using Vec = std::vector<Buf>;

  auto begin() { return chunks.begin(); }
  auto end()   { return chunks.end();   }

  size_t size(){ return sz; }
  size_t blkcount() { return chunks.size(); }

  void append(Buf& b) {
    chunks.push_back(b);
    sz += b->size();
  }

  void reset() {
    chunks.clear();
    sz = 0;
  }

  Vec chunks{};
  size_t sz{};
};
file filerino;

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
         << "Demo Page<span style='font-weight: lighter'>1</span></h1>"
         << "<h2>The C++ Unikernel</h2>"
         << "<p>You have successfully booted an IncludeOS TCP service with simple http. "
         << "For a more sophisticated example, take a look at "
         << "<a href='https://github.com/hioa-cs/IncludeOS/tree/master/examples/acorn'>Acorn</a>.</p>"
         << "<footer><hr/>&copy; 2017 IncludeOS </footer></body></html>";

  return stream.str();
}

void root_handler(http::Request_ptr req, http::Response_writer_ptr writer) {
  // printf("[WebServer]handle request=======\n");
  writer->header().set_field(http::header::Server, "IncludeOS/0.10");

  // GET /
  if (req->method() == http::GET && req->uri().to_string() == "/") {
    writer->write(HTML_RESPONSE());
    printf("[WebServer]GET request (path=/)\n");
    writer->header().set_field(http::header::Content_Type, "text/html; charset=UTF-8");
  } else if (req->method() == http::GET && req->uri().to_string() == "/test1") {
    // GET /test1 file
    auto& filesys = fs::memdisk().fs();
    auto buf = filesys.read_file("/static/test1.html");
    writer->write(buf.to_string());
    printf("[WebServer]GET request (path=/test1)\n");
    writer->header().set_field(http::header::Content_Type, "text/html; charset=UTF-8");
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

void send_udp_data(net::udp::Socket &client, net::Inet &inet, int bufflen, int interval) {
  const net::Addr destip = net::Addr(192,168,0,181);
  // const net::Addr destip = net::Addr(172, 16, 189, 1);
  unsigned int start_time= RTC::nanos_now();
  for (size_t i = 0; i < interval; i++) {
    const char c = 'A' + (i % 26);
    std::string buff(bufflen, c);
    // client.sendto(inet.gateway(), NCAT_RECEIVE_PORT, buff.data(), buff.size(), send_cb);
    client.sendto(destip, NCAT_RECEIVE_PORT, buff.data(), buff.size(), send_cb);
  }
  printf("======================================\n");
  printf("send udp data finished at %luus()\n", (RTC::nanos_now() - start_time) / 1000);
}

void udp_client_test(net::Inet &inet) {
  auto &udp = inet.udp();
  auto &client = udp.bind(16000);
  // 32KB
  printf("32KB======================================\n");
  send_udp_data(client, inet, 1024, 32);
  // // 512KB
  printf("512KB======================================\n");
  send_udp_data(client, inet, 1024, 512);
  // 1MB
  printf("1MB======================================\n");
  send_udp_data(client, inet, 1024, 1024);
}

static bool udp_server_test_first = true;
static unsigned long udp_server_test_start_time = 0;
static unsigned long udp_server_recv_size = 0;

void udp_server_test(net::Inet &inet, int recv_size) {
  auto &udp = inet.udp();
  auto &server = udp.bind(15001);

  printf("Running as Server. Waiting for data...\n");
  server.on_read(
      [recv_size]([[maybe_unused]] auto addr,
         [[maybe_unused]] auto port,
         const char *buf, int len) {
        auto data = std::string(buf, len);
        udp_server_recv_size += data.size();
        // printf("Received data len=%d\n", len);
        if (udp_server_test_first) {
          udp_server_test_start_time = RTC::nanos_now();
          // printf("start at %lu\n", udp_server_test_start_time);
          // printf("======================================\n");
          udp_server_test_first = false;
        }
        if(udp_server_recv_size >= recv_size - 10) { // not worked...
          printf("======================================\n");
          printf("finish time : %luus\n",(RTC::nanos_now() - udp_server_test_start_time) / 1000);
        }
        printf("time: %lu\n", (RTC::nanos_now() - udp_server_test_start_time) / 1000);
        printf("======================================\n");
      });
}

void http_test(net::Inet &inet) {
  using namespace http;

  printf("[WebServer]Starting server\n");
  server = std::make_unique<Server>(inet.tcp());
  server->on_request(root_handler);
  server->listen(HTTP_SERVE_PORT);
}

// static const uint32_t  SIZE = 1024*32;
bool      timestamps{true};
bool      SACK{true};
std::chrono::milliseconds dack{40};
uint64_t  received{0};
uint32_t  winsize{8192};
uint8_t   wscale{5};
uint16_t port_send {9000};
uint16_t port_recv {9001};
bool      keep_last = false;
net::tcp::buffer_t blob = nullptr;

void recv(size_t len)
{
  received += len;
}

static unsigned long start_time = 0;

void tcp_test(net::Inet &inet, const uint32_t file_size) {
  using namespace net::tcp;

  printf("start tcp test \n");

  blob = net::tcp::construct_buffer(file_size, '!');
  auto& tcp = inet.tcp();
  tcp.set_DACK(dack); // default
  tcp.set_MSL(std::chrono::seconds(3));

  tcp.set_window_size(winsize, wscale);
  tcp.set_timestamps(timestamps);
  tcp.set_SACK(SACK);

  tcp.listen(port_send).on_connect([file_size](Connection_ptr conn)
  {
    printf("%s connected. Sending file %u KB\n", conn->remote().to_string().c_str(), file_size/(1024));
    start_time = RTC::nanos_now();

    conn->on_disconnect([] (Connection_ptr self, Connection::Disconnect)
    {
      if(!self->is_closing())
        self->close();
      printf("tcp send result time=%d[ms] \n", (RTC::nanos_now()-start_time) / 1000);
      printf("======================================\n");
    });
    conn->on_write([](size_t n)
    {
      recv(n);
      printf("tcp send write result time=%d[ms] \n", (RTC::nanos_now()-start_time)/1000);
      printf("======================================\n");
    });

    if (! keep_last) {
      conn->write(blob);
    } else {
      for (auto b : filerino)
        conn->write(b);
    }
    conn->close();
  });

  tcp.listen(port_recv).on_connect([file_size](Connection_ptr conn)
  {
    printf("%s connected. \n", conn->remote().to_string().c_str());
    filerino.reset();
    start_time = RTC::nanos_now();

    conn->on_close([]
    {

    });
    conn->on_disconnect([] (net::tcp::Connection_ptr self,
                            net::tcp::Connection::Disconnect reason)
    {
      (void) reason;
      // if(const auto bytes_sacked = self->bytes_sacked(); bytes_sacked)
        // printf("SACK: %zu bytes (%zu kB)\n", bytes_sacked, bytes_sacked/(1024));
      printf("tcp recv result time=%d[ms] \n", (RTC::nanos_now()-start_time)/1000);
      printf("======================================\n");

      if(!self->is_closing())
        self->close();
    });
    conn->on_read(file_size, [] (buffer_t buf)
    {
      recv(buf->size());
      if (UNLIKELY(keep_last)) {
        filerino.append(buf);
      }
      // printf("tcp recv read time=%d[ms] \n", (RTC::nanos_now()-start_time)/1000);
      // printf("======================================\n");
    });
  });
}

void Service::start() {
  // printf("Booted at monotonic_us=%ld \n", RTC::nanos_now());

  fs::memdisk().init_fs(
      [](auto err, auto &) {
        assert(!err);
      });

  auto &inet = net::Interfaces::get(0);
  // inet.network_config({172, 16, 189, 132}, {255, 255, 255, 0}, {172, 16, 189, 1});
  inet.network_config({192, 168, 0, 196}, {255, 255, 255, 0}, {192, 168, 0, 1});

  udp_server_test(inet, 1024 * 1024);
  // udp_client_test(inet);
  // tcp_test(inet, 1024 * 1024);
  // http_test(inet);

  printf("*** service started ***\n");
}
