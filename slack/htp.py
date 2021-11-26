import requests
import json

response = requests.get("http://localhost:8888/mbta/v1/route/Green-B/place-bland/0")
data = response.json()["data"]
pred = json.loads(data)["data"][0]["attributes"]
print(pred["arrival_time"])

