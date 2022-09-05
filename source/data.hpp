#ifndef __MY_DATA__
#define __MY_DATA__

#include "util.hpp"
#include <unordered_map>

namespace cloud{
    typedef struct _FileInfo{

        std::string filename;
        std::string url_path;
        std::string real_path;
        size_t file_size;
        time_t backup_time;
        bool pack_flag;
        std::string pack_path;
    }FileInfo;


    class DataManager{
        private:
            std::string _back_dir = "./backup_dir/";
            std::string _pack_dir = "./pack_dir/";
            std::string _download_prefix = "/download/";
            std::string _pack_subfix = ".zip";
            
            std::string _back_info_file = "./backup.dat"; 
            std::unordered_map<std::string, FileInfo> _back_info; // _map<url_path, FileInfo>;

        public:
            DataManager()
            {
                FileUtil(_back_dir).CreateDirectory();
                FileUtil(_pack_dir).CreateDirectory();
                if(FileUtil(_back_info_file).Exist())
                {
                    Initload();
                }
            }
            
            bool Storage()
            {
                Json::Value infos;
                std::vector<FileInfo> arry;
                this->SelectAll(&arry);

                for(auto &a : arry)
                {
                    Json::Value info;
                    info["filename"] = a.filename;
                    info["url_path"] = a.url_path;
                    info["real_path"] = a.real_path;
                    info["file_size"] = (Json::UInt64)a.file_size;
                    info["backup_time"] = (Json::UInt64)a.backup_time;
                    info["pack_flag"] = a.pack_flag;
                    info["pack_path"] = a.pack_path;
                    infos.append(info);
                }

                std::string body;
                JsonUtil::Serialize(infos, &body);
                FileUtil(_back_info_file).Write(body);
                return true;
            }

            
            bool Initload()
            {
                std::string body;
                bool ret = FileUtil(_back_info_file).Read(&body);
                if(ret == false)
                {
                    std::cout<< "initload back_info_file failed\n";
                    return false;
                }
                Json::Value infos;
                ret = JsonUtil::Unserialize(body, &infos);
                if(ret == false)
                {
                    std::cout<< "initload Unserialize failed\n";
                    return false;
                }
                int sz = infos.size();
                for(int i=0; i<sz; ++i)
                {
                    FileInfo info;
                    info.filename = infos[i]["filename"].asString();
                    info.url_path = infos[i]["url_path"].asString();
                    info.real_path = infos[i]["real_path"].asString();
                    info.file_size = infos[i]["file_size"].asInt64();
                    info.backup_time = infos[i]["backup_time"].asInt64();
                    info.pack_flag = infos[i]["pack_flag"].asBool();
                    info.pack_path = infos[i]["pack_path"].asString();
                    _back_info[info.url_path] = info;
                }
                return true;

            }
            

            //insert
            bool Insert(const std::string &pathname)
            {
                if(FileUtil(pathname).Exist() == false)
                {
                    std::cout<< "insert file not exist\n";
                    return false;
                }

                //such as : pathname = ./backup_dir/a.txt 
                FileInfo info;
                info.filename = FileUtil(pathname).Name(); // a.txt
                info.url_path = _download_prefix + info.filename; // ./download/a.txt
                info.real_path = pathname;
                info.file_size = FileUtil(pathname).Size();
                info.backup_time = FileUtil(pathname).MTime();
                info.pack_flag = false; // just upload in uncompressed state 
                info.pack_path = _pack_dir + info.filename + _pack_subfix; // ./pack_dir/a.txt.zip

                _back_info[info.url_path] = info; // add to map
                Storage();
                return true;
            }

            bool UpdataStatus(const std::string &pathname, bool status)
            {
                std::string url_path = _download_prefix + FileUtil(pathname).Name();
                auto it = _back_info.find(url_path);
                if(it == _back_info.end())
                {
                    std::cout<< "file info is not exist\n";
                    return false;
                }
             
                it->second.pack_flag = status;
                Storage();
                return true;
            }

            bool SelectAll(std::vector<FileInfo> *infos)
            {
                for(auto it = _back_info.begin(); it != _back_info.end(); ++it)
                {
                    infos->push_back(it->second);
                }
                return true;
            }

            bool SelectOne(const std::string &url_path, FileInfo *info)
            {
                auto it = _back_info.find(url_path);
                if(it == _back_info.end())
                {
                    std::cout<< "file info is not exist\n";
                    return false;
                }
                *info = it->second;
                return true;
            }

            bool SelectOneByrealpath(const std::string &realpath, FileInfo *info)
            {
                for(auto it = _back_info.begin(); it != _back_info.end(); ++it)
                {
                    if(it->second.real_path == realpath)
                    {
                        *info = it->second;
                        return true;
                    }
                }
                return false;

            }

            bool Delete(const std::string &url_path)
            {
                auto it = _back_info.find(url_path);
                if(it == _back_info.end())
                {
                    std::cout<< "file info is not exist\n";
                    return false;
                }
                _back_info.erase(it);
                Storage();
                return true;
            }


    };

}


#endif
