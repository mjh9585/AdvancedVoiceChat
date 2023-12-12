#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <memory>

#include "rtc/rtc.hpp"

using json = nlohmann::json;

class Client{
public:
    const rtc::SSRC targetSSRC = 42;

    enum class State {
        Waiting,
        WaitingOffer,
        WaitingAnswer,
        Ready,
        Disconnected
    };

    Client(std::string id, std::shared_ptr<rtc::WebSocket> ws) : ws(ws), id(id) {};
    ~Client() {
        pc->close();
    };

    void createPeerConnection(rtc::Configuration config){
        state = State ::WaitingOffer;

        pc = std::make_shared<rtc::PeerConnection>(config);
        pc->onStateChange([&](rtc::PeerConnection::State state) {
            std::cout << "Client(" << this->id << "): State: " << state << std::endl;
            if(state == rtc::PeerConnection::State::Connected){
                this->state = State::Ready;
                if(this->connectCallback)
                    connectCallback(this);
            } else if(state == rtc::PeerConnection::State::Disconnected) {
                this->state = State::Disconnected;
                if(this->disconnectCallback)
                    disconnectCallback(this);
            }
        });

        pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
			std::cout << "Client(" << this->id << "): Gathering State: " << state << std::endl;
			if (state == rtc::PeerConnection::GatheringState::Complete) {
                auto description = pc->localDescription();
                json message = {{"id", this->id},
                                {"type", description->typeString()},
                                {"sdp", std::string(description.value())}};
                
                this->state = State::WaitingAnswer;
                this->ws->send(message.dump());
			}
		});

        //receive channel
		rtc::Description::Audio recvMedia("recv", rtc::Description::Direction::RecvOnly);
		recvMedia.addOpusCodec(111);
		recvMedia.setBitrate(100); // Request 100kbps
		this->receiveTrack = pc->addTrack(recvMedia);
		this->receiveTrack->setMediaHandler(std::make_shared<rtc::RtcpReceivingSession>());
        this->receiveTrack->onMessage([&](rtc::binary message){
            handleIncomingAudio(message);
        }, nullptr);
		
		//send channel
		rtc::Description::Audio sendMedia("send", rtc::Description::Direction::SendOnly);
		sendMedia.addOpusCodec(111); 
		sendMedia.setBitrate(100); // Request 100kbps
		sendMedia.addSSRC(targetSSRC, "audio-send");

		this->sendTrack = pc->addTrack(sendMedia);

        pc->setLocalDescription();
    }

    void setRemoteDescription(rtc::Description desc){
        this->pc->setRemoteDescription(desc);
    }

    //callback function
    void onDisconnect(std::function<void(Client*)> callback){disconnectCallback = callback;};
    void onConnect(std::function<void(Client*)> callback){connectCallback = callback;};

    //variable methods
    std::string getID(){ return id;};

private:
    //states
    std::string id;
    State state = State::Waiting;

    //server Connections
    std::shared_ptr<rtc::WebSocket> ws;
    std::shared_ptr<rtc::PeerConnection> pc;

    //media tracks
    std::shared_ptr<rtc::Track> receiveTrack;
    std::shared_ptr<rtc::Track> sendTrack;
    
    void handleIncomingAudio(rtc::binary message){
        auto rtp = reinterpret_cast<rtc::RtpHeader*>(message.data());
		rtp->setSsrc(targetSSRC);
        if(sendTrack != nullptr && sendTrack->isOpen()){
            sendTrack->send(message);
        }
        // printf("Got message of size %ld, rtp size %ld\r\n", message.size(), rtp->getSize());
    }

    //callbacks
    std::function<void(Client*)> disconnectCallback;
    std::function<void(Client*)> connectCallback;
};

#endif //CLIENT_HPP
