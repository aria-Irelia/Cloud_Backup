//#include "util.hpp"
//#include "data.hpp"
#include "hot.hpp"
#include "server.hpp"
#include <thread>

void FileUtiltest()
{
    //cloud::FileUtil("./testdir/adir").CreateDirectory();
    //cloud::FileUtil("./testdir/a.txt").Write("hello bit\n");


    std::string body;
    cloud::FileUtil("./testdir/a.txt").Read(&body);

    std::cout<< body <<std::endl;

    std::cout<< cloud::FileUtil("./testdir/a.txt").Size() << std::endl;
    std::cout<< cloud::FileUtil("./testdir/a.txt").MTime() << std::endl;
    std::cout<< cloud::FileUtil("./testdir/a.txt").ATime() << std::endl;

    std::vector<std::string> arry;
    cloud::FileUtil("./testdir").ScanDirectory(&arry);
    for(auto &a: arry)
    {
        std::cout<< a << std::endl;
    }
}

void Jsontest()
{
    Json::Value val;
    val["name"] = "xiaoming";
    val["gender"] = "male";
    val["age"] = 18;
    val["score"].append(77.5);
    val["score"].append(78.5);
    val["score"].append(79.5);

    std::string body;
    cloud::JsonUtil::Serialize(val, &body);
    std::cout<< body << std::endl;

    Json::Value root;
    cloud::JsonUtil::Unserialize(body, &root);
    std::cout<< root["name"].asString() <<std::endl;
    std::cout<< root["gender"].asString() <<std::endl;
    std::cout<< root["age"].asInt() <<std::endl;
    std::cout<< root["score"][0].asFloat() <<std::endl;
    std::cout<< root["score"][1].asFloat() <<std::endl;
    std::cout<< root["score"][2].asFloat() <<std::endl;

}

void compresstest()
{
    cloud::FileUtil("./hello.txt").Compress("hello.zip");
    cloud::FileUtil("./hello.zip").Uncompress("bit.txt");

}

void datatest()
{
    cloud::DataManager data;

    //data.Delete("/download/研究生离校申请书.docx");
    //data.Delete("/download/个人简历.pdf");
    //data.Delete("/download/leetcode note.txt");
    /*
    std::vector<cloud::FileInfo> arry;
    data.SelectAll(&arry);
    for(auto &e : arry)
    {
        std::cout<< e.filename <<std::endl;
        std::cout<< e.url_path <<std::endl;
        std::cout<< e.real_path <<std::endl;
        std::cout<< e.file_size <<std::endl;
        std::cout<< e.backup_time <<std::endl;
        std::cout<< e.pack_flag <<std::endl;
        std::cout<< e.pack_path <<std::endl;
    }
    */

    
    data.Insert("./backup_dir/hello.txt");
    data.UpdataStatus("./backup_dir/hello.txt", true);
    std::vector<cloud::FileInfo> arry;
    //arry.clear();
    data.SelectAll(&arry);
    for(auto &e : arry)
    {
        std::cout<< e.filename <<std::endl;
        std::cout<< e.url_path <<std::endl;
        std::cout<< e.real_path <<std::endl;
        std::cout<< e.file_size <<std::endl;
        std::cout<< e.backup_time <<std::endl;
        std::cout<< e.pack_flag <<std::endl;
        std::cout<< e.pack_path <<std::endl;
    }

    std::cout<< "------------------------------\n";

    data.UpdataStatus("./backup_dir/hello.txt", false);
    
    cloud::FileInfo e;
    data.SelectOne("/download/hello.txt", &e);

    std::cout<< e.filename <<std::endl;
    std::cout<< e.url_path <<std::endl;
    std::cout<< e.real_path <<std::endl;
    std::cout<< e.file_size <<std::endl;
    std::cout<< e.backup_time <<std::endl;
    std::cout<< e.pack_flag <<std::endl;
    std::cout<< e.pack_path <<std::endl;
    
    std::cout<< "------------delete---------\n";
    

    /*
    data.Delete("/download/hello.txt");
    arry.clear();
    data.SelectAll(&arry);
    for(auto &e : arry)
    {
        std::cout<< e.filename <<std::endl;
        std::cout<< e.url_path <<std::endl;
        std::cout<< e.real_path <<std::endl;
        std::cout<< e.file_size <<std::endl;
        std::cout<< e.backup_time <<std::endl;
        std::cout<< e.pack_flag <<std::endl;
        std::cout<< e.pack_path <<std::endl;
    }
    */

}

cloud::DataManager *_data;

void hottest()
{
    _data = new cloud::DataManager();
    cloud::HotManager cloud;
    cloud.RunModule();
}

void servertest()
{
    cloud::Server srv;
    srv.RunModule();
}

int main()
{
    _data = new cloud::DataManager();
    //FileUtiltest();
    //Jsontest();
    //compresstest();
    //datatest();
    //hottest();
    //servertest();

    std::thread hot_thread(hottest);
    std::thread srv_thread(servertest);

    hot_thread.join();
    srv_thread.join();
    return 0;
}
