#ifndef __MY_UTIL__
#define __MY_UTIL__

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <experimental/filesystem>
#include <time.h>
#include <sys/stat.h>
#include <jsoncpp/json/json.h>
#include "bundle.h"

namespace fs = std::experimental::filesystem;

namespace cloud{

    class FileUtil{
        private:
            std::string _name;
        public:
            FileUtil(const std::string &name): _name(name){}

            bool Exist()  //whether the file exists.
            {
                return fs::exists(_name);
            }
            std::string Name()
            {
                return fs::path(_name).filename().string(); // return a pure file name
            }
            size_t Size()  //file size.
            {
                if(this->Exist() == false)
                {
                    return 0;
                }
                return fs::file_size(_name);

            }
            time_t MTime()  //last modification time.
            {
                if(this->Exist() == false)
                {
                    return 0;
                }
                auto ftime = fs::last_write_time(_name);
                // std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);
                std::time_t cftime = decltype(ftime)::clock::to_time_t(ftime);
                return cftime;
            }
            time_t ATime()  // last access time.
            {
                if(this->Exist() == false)
                {
                    return 0;
                }
                struct stat st;
                stat(_name.c_str(), &st);
                return st.st_atime;
            }
            bool Read(std::string *body)  // read the file data into the body.
            {
                if(this->Exist() == false)
                {
                    return false;
                }
                std::ifstream ifs;
                ifs.open(_name, std::ios::binary);
                if(ifs.is_open() == false)
                {
                    std::cout<<"read open failed\n";
                    return false;
                }
                size_t fsize=this->Size();
                body->resize(fsize);
                ifs.read(&(*body)[0], fsize);
                if(ifs.good() == false)
                {
                    std::cout<<"read file failed\n";
                    ifs.close();
                    return false;
                }
                ifs.close();
                return true;
            }
            bool Write(const std::string &body)  //write the data in the body to a file.
            {
                std::ofstream ofs;
                ofs.open(_name, std::ios::binary);
                if(ofs.is_open() == false)
                {
                    std::cout<<"write open failed\n";
                    return false;
                }
                ofs.write(body.c_str(), body.size());
                if(ofs.good() == false)
                {
                    std::cout<<"write file failed\n";
                    ofs.close();
                    return false;
                }
                ofs.close();
                return true;

            }

            bool CreateDirectory() // create directory.
            {
                if(this->Exist())
                {
                    return true;
                }
                fs::create_directories(_name);
                return true;
            }
            bool ScanDirectory(std::vector<std::string> *arry) // scan directory.
            {
                if(this->Exist() == false)
                {
                    return false;
                }
                for(auto &a : fs::directory_iterator(_name))
                {
                    if(fs::is_directory(a) == true)
                    {
                        continue;
                    }
                    //std::string pathname = fs::path(a).filename().string(); //pure filename
                    std::string pathname = fs::path(a).relative_path().string();//filename with path
                    arry->push_back(pathname);
                }
                return true;


            }

            bool Remove()  //
            {
                if(this->Exist() == false)
                {
                    return true;
                }
                fs::remove_all(_name);
                return true;

            }

            bool Compress(const std::string &packname)
            {
                if(this->Exist() == false)
                {
                    return false;
                }
                std::string body;
                if(this->Read(&body) == false)
                {
                    std::cout<< "compress read file failed\n";
                    return false;
                }
                std::string packed = bundle::pack(bundle::LZIP, body);
                if(FileUtil(packname).Write(packed) == false)
                {
                    std::cout<< "compress write pack data failed\n";
                    return false;
                }
                fs::remove_all(_name); //delete the source file  after compression
                return true;

            }

            bool Uncompress(const std::string &filename)
            {
                if(this->Exist() == false)
                {
                    return false;
                }
                std::string body;
                if(this->Read(&body) == false)
                {
                    std::cout<< "uncompress read pack data failed\n";
                    return false;
                }
                std::string unpack_data = bundle::unpack(body);
                if(FileUtil(filename).Write(unpack_data) == false)
                {
                    std::cout<< "uncompress write file data failed\n";
                    return false;
                }
                fs::remove_all(_name); //decompress and delete the package
                return true;
            }
    };

   class JsonUtil{
        public:
            static bool Serialize(const Json::Value &val, std::string *body)
            {
                Json::StreamWriterBuilder swb;
                Json::StreamWriter *sw = swb.newStreamWriter();
                std::stringstream ss;
                int ret = sw->write(val, &ss);
                if(ret != 0)
                {
                    std::cout<< "serialize failed\n";
                    delete sw;
                    return false;
                }
                *body = ss.str();
                delete sw;
                return true;
            }
            static bool Unserialize(const std::string &body, Json::Value *val)
            {
                Json::CharReaderBuilder crb;
                Json::CharReader *cr = crb.newCharReader();
                std::string err;
                bool ret = cr->parse(body.c_str(), body.c_str() + body.size(), val, &err);
                if(ret == false)
                {
                    std::cout<< "Unserialiaze failed\n";
                    delete cr;
                    return false;
                }
                delete cr;
                return true;

            }
     };


}



#endif 
