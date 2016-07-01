//expand_db.cpp
//2016-06-03
//Ben Bettisworth
//program to splay out a superstar database file into a directory structure
//or take a directory structure and make it into a database file

#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
#include <string>
using std::string;
#include <stdexcept>
using std::runtime_error;
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fts.h>

#define FILEIO_CHECK(code, string) {if(!code){ throw runtime_error(string);}}

struct data_struct_t{
    uint64_t data_size;
    std::string data_value;
};

//code stolen from stackoverflow to make directories recursivly
//https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
int _mkdir(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p; 

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return -1; 
    }   
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1; 
            }

            *p = '/';
        }
    }   

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1; 
    }   

    return 0;
}

void read(const std::string& filename){
    ifstream db_file(filename.c_str());
    uint64_t db_size = 0;
    db_file.read((char*)&db_size, sizeof(db_size));
    for(size_t i = 0;i<db_size;++i){
        data_struct_t key_struct;
        FILEIO_CHECK(db_file.read((char*)&key_struct.data_size, sizeof(key_struct.data_size)),
                "failed to read size of key");

        key_struct.data_value.resize(key_struct.data_size);
        FILEIO_CHECK(db_file.read(&key_struct.data_value[0],key_struct.data_size),
                "failed to read a key");

        data_struct_t value_struct;
        FILEIO_CHECK(db_file.read((char*)&value_struct.data_size, sizeof(value_struct.data_size)),
                "failed to read size of value");

        value_struct.data_value.resize(value_struct.data_size);
        FILEIO_CHECK(db_file.read(&value_struct.data_value[0],value_struct.data_size),
                "failed to read value");
        if(key_struct.data_value.size()>0)//Account for Empty Things...
        {
            int status = _mkdir((key_struct.data_value).c_str());   
            if(status){
                throw runtime_error("error creating directory \'" + key_struct.data_value
                        + "\'");
            }
            ofstream value_file((key_struct.data_value + "value").c_str());
            value_file<<value_struct.data_value;
            value_file.close();
        }
    }
}

void write(string root_dirname, const string& db_filename){
    char *tmp_path[] = {&root_dirname[0], 0};
    FTS *tree = fts_open(tmp_path, FTS_NOCHDIR, 0);

    ofstream db_file(db_filename.c_str(), std::ios::binary);
    uint64_t db_entries =0;
    db_file.write((char*)&db_entries, sizeof(db_entries));
    
    FTSENT *node = 0;
    while ( (node = fts_read(tree)) ){
        if(!strcmp(node->fts_name,"value")){
            db_entries++;
            ifstream value_file(node->fts_path);
            std::ostringstream file_data;
            file_data << value_file.rdbuf();
            value_file.close();

            data_struct_t key;
            key.data_value = node->fts_path;
            key.data_value.erase(key.data_value.find("value"));
            key.data_size = key.data_value.size();
            FILEIO_CHECK(db_file.write((char*)&key.data_size, sizeof(key.data_size)),
                    "failed to write size of key");
            FILEIO_CHECK(db_file.write(key.data_value.c_str(), key.data_size),
                    "failed to write key");
            
            data_struct_t value;
            value.data_value = file_data.str();
            value.data_size = value.data_value.size();
            FILEIO_CHECK(db_file.write((char*)&value.data_size, sizeof(value.data_size)),
                    "failed to write size of value");
            FILEIO_CHECK(db_file.write(value.data_value.c_str(), value.data_size),
                    "failed to write value");
        }
            
    }
    db_file.seekp(0);
    db_file.write((char*)&db_entries, sizeof(db_entries));
}

int main(int argc, char** argv){
    bool read_flag=false;
    std::string read_file;
    bool write_flag=false;
    std::string write_file;
    bool output_flag = false;
    std::string output_file;

    if(argc == 1){
        std::cerr<<"Not enough arguments\n";
        return 1;
    }

    for(int i=1;i<argc;++i){
        if(!strcmp("-r",argv[i])){
            read_flag = true;
            if(i+1 == argc){
                std::cerr<<"not enough arguments\n";
                return 1;
            }
            read_file = argv[i+1];
        }
        else if(!strcmp("-w",argv[i])){
            write_flag = true;
            if(i+1 == argc){
                std::cerr<<"not enough arguments\n";
                return 1;
            }
            write_file = argv[i+1];
        }
        else if(!strcmp("-o", argv[i])){
            output_flag = true;
            if(i+1 == argc){
                std::cerr<<"not enough arguments\n";
                return 1;
            }
            output_file = argv[i+1];
        }
    }
    
    if(read_flag && write_flag){
        std::cerr<<"Can only read or write, not both\n";
        return 1;
    }
    if(write_flag && !output_flag){
        std::cerr<<"No ouput specified, aborting\n";
        return 1;
    }

    if(read_flag) read(read_file);
    else write(write_file, output_file);


    return 0;
}
