/**
 * libdatachannel client example
 * Copyright (c) 2020 Staz Modrzynski
 * Copyright (c) 2020 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

#include "rtc/rtc.hpp"
#include "nlohmann/json.hpp"

#include "client.hpp"

using namespace std;
using json = nlohmann::json;

// struct Receiver {
// 	std::shared_ptr<rtc::PeerConnection> conn;
// 	std::shared_ptr<rtc::Track> track;
// };

// all connected clients
unordered_map<string, shared_ptr<Client>> clients{};

struct ConnectedPair {
	std::weak_ptr<Client> client1;
	std::weak_ptr<Client> client2;
	bool ready = false;
};

ConnectedPair connectionPair;

void handleRequest(string id, rtc::Configuration config, shared_ptr<rtc::WebSocket> ws) {

	if (clients.find(id) == clients.end()) { //not found
		// cout << "New Client!" <<endl;
		auto client = make_shared<Client>(id, ws);
		client->createPeerConnection(config);
		clients.emplace(id, client);
		client->onDisconnect([](Client * c){
			clients.erase(c->getID());
		});

		// client->onConnect([client](Client * c){
		// 	cout << "Client " << c->getID() << " connected" << endl;
		// 	if(connectionPair.client1.expired()){
		// 		cout << "Set as Client 1" << endl;
		// 		connectionPair.client1 = client;
		// 	}else if(connectionPair.client2.expired()){
		// 		cout << "Set as Client 2" << endl;
		// 		connectionPair.client2 = client;
		// 	}
		// });
	}
}

void handlerAnswer(string id, string type, string sdp, shared_ptr<rtc::WebSocket> ws){
	// cout << "Handling ans" << endl;
	if (auto res = clients.find(id); res != clients.end()) {
		// cout << "Found Client" << endl;
		auto client = res->second;
		auto description = rtc::Description(sdp, type);
		client->setRemoteDescription(description);
	}
}

void wsOnMessage(json msg, rtc::Configuration config, shared_ptr<rtc::WebSocket> ws) {
    auto it = msg.find("id");
    if (it == msg.end())
        return;
    string id = it->get<string>();

    it = msg.find("type");
    if (it == msg.end())
        return;
    string type = it->get<string>();

	cout << "Got Request from "<< id <<" of type "<< type << endl;

	if(type == "request"){
		handleRequest(id, config, ws);
	} else if(type == "answer"){
		it = msg.find("sdp");
		if (it == msg.end())
			return;
		string sdp = it->get<string>();
		handlerAnswer(id, type, sdp, ws);
	}

    // if (type == "request") {
    //     clients.emplace(id, createPeerConnection(config, make_weak_ptr(ws), id));
    // } else if (type == "answer") {
    //     if (auto jt = clients.find(id); jt != clients.end()) {
    //         auto pc = jt->second->peerConnection;
    //         auto sdp = message["sdp"].get<string>();
    //         auto description = Description(sdp, type);
    //         pc->setRemoteDescription(description);
    //     }
    // }
}



int main() {
	// std::vector<std::shared_ptr<Receiver>> receivers;
	const string websocketIPAddress = "127.0.0.1";
	const uint16_t websocketPort = 5000;
	const string serverID = "server";
	const string websocketURL= "wss://" + websocketIPAddress + ":" + to_string(websocketPort) + "/" + serverID;

	try {
		//set WebRTC logging level
		rtc::InitLogger(rtc::LogLevel::Info);

		// arg parsing
		// HA, LOL 

		//Configuration settings
		rtc::Configuration webRTCConfig;
		string stunServer = "stun:stun.l.google.com:19302";
		cout << "STUN server is " << stunServer << endl;
		webRTCConfig.iceServers.emplace_back(stunServer);
		webRTCConfig.portRangeBegin = 65000;
		// webRTCConfig.disableAutoNegotiation = true;

		rtc::WebSocket::Configuration webSocketConfig;
		webSocketConfig.disableTlsVerification = true; //need for the server to accept the self signed certificate 

		//Websocket setup and connection
		auto ws = make_shared<rtc::WebSocket>(webSocketConfig);

		ws->onOpen([]() { cout << "WebSockets: Connected, Signaling ready!" << endl; });
		ws->onClosed([]() { cout << "WebSockets: WARNING: Connection Closed!" << endl; });
		ws->onError([](const string &error) { cout << "WebSockets: ERROR: Failed with error: " << error << endl; });

		ws->onMessage([&](variant<rtc::binary, string> data) {
			if (!holds_alternative<string>(data))
				return;

			json message = json::parse(get<string>(data));
			wsOnMessage(message, webRTCConfig, ws);
		});

		cout << "Connecting to Websocket at '" << websocketURL <<"' with id " << serverID << endl;
		ws->open(websocketURL);

		cout << "Waiting for signaling Websocket to connect..";
		while (!ws->isOpen()) {
			cout << ".";
			if (ws->isClosed())
				return 1;
			this_thread::sleep_for(100ms);
		}
		cout << endl;

		while (true)
		{	
			// if(!connectionPair.client1.expired() && !connectionPair.client2.expired()){
			// 	if(!connectionPair.ready){

			// 		cout << "Connecting Clients together" << endl;
			// 		connectionPair.ready = true;

			// 		shared_ptr<Client> c1 = connectionPair.client1.lock();
			// 		shared_ptr<Client> c2 = connectionPair.client2.lock();

			// 		shared_ptr<rtc::Track> c1Track = c1->getSendTrack();
			// 		shared_ptr<rtc::Track> c2Track = c2->getSendTrack();

			// 		c1->setSendTrack(c2Track);
			// 		c2->setSendTrack(c1Track);

			// 	}
			// }else{
			// 	connectionPair.ready = false; 
			// }

			this_thread::sleep_for(100ms);
		}
		


		// std::cout << "Press any key to exit." << std::endl;
		// char dummy;
		// std::cin >> dummy;

		return 0;

		// auto pc = std::make_shared<rtc::PeerConnection>();
		// pc->onStateChange(
		//     [](rtc::PeerConnection::State state) { std::cout << "State: " << state << std::endl; });
		// pc->onGatheringStateChange([pc](rtc::PeerConnection::GatheringState state) {
		// 	std::cout << "Gathering State: " << state << std::endl;
		// 	if (state == rtc::PeerConnection::GatheringState::Complete) {
		// 		auto description = pc->localDescription();
		// 		json message = {{"type", description->typeString()},
		// 		                {"sdp", std::string(description.value())}};
		// 		std::cout << "Please copy/paste this offer to the SENDER: " << message << std::endl;
		// 	}
		// });

		// const rtc::SSRC targetSSRC = 42;

		// //receive channel
		// rtc::Description::Audio media("recv", rtc::Description::Direction::RecvOnly);
		// media.addOpusCodec(111);
		// media.setBitrate(
		//     100); // Request 100kbps

		// auto recvTrack = pc->addTrack(media);
		// recvTrack->setMediaHandler(std::make_shared<rtc::RtcpReceivingSession>());
		
		// //send channel
		// rtc::Description::Audio media2("send", rtc::Description::Direction::SendOnly);
		// media2.addOpusCodec(111);
		// media2.setBitrate(100);
		// media2.addSSRC(targetSSRC, "audio-send");

		// auto sendTrack = pc->addTrack(media2);

		// //echo audio back
		// recvTrack->onMessage(
		//     [&sendTrack, targetSSRC](rtc::binary message) {
		// 	    // This is an RTP packet
		// 	    auto rtp = reinterpret_cast<rtc::RtpHeader *>(message.data());
		// 	    rtp->setSsrc(targetSSRC);
		// 		if(sendTrack != nullptr && sendTrack->isOpen()){
		// 			sendTrack->send(message);
		// 		}
		// 		printf("Got message of size %ld, rtp size %ld\r\n", message.size(), rtp->getSize());
		//     },
		//     nullptr);

		// pc->setLocalDescription();

		// // Set the sender's answer
		// std::cout << "Please copy/paste the answer provided by the SENDER: " << std::endl;
		// std::string sdp;
		// std::getline(std::cin, sdp);
		// std::cout << "Got answer" << sdp << std::endl;
		// json j = json::parse(sdp);
		// rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
		// pc->setRemoteDescription(answer);
		
		// std::cout << "Press any key to exit." << std::endl;
		// // char dummy;
		// std::cin >> dummy;

		// fclose(fout);

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
