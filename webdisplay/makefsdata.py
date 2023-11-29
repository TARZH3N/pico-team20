#!/usr/bin/python3

# This script is by @rspeir on GitHub: 
# https://github.com/krzmaz/pico-w-webserver-example/pull/1/files/4b3e78351dd236f213da9bebbb20df690d470476#diff-e675c4a367e382db6f9ba61833a58c62029d8c71c3156a9f238b612b69de279d
# Renamed output to avoid linking incorrect file

import os
import binascii

#Create file to write output into
output = open('htmldata.c', 'w') 

#Traverse directory, generate list of files
files = list()
os.chdir('./html_files')
for(dirpath, dirnames, filenames) in os.walk('.'):
    files += [os.path.join(dirpath, file) for file in filenames]

filenames = list()
varnames  = list()

#Generate appropriate HTTP headers
for file in files:

    if '404' in file:
        header = "HTTP/1.0 404 File not found\r\n"
    else:
        header = "HTTP/1.0 200 OK\r\n"

    header += "Server: lwIP/pre-0.6 (http://www.sics.se/~adam/lwip/)\r\n"

    if '.html' in file:
        header += "Content-type: text/html\r\n"
    elif '.shtml' in file:
        header += "Content-type: text/html\r\n"
    elif '.jpg' in file:
        header += "Content-type: image/jpeg\r\n"
    elif '.gif' in file:
        header += "Content-type: image/gif\r\n"
    elif '.png' in file:
        header += "Content-type: image/png\r\n"
    elif '.class' in file:
       header += "Content-type: application/octet-stream\r\n"
    elif '.js' in file:
       header += "Content-type: text/javascript\r\n"
    elif '.css' in file:
       header += "Content-type: text/css\r\n"
    elif '.svg' in file:
       header += "Content-type: image/svg+xml\r\n"
    else:
        header += "Content-type: text/plain\r\n"

    header += "\r\n"

    fvar = file[1:]                 #remove leading dot in filename
    fvar = fvar.replace('/', '_')   #replace *nix path separator with underscore
    fvar = fvar.replace('\\', '_')  #replace DOS path separator with underscore
    fvar = fvar.replace('.', '_')   #replace file extension dot with underscore

    output.write("static const unsigned char data{}[] = {{\n".format(fvar))
    output.write("\t/* {} */\n\t".format(file))

    #first set of hex data encodes the filename
    b = bytes(file[1:].replace('\\', '/'), 'utf-8')     #change DOS path separator to forward slash     
    for byte in binascii.hexlify(b, b' ', 1).split():
        output.write("0x{}, ".format(byte.decode()))
    output.write("0,\n\t")

    #second set of hex data is the HTTP header/mime type we generated above
    b = bytes(header, 'utf-8')
    count = 0
    for byte in binascii.hexlify(b, b' ', 1).split():
        output.write("0x{}, ".format(byte.decode()))
        count = count + 1
        if(count == 10):
            output.write("\n\t")
            count = 0
    output.write("\n\t")

    #finally, dump raw hex data from files
    with open(file, 'rb') as f:
        count = 0
        while(byte := f.read(1)):
            byte = binascii.hexlify(byte)
            output.write("0x{}, ".format(byte.decode()))
            count = count + 1
            if(count == 10):
                output.write("\n\t")
                count = 0
        output.write("};\n\n")

    filenames.append(file[1:])
    varnames.append(fvar)

for i in range(len(filenames)):
    prevfile = "NULL"
    if(i > 0):
        prevfile = "file" + varnames[i-1]

    output.write("const struct fsdata_file file{0}[] = {{{{ {1}, data{2}, ".format(varnames[i], prevfile, varnames[i]))
    output.write("data{} + {}, ".format(varnames[i], len(filenames[i]) + 1))
    output.write("sizeof(data{}) - {}, ".format(varnames[i], len(filenames[i]) + 1))
    output.write("FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT}};\n")

output.write("\n#define FS_ROOT file{}\n".format(varnames[-1])) 
output.write("#define FS_NUMFILES {}\n".format(len(filenames)))

#Uncomment if Code is implemented in C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# // Define the file structure
# struct fsdata_file {
#     const char *name;
#     const unsigned char *data;
#     const int len;
#     int flags;
# };

# // Define constants for file flags
# #define FS_FILE_FLAGS_HEADER_INCLUDED  0x01
# #define FS_FILE_FLAGS_HEADER_PERSISTENT 0x02

# // Function to generate HTTP headers
# const char *generate_http_header(const char *file) {
#     const char *header;
#     if (strstr(file, "404") != NULL) {
#         header = "HTTP/1.0 404 File not found\r\n";
#     } else {
#         header = "HTTP/1.0 200 OK\r\n";
#     }

#     header = strcat(header, "Server: lwIP/pre-0.6 (http://www.sics.se/~adam/lwip/)\r\n");

#     if (strstr(file, ".html") != NULL || strstr(file, ".shtml") != NULL) {
#         header = strcat(header, "Content-type: text/html\r\n");
#     } else if (strstr(file, ".jpg") != NULL) {
#         header = strcat(header, "Content-type: image/jpeg\r\n");
#     } else if (strstr(file, ".gif") != NULL) {
#         header = strcat(header, "Content-type: image/gif\r\n");
#     } else if (strstr(file, ".png") != NULL) {
#         header = strcat(header, "Content-type: image/png\r\n");
#     } else if (strstr(file, ".class") != NULL) {
#         header = strcat(header, "Content-type: application/octet-stream\r\n");
#     } else if (strstr(file, ".js") != NULL) {
#         header = strcat(header, "Content-type: text/javascript\r\n");
#     } else if (strstr(file, ".css") != NULL) {
#         header = strcat(header, "Content-type: text/css\r\n");
#     } else if (strstr(file, ".svg") != NULL) {
#         header = strcat(header, "Content-type: image/svg+xml\r\n");
#     } else {
#         header = strcat(header, "Content-type: text/plain\r\n");
#     }

#     return header;
# }

# int main() {
#     // Create a file to write output into
#     FILE *output = fopen("htmldata.c", "w");

#     // Traverse directory, generate a list of files
#     char *files[1024]; // Assuming a maximum of 1024 files
#     int numFiles = 0;
#     FILE *dir;
#     char command[1024];

#     system("cd ./html_files");
#     dir = popen("find ./ -type f", "r");

#     while (fgets(command, sizeof(command), dir) != NULL) {
#         files[numFiles] = strdup(command);
#         numFiles++;
#     }
#     pclose(dir);

#     char *filenames[1024];
#     char *varnames[1024];

#     // Generate appropriate HTTP headers and C data arrays
#     for (int i = 0; i < numFiles; i++) {
#         const char *file = files[i];
#         const char *header = generate_http_header(file);

#         char fvar[1024];
#         strcpy(fvar, file + 1); // Remove leading dot in filename
#         for (int j = 0; fvar[j]; j++) {
#             if (fvar[j] == '/' || fvar[j] == '\\') {
#                 fvar[j] = '_'; // Replace path separators with underscores
#             }
#             if (fvar[j] == '.') {
#                 fvar[j] = '_'; // Replace file extension dot with underscore
#             }
#         }

#         fprintf(output, "static const unsigned char data%s[] = {\n", fvar);
#         fprintf(output, "\t/* %s */\n\t", file);

#         // First set of hex data encodes the filename
#         int len = strlen(file);
#         for (int j = 0; j < len; j++) {
#             fprintf(output, "0x%02X, ", file[j]);
#         }
#         fprintf(output, "0,\n\t");

#         // Second set of hex data is the HTTP header/mime type we generated above
#         len = strlen(header);
#         int count = 0;
#         for (int j = 0; j < len; j++) {
#             fprintf(output, "0x%02X, ", header[j]);
#             count++;
#             if (count == 10) {
#                 fprintf(output, "\n\t");
#                 count = 0;
#             }
#         }
#         fprintf(output, "\n\t");

#         // Finally, dump raw hex data from files
#         FILE *f = fopen(file, "rb");
#         int byte;
#         count = 0;
#         while ((byte = fgetc(f)) != EOF) {
#             fprintf(output, "0x%02X, ", byte);
#             count++;
#             if (count == 10) {
#                 fprintf(output, "\n\t");
#                 count = 0;
#             }
#         }
#         fprintf(output, "};\n\n");

#         filenames[i] = strdup(file + 1);
#         varnames[i] = strdup(fvar);
#     }

#     for (int i = 0; i < numFiles; i++) {
#         const char *prevfile = "NULL";
#         if (i > 0) {
#             prevfile = varnames[i - 1];
#         }

#         fprintf(output, "const struct fsdata_file file%s[] = {{{ {1}, data%s, ", varnames[i], prevfile);
#         fprintf(output, "data%s + %d, ", varnames[i], strlen(filenames[i]) + 1);
#         fprintf(output, "sizeof(data%s) - %d, ", varnames[i], strlen(filenames[i]) + 1);
#         fprintf(output, "FS_FILE_FLAGS_HEADER_INCLUDED | FS_FILE_FLAGS_HEADER_PERSISTENT}};\n", varnames[i]);
#     }

#     fprintf(output, "\n#define FS_ROOT file%s\n", varnames[numFiles - 1]);
#     fprintf(output, "#define FS_NUMFILES %d\n", numFiles);

#     fclose(output);

#     return 0;
# }
