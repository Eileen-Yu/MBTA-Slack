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

MAX_PREDICTION = 3

app = App(token=SLACK_BOT_TOKEN)

@app.event("app_mention")                                                                                                       
def mention_handler(body, say):
        print("[Request]" + json.dumps(body))

        msg = body["event"]["text"]
        _self, line, stop, direction = msg.replace("\xa0", " ").split(" ")
        requestUrl = MBTA_PREFIX + "/" + line + "/" + stop + "/" + direction

        response = requests.get(requestUrl)
        rawData = response.json()["data"] 
        predicts = json.loads(rawData)["data"]

        all_fields = [{
                "type": "mrkdwn",
                "text": "*Upcoming " + str(i + 1) + ":*\n" + p["attributes"]["arrival_time"]
                } for i, p in enumerate(predicts)]

        fields = all_fields[:MAX_PREDICTION] if len(all_fields) > 0 else {
                "type": "mrkdwn",
                "text": "*No upcoming rails*"
        }

        slack_response = {
            "text": "Green-B Blandford Street",
        	"blocks": [
        		{
        			"type": "section",
                                "fields": fields
        		}
        	]
        }

        # No need to print logs for none-error cases
        # print("[Response]" + json.dumps(slack_response, indent = 2))

        say(slack_response)


if __name__ == "__main__":
        handler = SocketModeHandler(app, SLACK_APP_TOKEN)
        handler.start()
