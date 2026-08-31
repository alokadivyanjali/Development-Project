# Development-Project

This project develops a Solar-Powered Autonomous Smart Bin using IoT technology to improve waste management and environmental monitoring. It provides real-time monitoring, emergency detection, and route optimisation to reduce costs, fuel consumption, and environmental pollution.

The proposed solution is a two layered hardware and software architecture. The physical layer is a basic trash can with the addition of a number of environmental sensors (DHT11 (Digital Humidity), Ultrasonic Distance Measurement Sensor, MQ-2 Smoke Sensor and a SW-520D Tilt Sensor). The node runs on an ESP32 microcontroller with the "Deep Sleep" power conservation feature, and receives physical data, then sends a thin JSON payload over a 2G GPRS connection through the SIM800L module.
The Thing Speak HTTP bridge is used to be a secure intermediary between two networks with conflicting security standards, specifically the modern network and the legacy 2G network, that sends the data to a Google Firebase Realtime Database. The application layer has a custom-built responsive JS/HTML web dashboard. Because everything in the park can be done through login in, park administrators can be able to monitor the capacity of trash bins in real-time, view fires or monkey vandalism in real-time if there are any and can also plan their routes in the park algorithmically by interacting with a map to transform the passive park infrastructure into an intelligent data-driven ecosystem. In conclusion, this thesis suggests a scalable blueprint which can be adapted in the other national parks and protected reserves as a zero-emission blueprint without compromising nature.

<img width="940" height="1106" alt="image" src="https://github.com/user-attachments/assets/8818d704-d094-4847-b65d-f3950e997b48" />


Web Dashboard  

<img width="940" height="429" alt="image" src="https://github.com/user-attachments/assets/12e177a5-5d04-4d6b-9fe7-08fcca6463a3" />

<img width="940" height="428" alt="image" src="https://github.com/user-attachments/assets/5f945836-78d3-430c-bb20-38c6c8de928e" />

