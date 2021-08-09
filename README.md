# Garage Door Opener

This repository contains all the code required to build a Wifi connected garade door opener that works with ANY garage door. It includes the Arduino sketch, a React-Native Mobile Application, and a simple AWS API to cloud enable your devices.

## Projects

```
.
├── Arduino-Projects/
│   ├── ActionDevice - Arduino sketch for the esp8266 relay device.
│   └── SensorDevice - Arduino sketch for the open/close sensor
├── garagedoorapp   - React-Native application
└── iot-stack       - AWS API Gateway stack to cloud enable the Arduino devices.
```

## Variables you need to create.

|Variable Name|Description|
|---|---|
|ActionDeviceID|A string to identify the action device. Generate a UUID for this.|
|SensorDeviceID|A string to identify the sensor device. Generate a UUID for this.|
|API Key|A string used as a shared password between the device, mobile, and api projects. I create 2 UUID and use that as my key|

**Note** you can conveniently create UUIDs with this handy online tool. I do not include dashes or braces. [https://www.guidgenerator.com/](https://www.guidgenerator.com/)

So these ID's and Key you are going to create/make up on your own. Be sure to use the correct keys in the right places.


## What this project is NOT

This is not an AWS IoT project. Instead, I created a very simple API and DynamoDB to maintain device state and allow the mobile application
to interact with the device. 

Why does this not use AWS IoT. For one, I am not experienced in AWS IoT and I needed something quick and simple to get the job done.

## Why? What is this for?

So a few years ago I bought a 'My Q' garage door opened from Amazon for about $90. They have since gone down to as low as about $20. However my 
garage door motor head circuit board died and it was replaced with an updated version that no longer worked with the 'My Q' device.  I never realized how much I used the opener from my phone until it was gone.  So instead of paying $400 for an Overhead Door add-on board for my garage door opener I built my own. Each device (there are 2) costs about $1.50 ($3 if you are impatient).  But even if you are impatient and pay $6 for 2 devices off Amazon it is still $14 cheaper than the lowest price on Amazon however you have the added benefit that you can program it to do whatever you need.

