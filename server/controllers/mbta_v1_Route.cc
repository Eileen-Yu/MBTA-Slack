#include "mbta_v1_Route.h"
#include <curl/curl.h>
#include <memory>
#include <string>
#include <stdexcept>

using namespace mbta::v1;

const std::string REQUEST_PREFIX = "https://api-v3.mbta.com/";
const std::string API_TOKEN = "api_key=8ba0ca46476449399829b5304937dd19";

class InvalidPathError: public std::runtime_error{
  public:
    InvalidPathError(std::string msg) : std::runtime_error(msg) { }
};

class RemoteDataError: public std::runtime_error{
  public:
    RemoteDataError(std::string msg) : std::runtime_error(msg) { }
};

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

  // FIXME: Cannot caught remote data error when request failed
  if(s == ""){
    throw RemoteDataError("Fail to fetch remote data");
  }
  return s;
}

std::string createUrlForInfo(std::string direction, std::string stop, std::string route) {
  return REQUEST_PREFIX + "predictions?filter%5Bdirection_id%5D=" + direction + "&filter%5Bstop%5D=" + stop + "&filter%5Broute%5D=" + route + "&"+ API_TOKEN;
}

std::string validateRouteGB(std::string routeInput) {
  if (routeInput != "Green-B" && routeInput != "green-b" && routeInput != "greenb" && routeInput != "b" && routeInput != "B") {
    throw InvalidPathError("Invalid route input: " + routeInput);
  }

  return "Green-B";
}

std::string validateStop(std::string stopInput) {
  // TODO: Create a stop map
  return "place-bland";
}

std::string validateDirection(std::string directionInput) {
  // TODO: Create a direction map
  return "0";
}

std::string validateSpecificGB(std::string routeInput, std::string stopInput, std::string directionInput) {
  std::string route = validateRouteGB(routeInput);
  std::string stop = validateStop(stopInput);
  std::string direction = validateDirection(directionInput);

  return createUrlForInfo(direction, stop, route);
}

void Route::getInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route, std::string stop, std::string direction) const {
  LOG_DEBUG<<"Route "<<route<<"\n";
  LOG_DEBUG<<"Stop "<<stop<<"\n";
  LOG_DEBUG<<"Direction "<<direction<<"\n";

  try{
    std::string validateURL = validateSpecificGB(route, stop, direction);
    Json::Value ret;
    ret["route"]=route;
    ret["stop"]=stop;
    ret["direction"]=direction;
    ret["data"] = getData(validateURL).c_str();
    ret["source"] = validateURL;
    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
  } catch (const InvalidPathError& e ) {
    Json::Value ret;
    ret["message"] = e.what();

    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } catch (const RemoteDataError& e) {
    Json::Value ret;
    ret["message"] = e.what();

    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
  }
}

std::string createUrlForBasicInfo(std::string route) {
  return REQUEST_PREFIX + "routes/" + route + "?" + API_TOKEN;
}

void Route::getBasicInfo(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string route) const {
  LOG_DEBUG<<"Route "<<route<<"\n";

  try {
    std::string validatedRoute = validateRouteGB(route);

    std::string url = createUrlForBasicInfo(validatedRoute);
 
    Json::Value ret;
    ret["route"] = route;
    ret["data"] = getData(url).c_str();

    auto resp=HttpResponse::newHttpJsonResponse(ret);
    callback(resp);
  } catch (const InvalidPathError& e ) {
    Json::Value ret;
    ret["message"] = e.what();

    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } catch (const RemoteDataError& e) {
    Json::Value ret;
    ret["message"] = e.what();

    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
  }
}
