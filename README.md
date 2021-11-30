# mbtaBot

## Introduction
mbtaBot is a slack bot which can feedback timely MBTA shedule. 
By introducing the bot in the slack channel and interacting with it, users can get at most three upcoming trains for a certain stop.

The command is quite easy, just like
```
@mbta {line} {stop} {direction} 
```
For example,
```
@mbta green-b blandford-street boston-college
```

Considering user experience, it also supports various kinds of abbreviation. For line, you can just type "greenb", or even just "b". For stop, something like "bfs" is also 
workable. As for the direction, "0" means outbound while "1" means inbound. So the resule is the same with the following command
```
@mbta b bfs 0
```
**p.s** As it's just a sample, `v1` just includes *Green-B* and several *BU* related stops. But with well-designed data structure(possible abbreviations are stored as unordered_set and then traversed to the specific one MBTA API uses through map), extension is only a matter of time.

### screenshot of test result
![avatar](https://i.ibb.co/8mdv5tj/test-result.jpg)

## Building
mbtaBot is composed by two parts. The main part is a RESTful API web service, which is written in C++ with dependencies on [Drogon](https://github.com/drogonframework/drogon) and [libcurl](https://curl.se/libcurl/). The project is managed by CMake. To simplify build & deploy, and extend availability in multiple platforms(MacOS, Windows, Unix, etc.), all things are wrapped into Docker. Once have Docker installed, you can simply run the following steps to start the service:

1. Build the Docker image based on the given `Dockerfile` in the `server/` directory

   ```shell
   # Create a Docker image with a tag mbta-server
   docker build -t mbta-server --no-cache .
   ```

2. Run the web service with the Docker image

   ```shell
   # Run the web service in the container & bind it with the host port 3003
   docker run -p 127.0.0.1:3003:80 mbta-server:latest
   ```

Once the service is launched, you can check the localhost with a route. For instance, in browser:

![avatar](https://i.ibb.co/g674nyv/RESTful-API.jpg)



The second part is a slack client implemented by a simple Python script. This is written with Python **3.8.3**. 

1. Install dependencies based on `requirements.txt`.
   (Virtual environment is suggested before installing dependencies.)

   ```shell
   pip install -r requirements.txt
   ```

2. Create a Slack bot. Ref: https://api.slack.com/messaging/webhooks
   Caution: Remember to generate and store **SLACK APP TOKEN** and **SLACK BOT TOKEN**

3. Write a `.env` file, put slack tokens and server host there:

   ```
   SLACK_BOT_TOKEN = "..."
   SLACK_APP_TOKEN = "..."
   SERVER=""http://localhost:3003"
   ```

4. Run slack client by:

   ```shell
   python3 app.py
   ```



## Project Experience

The idea of the project comes from the experience of taking transportation from MBTA with too many rail delays and cancelling.

The good news is MBTA provides its official restful api which makes it possible to query for the rail's status. So originally, I was thinking of creating a standalone app to get rail prediction info. But for a second thought, I feel like having a web service to provide rail info in a more human-readable manner is a more flexible and sustainable choice. Based on that, we can build customized app according to our needs. For instance, the slack bot to provide a simple user interface to get rail prediction info.

Certainly, several issues were conquered during the development. 

The first is how to choose appropriate data structures to store the abbreviation for line and stop names. The main difficulty is that for a certain line or stop name, user input can be various kind. For example, when querying `Green-B`, user may input `Green-B`,  `green-b`, `greenb`, `green b`, or even `b`. So it is hard to support all kinds of input at the very beginning because it really relies on the collection of user experience. We need to store the data in a structure which is not only easy to query, but also flexible for future expand. Thus, I used `std::unordered_set` to store various forms of input of a specific line or stop, and then traversed it to a `std::map`, where key is the abbreviation form while the value is the line/stop ID. In that way we can easily add new abbreviation in the unordered_set without attention to the traversed map. 

A derivative question is how to maintain and provide data for each request. Since stops and lines won't change at runtime, they can be set as static attributes.  

The second is "Address of stack memory associated with local variable returned" warning. This appears when I tried to return a pointer of a variable in a function scope. In this case the value in the address is not the one we need. The reason is that a variable declared in the function is generally stored on stack, so it will be automatically released when function completes and block goes out of the scope. I tried to use the `new` keyword to store the data in the heap, which worked. 

One interesting thing is when using libcurl to send request, the official doc suggestes to create a pointer `CURL*` by ` curl_easy_init()`, which does not explicitly use the keyword `new`. But the data seemed to be stored in the heap memory still. By having a quick view on the [source code](https://github.com/curl/curl/blob/0942017162b064c223cc6111306bf06903f3262b/lib/url.c#L644), I noticed that CURL is put in the heap by `calloc`. 

The third is to handle with errors that may exist. For this part, I wrote two customized error classes `InvalidPathError` and `RemoteDataError` to categorize errors. The former can catch errors caused by invalid user input, while the later reflect all issues caused by unsuccessful request.  



