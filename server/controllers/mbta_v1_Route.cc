#include "mbta_v1_Route.h"
#include <curl/curl.h>
using namespace mbta::v1;
//add definition of your processing function here

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s) {
  size_t newLength = size*nmemb;
  try {
    s->append((char*)contents, newLength);
  }
  catch(std::bad_alloc &e) {
    //handle memory problem
    return 0;
  }
  return newLength;
}

std::string getData(std::string url) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  std::string s;

  if(curl) {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "accept: application/vnd.api+json");
    chunk = curl_slist_append(chunk, "x-api-key: 8ba0ca46476449399829b5304937dd19");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    
    curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
    //curl_easy_setopt(curl, CURLOPT_URL,  "https://api-v3.mbta.com/routes/Green-B?api_key=8ba0ca46476449399829b5304937dd19");

    // Prediction
    //curl_easy_setopt(curl, CURLOPT_URL,  "https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=1&filter%5Bstop%5D=place-bland&filter%5Broute%5D=Green-B&api_key=8ba0ca46476449399829b5304937dd19");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    //curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output


    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }
  return s;
}

void Route::getInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route, std::string stop, std::string direction) const {
    LOG_DEBUG<<"Route "<<route<<"\n";
    LOG_DEBUG<<"Stop "<<stop<<"\n";
    LOG_DEBUG<<"Direction "<<direction<<"\n";

    // TODO: Dynamic query values based on request route
    std::string url = "https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=1&filter%5Bstop%5D=place-bland&filter%5Broute%5D=Green-B&api_key=8ba0ca46476449399829b5304937dd19";

    Json::Value ret;
    ret["route"]=route;
    ret["stop"]=stop;
    ret["direction"]=direction;
    ret["data"] = getData(url).c_str();
    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}

void Route::getBasicInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route) const {
    LOG_DEBUG<<"Route "<<route<<"\n";
    std::string url = "https://api-v3.mbta.com/routes/Green-B?api_key=8ba0ca46476449399829b5304937dd19";
 
    Json::Value ret;
    ret["route"]=route;
    ret["data"]=getData(url).c_str();
    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}

