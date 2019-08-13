/* Defines the location of the webservers files. This should be set to equal the
   directory where you want the server to serve files from. */
#define WEBSITE_DIR "path/to/website/folder"

/* The port the server will listen on. */
#define PORT 8081

/* The number of connections the server will have on hold until a listener picks
   it up */
#define CONNECTION_BACKLOG_SIZE 256

/* The size of the buffer used for loading the requested file into memory before
   sending it to the user and for reading the GET request of the user. */
#define BUFFER_SIZE 4096

/* This defines the header and the html that is sent back requests that 404 */
#define HTTP_404 \
"HTTP/1.0 404 Not Found\r\n"\
"Content-Type: text/html\r\n"\
"Content-Length: 219\r\n"\
"\r\n"\
"<!doctype html>\n"\
"<html lang='en'>\n"\
"  <head>\n"\
"    <meta charset='utf-8'>\n"\
"    <title>File Not Found</title>\n"\
"  </head>\n"\
"  <body>\n"\
"    <h1>Error: File Not Found!</h1>\n"\
"    Could not find the file you requested!\n"\
"  </body>\n"\
"</html>\n"

/* This defines all of the supported file types and their related MIME type.
   Types can be added as need but this are consider the basic web extensions */
static struct {
  const char* ext;
  const char* mime;
} extensions[] = {
  {".html", "text/html"},
  {".js",   "text/javascript"},
  {".css",  "text/css"},
  {0, 0}
};
