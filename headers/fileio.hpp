//#pragma once
#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>

using namespace std;

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input);

//Reads given file into a string and returns it
string read_file(char *file);

// removes all the temporarily created files
void removeTempFiles();

#endif