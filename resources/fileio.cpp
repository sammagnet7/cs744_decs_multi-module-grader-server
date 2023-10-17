#include "../headers/fileio.hpp"

using namespace std;

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input)
{
    ofstream received_file(filename);
    received_file.write(input.c_str(), input.length());
    received_file.close();
}

//Reads given file into a string and returns it
string read_file(char *file)
{
    ifstream filestream(file, ios::binary);
    string filedata((istreambuf_iterator<char>(filestream)),
                       istreambuf_iterator<char>());
    string txt, txt_accumulator;
    // while (getline(filestream, txt))
    // {
    //     txt_accumulator += (txt + "\n");
    // }
    filestream.close();
    return filedata;
}

//Wrapper function
void _remove(string file)
{
    try
    {
        remove(file.c_str());
    }
    catch (...)
    {
    }
}

// removes all the temporarily created files
void removeTempFiles()
{
    _remove("compiler_output.txt");
    _remove("prog_output.txt");
    _remove("actual_file.txt");
    _remove("expected_file.txt");
    _remove("diffcheck.txt");
    _remove("received.*");
    _remove("received");
}