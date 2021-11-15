import os
import json
import requests

from slack_bolt import App
from slack_bolt.adapter.socket_mode import SocketModeHandler
from dotenv import load_dotenv

load_dotenv()
SLACK_BOT_TOKEN = os.environ["SLACK_BOT_TOKEN"]
SLACK_APP_TOKEN = os.environ["SLACK_APP_TOKEN"]

MBTA_PREFIX = "http://localhost:8888/mbta/v1/route"

app = App(token=SLACK_BOT_TOKEN)

@app.event("app_mention")                                                                                                       
def mention_handler(body, say):
        msg = body["event"]["text"]
        _self, line, stop, direction = msg.replace("\xa0", " ").split(" ")
        print(line, stop, direction, sep = " ")
        # response = requests.get(MBTA_PREFIX + "/" + line +  "/Green-B/place-bland/0")
        requestUrl = MBTA_PREFIX + "/" + line + "/" + stop + "/" + direction
        print(111111111111)
        print(requestUrl)

        response = requests.get(requestUrl)

        rawData = response.json()["data"] 
        predicts = json.loads(rawData)["data"]

        pred = predicts[0]["attributes"]["arrival_time"]
        mid = predicts[1]["attributes"]["arrival_time"]
        last = predicts[2]["attributes"]["arrival_time"]


        slack_response = {
            "text": "Green-B Blandford Street",
        	"blocks": [
        		{
        			"type": "section",
        			"fields": [
        				{
        					"type": "mrkdwn",
        					"text": "*Upcoming 1:*\n" + pred
        				},
        				{
        					"type": "mrkdwn",
        					"text": "*Upcoming 2:*\n" + mid
        				},
        				{
        					"type": "mrkdwn",
        					"text": "*Upcoming 3:*\n" + last
        				}
        			]
        		}
        	]
        }

        response_data = json.dumps(slack_response)

        print(response_data)
        say(slack_response)


if __name__ == "__main__":
        handler = SocketModeHandler(app, SLACK_APP_TOKEN)
        handler.start()
