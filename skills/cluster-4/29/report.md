#  Skill 29 Security

Author: Ritam Das

Date: 2020-11-13
-----

## Summary
### Remote Controlled Robot/Car Security Vulnerabilities:

The communication between the ESP32 and the node server uses UDP, which also presents a security vulnerability. This no acknowledgement protocol is highly susceptible to spoofing and DOS attacks. Thus, anyone can send large volumes of packets to the server, potentially preventing allowing authorized users to cast votes or view data. To mititgate this, a section of code in server.js could be implemented such that all incoming packets to the server are monitored and limited at a certain threshold (packets per second, etc.), temporarily denying further incoming packets until below the threshold again.

UDP communication is also vulnerable to packet sniffing. An unauthorized user could intercept packets that are sent over UDP from the ESP32 to the node server and decipher the data within those packets (Wireshark). A potential solution to this is to use TCP communication, which is a safer handshaking protocol. TCP first confirms that the recipient is valid for data transmission and sends data following this acknowledgement. However, TCP would increase latency when communicating to the node server as UDP is a much faster protocol with a security tradeoff.

An unauthorized user also has the potential to log into the Pi directly and read into the database and server code. This could result in tampered data and unauthorized admin access. To prevent this, the wireless network the Pi is connected to should be setup for the WPA2 security mode, which is a much stronger security mode than WEP (currently being used & can be brute forced easily).

The web client to control the car is susceptible to DDOS attacks, making the website/server unreachable. To prevent this, you can provision more bandwidth  to handle spikes in incoming website traffic. 

The web client hosted on the node server could easily be accessed from any device on the local network. An authorized login can also be implemented on the web client to help deter unfriendly users.

## Sketches and Photos
![IMG_33C84155F6CC-1](https://user-images.githubusercontent.com/37518854/99200275-532ac900-2772-11eb-9107-163825d233c8.jpeg)


## Modules, Tools, Source Used Including Attribution


## Supporting Artifacts


-----
