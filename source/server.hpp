#ifndef __MY_SERVER__
#define __MY_SERVER__

#include "data.hpp"
#include "httplib.h"

extern cloud::DataManager *_data;

namespace cloud{

    class Server{

        private:
            int _srv_port = 9090;
            std::string _url_prefix = "/download/";
            std::string _backup_dir = "./backup_dir/";
            httplib::Server _srv;
        private:
            static void Upload(const httplib::Request &req, httplib::Response &rsp)
            {
                std::string _backup_dir = "./backup_dir/";

                if(req.has_file("file") == false)
                {
                    std::cout<< "upload file data format error\n";
                    rsp.status = 400;
                    return ;
                }

                httplib::MultipartFormData data = req.get_file_value("file");

                std::string realpath = _backup_dir + data.filename;

                if(FileUtil(realpath).Write(data.content) == false)
                {
                    std::cout<< "back file failed\n";
                    rsp.status = 500;
                    return ;
                }

                if(_data->Insert(realpath) == false)
                {
                    std::cout<< "insert info failed\n";
                    rsp.status = 500;
                    return ;
                }

                rsp.status = 200;
                std::cout << "new backup file : " << realpath << std::endl;
                return ;
            }

            static std::string  Strtime(time_t t)
            {
                return std::asctime(std::localtime(&t));
            }

            static void List(const httplib::Request &req, httplib::Response &rsp)
            {
                //get the file history backup information and organize an HTML page as the response body;
                std::vector<FileInfo> arry;
                if(_data->SelectAll(&arry) == false)
                {
                    std::cout<< "selectall back_info failed\n";
                    rsp.status = 500;
                    return ;
                }
                std::stringstream ss;
                ss << "<html>";
                ss <<   "<head>";
                ss <<       "<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
                ss <<       "<title>Download</title>";
                ss <<   "</head>";
                ss <<   "<body>";
                ss <<       "<h1>Download</h1>";
                ss <<       "<table>";
                for(auto &a: arry)
                {
                    // information for each line of the page;
                    ss <<       "<tr>";
                    ss <<           "<td> <a href='" << a.url_path << "'>" << a.filename << "</a></td>";
                    ss <<           "<td align='right'>" << Strtime(a.backup_time) << "</td>";
                    ss <<           "<td align='right'>" << a.file_size / 1024 << "k </td>";
                    ss <<       "</tr>";
                }
                ss <<       "</table>";
                ss <<   "</body>";
                ss << "</html>";

                rsp.set_content(ss.str(), "text/html");
                rsp.status = 200;

            }

            static std::string StrEtag(const std::string &filename)
            {
                // an Etag is a unique identifier for a file;
                time_t mtime = FileUtil(filename).MTime();
                size_t fsize = FileUtil(filename).Size();

                std::stringstream ss;
                ss << fsize << "-" << mtime;
                return ss.str();
            }

            static void Download(const httplib::Request &req, httplib::Response &rsp)
            {
                FileInfo info;
                if(_data->SelectOne(req.path, &info) == false)
                {
                    std::cout<< "selectone back_info failed\n";
                    rsp.status = 404;
                    return ;
                }

                if(info.pack_flag == true)
                {
                    FileUtil(info.pack_path).Uncompress(info.real_path);
                }

                if(req.has_header("If-Range"))
                {
                    //If-Range: Rtag
                    //Range = std::pair<ssize_t, ssize_t>;
                    //Ranges = std::vector<Range>;
                    std::string old_etag = req.get_header_value("If-Range");
                    std::string cur_etag = StrEtag(info.real_path);

                    if(old_etag == cur_etag)
                    {
                        //size_t start = req.Ranges[i].first;
                        //size_t end = req.Ranges[i].second;
                        //httplib has completed this function, we only need to set the status code to 206, the detection of the 
                        //status code will intercept the specified range of data to respond;
                        std::cout << req.ranges[0].first << "---" << req.ranges[0].second << std::endl;
                        FileUtil(info.real_path).Read(&rsp.body);
                        rsp.set_header("Content-Type", "application/octet-stream");
                        rsp.set_header("Accept-Ranges", "bytes");
                        rsp.set_header("ETag",cur_etag);
                        rsp.status = 206;
                        return ;
                    }

                }

                FileUtil(info.real_path).Read(&rsp.body);
                rsp.set_header("Content-Type", "application/octet-stream");
                //tell the client that server supports resumable breakpoints;
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", StrEtag(info.real_path));
                rsp.status = 200;
                return ;
            }

        public:
            Server()
            {
                FileUtil(_backup_dir).CreateDirectory();
            }

            bool RunModule()
            {
                //搭建服务器
                //请求——映射关系
                _srv.Post("/upload", Upload);
                _srv.Get("/list", List);
                std::string regex_download_path = _url_prefix +"(.*)";
                _srv.Get(regex_download_path, Download);

                //启动服务器
                _srv.listen("0.0.0.0", _srv_port);

                return true;
            }

    };

}



#endif

