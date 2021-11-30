#include "mbta_v1_Route.h"
#include <curl/curl.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_set>
#include <map>

using namespace mbta::v1;

const std::string REQUEST_PREFIX = "https://api-v3.mbta.com/";
const std::string API_TOKEN = "api_key=8ba0ca46476449399829b5304937dd19";

// sample stops and line
std::unordered_set<std::string> Route::BFS({
  "Blandford Street", "Blandford-Street", "blandford street", "blandford-street", "place-bland", "bfs", "BFS"
});

std::unordered_set<std::string> Route::BUE({
  "Boston University East", "Boston-University-East", "boston-university-east", "boston university east", "bu-east", "BU-east", "bu east", "BU east", "BU East", "place-buest", "bue", "BUE"
});


std::unordered_set<std::string> Route::GBD0({
  "0", "boston-college"
});

std::unordered_set<std::string> Route::GBD1({
  "1", "govern-center"
});


std::unordered_set<std::string> Route::GB({
  "Green-B", "green-b", "greenb", "b", "B", "Green B", "green b", "green B", "B"
});

Route::Route() {
  std::map<std::string, std::unordered_set<std::string>> line {
    {"Green-B", GB}
  };

  std::map<std::string, std::unordered_set<std::string>> stop {
    {"place-bland", BFS},
    {"place-buest", BUE}
  };

  mapTraverse(&line, &traversedLineMap);
  mapTraverse(&stop, &traversedStopMap);
}

void Route::mapTraverse(
  std::map<std::string, std::unordered_set<std::string>>* originMap,
  std::map<std::string, std::string>* targetMap) {
  for (auto mapItr = originMap->begin(); mapItr != originMap->end(); ++mapItr){
    std::unordered_set<std::string>* userInput = &mapItr->second;
    for (auto itr = userInput->begin(); itr != userInput->end(); ++itr){
      targetMap->insert({ (*itr),  mapItr->first });
    }
  }
}

class InvalidPathError: public std::runtime_error{
  public:
    InvalidPathError(std::string msg) : std::runtime_error(msg) { }
};

class RemoteDataError: public std::runtime_error{
  public:
    RemoteDataError(std::string msg) : std::runtime_error(msg) { }
};


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

CURL* initCurl() {
  try {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return curl_easy_init();
  } catch (...) {
    throw RemoteDataError("Fail to send request");
  }
}

void configCurl(CURL* curl, std::string* s){
  struct curl_slist *chunk = NULL;
  chunk = curl_slist_append(chunk, "accept: application/vnd.api+json");
  chunk = curl_slist_append(chunk, "x-api-key: 8ba0ca46476449399829b5304937dd19");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, s);
}

void sendRequest(CURL* curl){
  CURLcode res = curl_easy_perform(curl);

  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  }
}

std::string getData(std::string url) {
  CURL *curl = initCurl();

  std::string s;

  configCurl(curl, &s);

  curl_easy_setopt(curl, CURLOPT_URL,url.c_str());

  sendRequest(curl);

  curl_easy_cleanup(curl);

  if(s == ""){
    throw RemoteDataError("Fail to fetch remote data");
  }

  return s;
}

std::string createUrlForInfo(std::string direction, std::string stop, std::string route) {
  return REQUEST_PREFIX + "predictions?filter%5Bdirection_id%5D=" + direction + "&filter%5Bstop%5D=" + stop + "&filter%5Broute%5D=" + route + "&"+ API_TOKEN;
}

std::string Route::validateRouteGB(std::string routeInput) const {
  std::map<std::string, std::string>::const_iterator iter = traversedLineMap.find(routeInput);
  if(iter != traversedLineMap.end()){
    return iter->second;
  }

  throw InvalidPathError ("Invalid line input: " + routeInput);
}

std::string Route::validateStop(std::string stopInput) const {
  std::map<std::string, std::string>::const_iterator iter = traversedStopMap.find(stopInput);
  if(iter != traversedStopMap.end()){
    return iter->second;
  }

  throw InvalidPathError ("Invalid line input: " + stopInput);
}

std::string Route::validateDirection(std::string directionInput) const {
  if (GBD0.count(directionInput)) {
    return "0";
  }

  if (GBD1.count(directionInput)) {
    return "1";
  }

  throw InvalidPathError("Invalid direction input: " + directionInput);
}

std::string Route::validateSpecificGB(std::string routeInput, std::string stopInput, std::string directionInput) const {

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
    ret["route"] = validatedRoute;
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
