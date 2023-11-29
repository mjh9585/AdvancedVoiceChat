#include <stdio.h>
#include <stdlib.h>

#include "nlohmann/json.hpp"
#include "rtc/rtc.hpp"

#include <thread>

// using namespace rtc;
using namespace std;

using json = nlohmann::json;

shared_ptr<rtc::Track> addAudio(const shared_ptr<rtc::PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen){
    rtc::Description::Audio audio("audio", rtc::Description::Direction::SendRecv);
    audio.addOpusCodec(payloadType);
    audio.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(audio);
    // configure RTP
    auto rtpConfig = make_shared<rtc::RtpPacketizationConfig>(ssrc, cname, payloadType, rtc::OpusRtpPacketizer::DefaultClockRate);
    // create packetizer
    auto packetizer = make_shared<rtc::OpusRtpPacketizer>(rtpConfig);
    // add RTCP SR handler
    auto srReporter = make_shared<rtc::RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = make_shared<rtc::RtcpNackResponder>();
    packetizer->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(packetizer);
    track->onOpen(onOpen);
    return track;
}


int main(int argc, char *argv[]) {

    rtc::InitLogger(rtc::LogLevel::Debug);
    auto pc = std::make_shared<rtc::PeerConnection>();

    pc->onStateChange([](rtc::PeerConnection::State state) { 
        std::cout << "State: " << state << std::endl; 
    });

    pc->onGatheringStateChange([pc](rtc::PeerConnection::GatheringState state) {
        //cout << "Gathering State: " << state << std::endl;
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            cout << "Offer: " << endl;
            auto description = pc->localDescription();
            json message = {{"type", description->typeString()},
                            {"sdp", std::string(description.value())}};
            std::cout << message << std::endl;
        }
    });

    auto track = addAudio(pc, 111, 2, "audio-stream", "stream1", nullptr);
    
    track->onMessage([track](rtc::binary message){
        cout << "got sample of size: " << message.size() << endl;
        track->send(message.data(), message.size());
    }, nullptr);

    pc->setLocalDescription();

    std::cout << "Expect RTP audio traffic on localhost:5000" << std::endl;
    std::cout << "Copy/Paste the answer provided by the browser: " << std::endl;
    std::string sdp;
    std::getline(std::cin, sdp);

    std::cout << "Got answer" << sdp << std::endl;
    json j = json::parse(sdp);
    rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
    pc->setRemoteDescription(answer);

    std::cout << "Press any key to exit." << std::endl;
    char dummy;
    std::cin >> dummy;

    return 0;
}