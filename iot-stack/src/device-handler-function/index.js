const AWS = require('aws-sdk');
const docClient = new AWS.DynamoDB.DocumentClient();

/**
 * GET handler returns the telemetry data for the device id specified on the path parameter.
 * @param {object} event - AWS Lambda Integration Event 2.0 Object.
 * @param {object} context - AWS Context Object
 * @returns {object} - Lambda status message or the telemetry object.
 */
exports.getHandler = async (event) => {
  console.log('getHandler called.');
  console.log(JSON.stringify(event, undefined, 2));

  if (!validApiKey(event)) {
    return { statusCode: 401, body: 'Bad API key.' };
  }
  
  return await getDeviceTelemetry(event?.pathParameters?.deviceId);
};

/**
 * Takes telemetry data from the device and UPSERTs the data into 
 * DynamoDB. Then returns back the shouldOperate value. This value
 * is true if the action device should activate the relay on the device.
 * 
 * @param {object} event - AWS Lambda Integration Event 2.0 Object.
 * @returns {object} - {shouldOperate: boolean}
 */
exports.putHandler = async (event) => {
  console.log('putHandler called.');
  console.log(JSON.stringify(event, undefined, 2));

  if (!validApiKey(event)) {
    return { statusCode: 401, body: 'Bad API key.' };
  }
  
  let data;

  if (event?.isBase64Encoded && event?.body) {
    data = JSON.parse(Buffer.from(event.body, 'base64').toString('ascii'));
  } else if (event?.body) {
    data = JSON.parse(event.body);
  }

  const deviceId = event?.pathParameters?.deviceId;

  if (!deviceId) {
    return { statusCode: 400 };
  }

  const updatedDeviceTelemetry = await updateTelemetry(deviceId, data);

  return { shouldOperate: updatedDeviceTelemetry?.shouldOperate ? true : false };
};

/**
 * Takes telemetry data from the device and UPSERTs the data into 
 * DynamoDB. This call is used to turn the shouldOperate property to false
 * in DynamoDB after the relay has been toggled.
 * 
 * @param {object} event - AWS Lambda Integration Event 2.0 Object.
 * @returns {object} - nothing or lambda status object.
 */
exports.ackOperateHandler = async (event) => {
  console.log('ackOperateHandler called.');
  console.log(JSON.stringify(event, undefined, 2));

  if (!validApiKey(event)) {
    return { statusCode: 401, body: 'Bad API key.' };
  }

  const updatedTelemetry = {
    shouldOperate: false,
  };

  const deviceId = event?.pathParameters?.deviceId;

  if (!deviceId) {
    return { statusCode: 400 };
  }

  await updateTelemetry(deviceId, updatedTelemetry);
}

/**
 * Compares the authorization header (no 'Bearer' word expected) to the
 * API KEY. If they do not match we fail.
 * @param {object} event - AWS Lambda Integration Event 2.0 Object.
 * @returns 
 */
const validApiKey = event => {
  const headers = event?.headers;

  if (!headers) return false;

  if (headers['authorization'] === process.env['API_KEY']) {
    return true;
  }

  return false;
}

/**
 * UPSERTS the 'telemetry' object into the DynamoDB record. Anything that
 * exists prior is overwritten by any new values from the telemetry object
 * passed in. If there is a value in the DB already but it is not passed in 
 * with the telemetry value that value will not be overwritten. So only values
 * in the telemetry object passed in here are overwritten.
 * 
 * @param {string} deviceId - the device id we are updating telemetry for.
 * @param {object} telemetry - the device telemetry data.
 * @returns {object} - Returns the merged and updated telemetry data.
 */
const updateTelemetry = async (deviceId, telemetry) => {
  const existingDeviceTelemetry = await getDeviceTelemetry(deviceId);

  const updatedDeviceTelemetry = {
    ...existingDeviceTelemetry?.telemetry,
    ...telemetry,
    ... { lastUpdated: new Date().getTime() },
  };

  const request = {
    TableName: process.env['TABLE_NAME'],
    Item: {
      id: deviceId,
      telemetry: updatedDeviceTelemetry,
    }
  };

  let result;

  try {
    result = await docClient.put(request).promise();
    result = result?.Item;
  } catch (err) {
    result = { error: err.message };
  }

  return updatedDeviceTelemetry;
};

/**
 * Gets the telemetry data from the DynamoDB.
 * 
 * @param {string} deviceId - the id of the device you want telemetry data for.
 * @returns {object} - telemetry data.
 */
const getDeviceTelemetry = async (deviceId) => {
  if (!deviceId) return {};

  const request = {
    TableName: process.env['TABLE_NAME'],
    Key: {
      id: deviceId,
    }
  };

  let result;

  try {
    const existing = await docClient.get(request).promise();
    result = existing?.Item;
  } catch (err) {
    result = { error: err.message };
  }

  return result;
}