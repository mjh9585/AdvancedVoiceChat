/**
 * libdatachannel client example
 * Copyright (c) 2020 Staz Modrzynski
 * Copyright (c) 2020 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "rtc/rtc.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

using nlohmann::json;

struct Receiver {
	std::shared_ptr<rtc::PeerConnection> conn;
	std::shared_ptr<rtc::Track> track;
};

int main() {
	std::vector<std::shared_ptr<Receiver>> receivers;

	try {
		rtc::InitLogger(rtc::LogLevel::Verbose);

		auto pc = std::make_shared<rtc::PeerConnection>();
		pc->onStateChange(
		    [](rtc::PeerConnection::State state) { std::cout << "State: " << state << std::endl; });
		pc->onGatheringStateChange([pc](rtc::PeerConnection::GatheringState state) {
			std::cout << "Gathering State: " << state << std::endl;
			if (state == rtc::PeerConnection::GatheringState::Complete) {
				auto description = pc->localDescription();
				json message = {{"type", description->typeString()},
				                {"sdp", std::string(description.value())}};
				std::cout << "Please copy/paste this offer to the SENDER: " << message << std::endl;
			}
		});

		const rtc::SSRC targetSSRC = 42;

		//receive channel
		rtc::Description::Audio media("recv", rtc::Description::Direction::RecvOnly);
		media.addOpusCodec(111);
		media.setBitrate(
		    100); // Request 100kbps

		auto recvTrack = pc->addTrack(media);
		recvTrack->setMediaHandler(std::make_shared<rtc::RtcpReceivingSession>());
		
		//send channel
		rtc::Description::Audio media2("send", rtc::Description::Direction::SendOnly);
		media2.addOpusCodec(111);
		media2.setBitrate(100);
		media2.addSSRC(targetSSRC, "audio-send");

		auto sendTrack = pc->addTrack(media2);

		//echo audio back
		recvTrack->onMessage(
		    [&sendTrack, targetSSRC](rtc::binary message) {
			    // This is an RTP packet
			    auto rtp = reinterpret_cast<rtc::RtpHeader *>(message.data());
			    rtp->setSsrc(targetSSRC);
				if(sendTrack != nullptr && sendTrack->isOpen()){
					sendTrack->send(message);
				}
				printf("Got message of size %ld, rtp size %ld\r\n", message.size(), rtp->getSize());
		    },
		    nullptr);

		pc->setLocalDescription();

		// Set the sender's answer
		std::cout << "Please copy/paste the answer provided by the SENDER: " << std::endl;
		std::string sdp;
		std::getline(std::cin, sdp);
		std::cout << "Got answer" << sdp << std::endl;
		json j = json::parse(sdp);
		rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
		pc->setRemoteDescription(answer);
		
		std::cout << "Press any key to exit." << std::endl;
		char dummy;
		std::cin >> dummy;

		// fclose(fout);

	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
