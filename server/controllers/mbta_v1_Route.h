#pragma once
#include <drogon/HttpController.h>
using namespace drogon;
namespace mbta
{
namespace v1
{
class Route:public drogon::HttpController<Route>
{
  public:
    METHOD_LIST_BEGIN
    //use METHOD_ADD to add your custom processing function here;
    //METHOD_ADD(Route::get,"/{2}/{1}",Get);//path is /mbta/v1/Route/{arg2}/{arg1}
    //METHOD_ADD(Route::your_method_name,"/{1}/{2}/list",Get);//path is /mbta/v1/Route/{arg1}/{arg2}/list
    //ADD_METHOD_TO(Route::your_method_name,"/absolute/path/{1}/{2}/list",Get);//path is /absolute/path/{arg1}/{arg2}/list
    METHOD_ADD(Route::getInfo,"/{1}/{2}/{3}",Get);
    METHOD_ADD(Route::getBasicInfo,"/{1}",Get);
    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req,std::function<void (const HttpResponsePtr &)> &&callback,int p1,std::string p2);
    // void your_method_name(const HttpRequestPtr& req,std::function<void (const HttpResponsePtr &)> &&callback,double p1,int p2) const;
    void getInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route, std::string stop, std::string direction) const;
    void getBasicInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route) const;
};
}
}

