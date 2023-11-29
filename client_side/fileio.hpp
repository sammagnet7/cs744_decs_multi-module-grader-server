//#pragma once
#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

using namespace std;

//<<<<<<<<<<<<<<<============== Utility methods declarations below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// Reads given file into a string and returns it
string read_file(string file);

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input);

// removes all the temporarily created files
void removeTempFiles(vector<string> files_to_remove);

//Logs any passed message to log file
void logMessageToFile(const std::string& message);

#endif