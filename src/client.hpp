#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <memory>

#include "rtc/rtc.hpp"
#include "opus.h"
#include "audioBuffer.hpp"

using json = nlohmann::json;


#define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_VOIP

#define FRAME_SIZE 960
#define BITRATE 96000
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

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

    Client(std::string id, std::shared_ptr<rtc::WebSocket> ws) : ws(ws), id(id) {
        // outFile = id + ".raw";
        
        int err;

        //create a new decoder
        decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
        if (err<0)
        {
            fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
        }

        //Create a new encoder
        encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
        if (err<0)
        {
            fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
        }

        err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
        if (err<0)
        {
            fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
        }


        // //open output file
        // fout = fopen(outFile.c_str(), "w");
        // if (fout==NULL)
        // {
        //     fprintf(stderr, "failed to open output file: %s\n", strerror(errno));
        // }

    };

    ~Client() {
        pc->close();
        // fclose(fout);
    };

    /**
     * Creates a new peer connection
     * @param config The rtc configuration options to create the connection with 
    */
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
		rtc::Description::Audio sendMedia("audio-send", rtc::Description::Direction::SendOnly);
		sendMedia.addOpusCodec(111); 
		// sendMedia.setBitrate(100); // Request 100kbps
		sendMedia.addSSRC(targetSSRC, "audio-send", "stream1", "audio-send");
		this->sendTrack = pc->addTrack(sendMedia);
        // create RTP configuration
        auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(targetSSRC, "audio-send", 111, rtc::OpusRtpPacketizer::DefaultClockRate);
        // create packetizer
        auto packetizer = std::make_shared<rtc::OpusRtpPacketizer>(rtpConfig);
        // add RTCP SR handler
        auto srReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
        packetizer->addToChain(srReporter);
        // add RTCP NACK handler
        auto nackResponder = std::make_shared<rtc::RtcpNackResponder>();
        packetizer->addToChain(nackResponder);
        // set handler
        this->sendTrack->setMediaHandler(packetizer);

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

    std::shared_ptr<rtc::Track> getReceiveTrack(){return receiveTrack;};
    std::shared_ptr<rtc::Track> getSendTrack(){return sendTrack;};

    void setReceiveTrack(std::shared_ptr<rtc::Track> track){receiveTrack = track;};
    void setSendTrack(std::shared_ptr<rtc::Track> track){sendTrack = track;};

    AudioBuffer* getReceiveBuffer(){return &reciveBuffer;};
    AudioBuffer* getSendBuffer(){return &transmitBuffer;};

private:
    //states
    std::string id;
    State state = State::Waiting;

    std::string outFile;

    //server Connections
    std::shared_ptr<rtc::WebSocket> ws;
    std::shared_ptr<rtc::PeerConnection> pc;

    //media tracks
    std::shared_ptr<rtc::Track> receiveTrack;
    std::shared_ptr<rtc::Track> sendTrack;

    AudioBuffer reciveBuffer = AudioBuffer(48000);
    AudioBuffer transmitBuffer = AudioBuffer(48000);

    rtc::binary sample = {};
    
    void handleIncomingAudio(rtc::binary message){
        opus_int16 raw[FRAME_SIZE*CHANNELS];
        // opus_int16 raw[FRAME_SIZE*CHANNELS];
        // unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
        int frameSize;

        auto rtp = reinterpret_cast<rtc::RtpHeader*>(message.data());

        rtp->log();
		
        frameSize = message.size() - rtp->getSize();
        frameSize = opus_decode(decoder, (const unsigned char*)rtp->getBody(), frameSize, raw, MAX_FRAME_SIZE, 0);

        // rtp->setSsrc(targetSSRC);
        if(sendTrack != nullptr && sendTrack->isOpen()){
            auto *b = reinterpret_cast<const std::byte*>(rtp->getBody());
            sample.assign(b, b + (message.size() - rtp->getSize()));
            sendTrack->send(sample);
        }
        // if (frameSize<0)
        // {
        //     fprintf(stderr, "decoder failed: %s\n", opus_strerror(frameSize));
        // } else {
        //     /* Encode the frame. */
        //     nbBytes = opus_encode(encoder, raw, FRAME_SIZE, cbits, MAX_PACKET_SIZE);
        //     if (nbBytes<0)
        //     {
        //         fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
        //     }
        // }

        // printf("Got message of size %ld, rtp size %ld\r\n", message.size(), rtp->getSize());
    }

    //opus
    OpusDecoder *decoder;
    OpusEncoder *encoder;

    // FILE *fout;

    //callbacks
    std::function<void(Client*)> disconnectCallback;
    std::function<void(Client*)> connectCallback;
};

#endif //CLIENT_HPP
