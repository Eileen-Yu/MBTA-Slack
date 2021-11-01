#include "mbta_v1_Route.h"
using namespace mbta::v1;
//add definition of your processing function here

void Route::getInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route, std::string stop, std::string direction) const {
    LOG_DEBUG<<"Route "<<route<<"\n";
    LOG_DEBUG<<"Stop "<<stop<<"\n";
    LOG_DEBUG<<"Direction "<<direction<<"\n";

    //验证token有效性等
    //读数据库或缓存获取用户信息
    Json::Value ret;
    ret["route"]=route;
    ret["stop"]=stop;
    ret["direction"]=direction;
    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}

