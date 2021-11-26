#pragma once
#include <drogon/HttpController.h>
#include <unordered_set>
#include <map>
#include <string>

using namespace drogon;
namespace mbta
{
namespace v1
{
class Route:public drogon::HttpController<Route>
{
  private:
    // sample stop
    static std::unordered_set<std::string> BFS;

    static std::unordered_set<std::string> GBD0;

    static std::unordered_set<std::string> GBD1;

    static std::unordered_set<std::string> GB;

    std::map<std::string, std::string> traversedLineMap;
    std::map<std::string, std::string> traversedStopMap;

    void mapTraverse(std::map<std::string, std::unordered_set<std::string>>* originMap, std::map<std::string, std::string>* targetMap);

    std::string validateRouteGB(std::string routeInput) const;

    std::string validateStop(std::string stopInput) const;

    std::string validateDirection(std::string directionInput) const;

    std::string validateSpecificGB(std::string routeInput, std::string stopInput, std::string directionInput) const;

  public:
    Route();
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

