# ESP32 TelegramBot Home Automatic

ESP32 TelegramBot Home Automatic is a project that utilizes the ESP32 microcontroller and Telegram Bot API to control home devices automatically through Telegram messages.

## Project Description

This project aims to provide remote control over home devices through Telegram messages. By using the ESP32 microcontroller connected to Wi-Fi and communicating with the Telegram Bot API, users can send commands via Telegram messages to control home devices connected to the ESP32.

Some example applications that can be implemented in this project include controlling lights, fans, door locks, or other electronic devices connected to the ESP32. Through Telegram messages, users can turn devices on/off, set schedules, or monitor device status in real-time.

## Features

- Control home devices through Telegram messages.
- Support commands to turn devices on/off.
- Set scheduled tasks for automatic execution.
- Provide real-time device status updates via Telegram messages.
- Manage multiple home devices within a single Telegram account.

## Required Components

- ESP32 microcontroller
- Wi-Fi module (e.g., ESP8266)
- Breadboard or PCB for prototyping
- Connecting wires and required electronic components
- Telegram application on a mobile phone or computer

## How It Works

1. The ESP32 connects to the home Wi-Fi network.
2. The ESP32 connects to the Telegram Bot API using a valid bot token.
3. Users send commands via Telegram messages to the connected bot.
4. The bot forwards the commands to the ESP32 over the Wi-Fi connection.
5. The ESP32 receives the commands and controls the connected home devices.
6. The ESP32 sends a confirmation to the bot, and the bot replies to the user with a response message.

## Installation Steps

1. Set up the Arduino IDE or PlatformIO development environment.
2. Install the ESP8266 library for Wi-Fi connection.
3. Create a Telegram bot using BotFather and obtain the bot token.
4. Configure the ESP32 for Wi-Fi connection and bot token.
5. Add the program code to connect the ESP32 with the Telegram Bot API and control home devices.
6. Upload the program to the ESP32 using Arduino IDE or PlatformIO.
7. Connect the home devices you want to control to the ESP32.
8. Start controlling home devices through Telegram messages.

## Contribution

If you would like to contribute to this project, please open a pull request in the project's GitHub repository. We welcome your contributions.

## License

This project is licensed under the MIT License.
