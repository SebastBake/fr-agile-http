# fr-agile-http
A fragile http server written in C

## Features
 - No thread limit causes kamikaze-like behavior with many concurrent connections
 - OS cleans up memory leaks due to detatched threads
 - Response header length over 256 bytes causes buffer overflow
 - URL length over 256 bytes causes buffer overflow
 - Supports 5 mimetypes with support for both .jpg and .jpeg extensions
 - Very efficient request parsing (only parses request line)
 - Partially sends large files
