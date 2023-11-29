#include "fileio.hpp"


//Reads given file into a string and returns it
string read_file(string file)
{
    ifstream filestream(file.c_str(), ios::binary);
    string filedata((istreambuf_iterator<char>(filestream)),
                       istreambuf_iterator<char>());
    /*
    string txt, txt_accumulator;
    while (getline(filestream, txt))
    {
        txt_accumulator += (txt + "\n");
    }
    */
    filestream.close();
    return filedata;
}

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input)
{
    ofstream new_file(filename);
    new_file.write(input.c_str(), input.length());
    new_file.close();
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
        cout<<"error in remove";
    }
}

// removes all the temporarily created files
void removeTempFiles(vector<string> files_to_remove)
{
    for(string file : files_to_remove){
        _remove(file);
    }
}

void logMessageToFile(const std::string& message) {

    std::string directoryPath = "temp_files";
    std::string filePath = "temp_files/overall.log";

    // Check if the directory exists or create it if it doesn't
    if (!std::filesystem::exists(directoryPath))
    {
        std::filesystem::create_directory(directoryPath);
    }

    std::ofstream outputFile(filePath, std::ios::app); // 'app' for append mode
    if (outputFile.is_open()) {
        outputFile << message << std::endl;
        outputFile.close();
    } else {
        std::cerr << "Error opening file: " << filePath << std::endl;
    }
}
