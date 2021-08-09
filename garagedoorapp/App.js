/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow strict-local
 */

import React, {useState, useEffect} from 'react';
import type {Node} from 'react';
import {
  Button,
  SafeAreaView,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  useColorScheme,
  View,
  TouchableOpacity,
  Image,
} from 'react-native';

import {Colors} from 'react-native/Libraries/NewAppScreen';

const apiUrlBase = 'ENTER_YOUR_API_URL';
const doorActionDeviceId = 'ENTER_ACTION_DEVICE_ID';
const doorStateDeviceId = 'ENTER_SENSOR_DEVICE_ID';
const apiKey = 'ENTER_YOUR_API_ID';

const App: () => Node = () => {
  const isDarkMode = useColorScheme() === 'dark';
  const [isOpen, setIsOpen] = useState(false);
  const backgroundStyle = {
    backgroundColor: isDarkMode ? Colors.black : Colors.white,
    flexGrow: 1,
  };

  const textStyle = {
    color: isDarkMode ? Colors.white : Colors.black,
    textAlign: 'center',
    margin: 12,
  };

  const getDoorState = async () => {
    console.log('getDoorState called.');
    const response = await fetch(apiUrlBase + doorStateDeviceId, {
      method: 'GET',
      headers: {
        Accept: 'application/json',
        'Content-Type': 'application/json',
        authorization: apiKey,
      },
    });

    console.log('response.status:', response.status);
    if (response.status === 200) {
      const result = await response.json();
      const isOpen = result?.telemetry?.isOpen;
      console.log('isOpen:', isOpen);
      setIsOpen(isOpen);
    }
  };

  useEffect(() => {
    getDoorState();
    const interval = setInterval(() => {
      getDoorState();
    }, 10000);
  }, []);

  return (
    <SafeAreaView style={backgroundStyle}>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />
      <ScrollView
        contentInsetAdjustmentBehavior="automatic"
        contentContainerStyle={{flexGrow: 1, justifyContent: 'center'}}
        style={backgroundStyle}>
        <View
          style={{
            backgroundColor: isDarkMode ? Colors.black : Colors.white,
          }}>
          <View style={styles.buttonContainer}>
            <TouchableOpacity onPress={toggleDoor} style={styles.roundButton}>
              <Image
                source={require('./images/garage-open.png')}
                style={styles.imageButton}
              />
            </TouchableOpacity>
          </View>
          <Text style={textStyle}>
            {isOpen ? 'Press to Close' : 'Press to Open'}
          </Text>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
};

const toggleDoor = async () => {
  console.log('operate button pushed.');
  return fetch(apiUrlBase + doorActionDeviceId, {
    method: 'PUT',
    headers: {
      Accept: 'application/json',
      'Content-Type': 'application/json',
      authorization: apiKey,
    },
    body: JSON.stringify({shouldOperate: true}),
  });
};

const styles = StyleSheet.create({
  imageButton: {
    width: 40,
    height: 40,
  },
  sectionContainer: {
    marginTop: 32,
    paddingHorizontal: 24,
  },
  sectionTitle: {
    fontSize: 24,
    fontWeight: '600',
  },
  sectionDescription: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
  },
  highlight: {
    fontWeight: '700',
  },
  roundButton: {
    width: 100,
    height: 100,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 10,
    borderRadius: 100,
    backgroundColor: '#00A4F0',
    borderWidth: 2,
    borderColor: '#0070A3',
  },
  buttonContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
});

export default App;
