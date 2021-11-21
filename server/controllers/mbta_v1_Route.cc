#include "mbta_v1_Route.h"
#include <curl/curl.h>
#include <memory>
#include <string>
#include <stdexcept>

using namespace mbta::v1;

const std::string REQUEST_PREFIX = "https://api-v3.mbta.com/";
const std::string API_TOKEN = "api_key=8ba0ca46476449399829b5304937dd19";

// format function
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    auto buf = std::make_unique<char[]>( size );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

std::string getUrl(std::string direction, std::string stop, std::string line){
  return string_format("https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=%s&filter%5Bstop%5D=%s&filter%5Broute%5D=%s&api_key=8ba0ca46476449399829b5304937dd19", direction.c_str(), stop.c_str(), line.c_str());
}

//add definition of your processing function here

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s) {
  size_t newLength = size*nmemb;
  try {
    s->append((char*)contents, newLength);
  }
  catch(std::bad_alloc &e) {
    //handle memory problem
    std::cout<<e.what()<<"\n";
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
    try {
      LOG_DEBUG<<"Route "<<route<<"\n";
      LOG_DEBUG<<"Stop "<<stop<<"\n";
      LOG_DEBUG<<"Direction "<<direction<<"\n";

      // TODO: Dynamic query values based on request route
      //std::string url = "https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=1&filter%5Bstop%5D=place-bland&filter%5Broute%5D=Green-B&api_key=8ba0ca46476449399829b5304937dd19";
      std::string url = getUrl(direction,stop,route);

      Json::Value ret;
      ret["route"]=route;
      ret["stop"]=stop;
      ret["direction"]=direction;
      ret["data"] = getData(url).c_str();
      ret["source"] = url;
      auto resp=HttpResponse::newHttpJsonResponse(ret);
      callback(resp);
    } catch (...){
      Json::Value ret;
      ret["route"]=route;
      ret["stop"]=stop;
      ret["direction"]=direction;
      ret["Error"] = "Unable to get data.";
      auto resp=HttpResponse::newHttpJsonResponse(ret);
      callback(resp);
    }

}

std::string createUrlForBasicInfo(std::string route) {
  return REQUEST_PREFIX + "routes/" + route + "?" + API_TOKEN;
}

std::string validateRoute(std::string routeInput) {
  if (routeInput == "Green-B" || routeInput == "green-b" || routeInput == "greenb" || routeInput == "b" || routeInput == "B") {
    return "Green-B";
  }

  return "";
}

void Route::getBasicInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route) const {
    LOG_DEBUG<<"Route "<<route<<"\n";

    std::string validatedRoute = validateRoute(route);
    if (validatedRoute == "") {
      Json::Value ret;
      std::string message = "Invalid route input " + route;
      ret["message"] = message;

      auto resp = HttpResponse::newHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);

      return;
    }

    std::string url = createUrlForBasicInfo(validatedRoute);
 
    Json::Value ret;
    ret["route"] = route;
    ret["data"] = getData(url).c_str();

    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
}


