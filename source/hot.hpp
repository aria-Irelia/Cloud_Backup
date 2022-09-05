#ifndef __HOT__
#define __HOT__

#include "data.hpp"
#include <unistd.h>

extern cloud::DataManager *_data;

namespace cloud{

    class HotManager{
        private:
            time_t  _hot_time = 30;    
            std::string _backup_dir = "./backup_dir/";
        
        public:
            HotManager()
            {
                FileUtil(_backup_dir).CreateDirectory();
            }

            bool IsHot(const std::string &filename)
            {
                time_t atime = FileUtil(filename).ATime();
                time_t ctime = time(NULL);
                if((ctime - atime ) > _hot_time )
                {
                    return false;
                }
                return true;
            }

            bool RunModule()
            {
                while(1)
                {
                    //directory traversal
                    std::vector<std::string> arry;
                    FileUtil(_backup_dir).ScanDirectory(&arry);
                    
                    for(auto &file : arry)
                    {
                        //check whether the file is a hotspot file
                        if(IsHot(file) == true)
                        {
                            continue;
                        }

                        //obtain historical backup information
                        FileInfo info;
                        bool ret = _data->SelectOneByrealpath(file, &info);
                        if (ret == false)
                        {
                            //files detected, but no backup information,it may be an incomplete file.
                            std::cout<< "an exception file is detected. delete it\n";
                            FileUtil(file).Remove();
                            continue;

                            //_data->Insert(file);
                            //_data->SelectOneByrealpath(file, &info);
                            
                        }

                        //compress non-hot files
                        FileUtil(file).Compress(info.pack_path);

                        //modifying backup information
                        _data->UpdataStatus(file, true);

                        std::cout << info.real_path << "-----compress-------" << info.pack_path << std::endl;
                    }
                    usleep(1000);

                }

                return true;
            }
    };
}


#endif
