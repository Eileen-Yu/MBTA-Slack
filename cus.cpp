#include <curl/curl.h>
#include "json.hpp"

#include <iostream>
#include <string>

using json = nlohmann::json;

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


int main() {
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

    curl_easy_setopt(curl, CURLOPT_URL,  "https://api-v3.mbta.com/routes/Green-B?api_key=8ba0ca46476449399829b5304937dd19");

    // Prediction
    //curl_easy_setopt(curl, CURLOPT_URL,  "https://api-v3.mbta.com/predictions?filter%5Bdirection_id%5D=1&filter%5Bstop%5D=place-bland&filter%5Broute%5D=Green-B&api_key=8ba0ca46476449399829b5304937dd19");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    //curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output


    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    /* always cleanup */

    curl_easy_cleanup(curl);
  }


  auto j = json::parse(s.c_str());
  auto destinations = j["data"]["attributes"]["direction_destinations"];

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://hooks.slack.com/services/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");

    auto jslack = R"(
      {
        "blocks": [
          {
            "type": "section",
            "fields": [
              {
                "type": "mrkdwn",
                "text": "*Route:*\nGreen-B"
              },
              {
                "type": "mrkdwn",
                "text": "*Stop:*\nBlandford Street"
              },
              {
                "type": "mrkdwn",
                "text": "*Destination:*\nBoston College"
              },
              {
                "type": "mrkdwn",
                "text": "*Next:*\nUpcoming time"
              }
            ]
          }
        ]
      }
    )"_json;

    std::string des_prefix = "*Destination:*\n";
    std::string des = des_prefix.append(destinations[0]).append(", ").append(destinations[1]);

    jslack["blocks"][0]["fields"][2]["text"] = des.c_str();

    std::string slack_msg = jslack.dump();

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, slack_msg.c_str());
 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
 
    curl_easy_cleanup(curl);
  }

  std::cout<< "Program finished!" << std::endl;
}

