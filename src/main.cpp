#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nlohmann/json.hpp"
#include "rtc/rtc.hpp"

#include "opus.h"

#include <thread>
#include <memory>

#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

// using namespace rtc;
using namespace std;

using json = nlohmann::json;

shared_ptr<rtc::Track> addAudio(const shared_ptr<rtc::PeerConnection> pc, const uint8_t payloadType, const uint32_t ssrc, const string cname, const string msid, const function<void (void)> onOpen){
    rtc::Description::Audio audio("recv", rtc::Description::Direction::RecvOnly);
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

    char outFile[] = "test.raw";
    FILE *fout;
    
    unsigned char cbits[MAX_PACKET_SIZE];
    int nbBytes;
    

    OpusDecoder *decoder;
    int err;

    /* Create a new decoder state. */
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err<0)
    {
        fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
        return EXIT_FAILURE;
    }

    //open output file
    fout = fopen(outFile, "w");
    if (fout==NULL)
    {
        fprintf(stderr, "failed to open output file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    rtc::InitLogger(rtc::LogLevel::Debug);
    auto pc = make_shared<rtc::PeerConnection>();

    pc->onStateChange([](rtc::PeerConnection::State state) { 
        cout << "State: " << state << endl; 
    });

    pc->onGatheringStateChange([pc](rtc::PeerConnection::GatheringState state) {
        //cout << "Gathering State: " << state << endl;
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            cout << "Offer: " << endl;
            auto description = pc->localDescription();
            json message = {{"type", description->typeString()},
                            {"sdp", string(description.value())}};
            cout << message << endl;
        }
    });

    auto track = addAudio(pc, 111, 2, "audio-stream", "stream1", nullptr);
    
    track->onMessage([track, decoder, fout](rtc::binary message){
        auto rtp = reinterpret_cast<rtc::RtpHeader *>(message.data());
        opus_int16 out[FRAME_SIZE*CHANNELS];
        unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
        int frame_size;
        // cout << "got sample of size: " << message.size() << endl;
        frame_size = opus_decode(decoder, (const unsigned char*)rtp->getBody(), rtp->getSize(), out, MAX_FRAME_SIZE, 0);
        if (frame_size<0)
        {
            fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
        } else {
            for(int i=0; i<CHANNELS*frame_size; i++)
            {
                pcm_bytes[2*i]=out[i]&0xFF;
                pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
            }
            
            fwrite(out, sizeof(short), frame_size*CHANNELS, fout);
        }
        printf("Got message of size %ld, decoded %d\r\n", rtp->getSize(), frame_size);
        // track->send(message.data(), message.size());
    }, nullptr);

    pc->setLocalDescription();

    cout << "Expect RTP audio traffic on localhost:5000" << endl;
    cout << "Copy/Paste the answer provided by the browser: " << endl;
    std::string sdp;
    getline(cin, sdp);

    cout << "Got answer" << sdp << endl;
    json j = json::parse(sdp);
    rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
    pc->setRemoteDescription(answer);

    cout << "Press any key to exit." << endl;
    char dummy;
    cin >> dummy;

    fclose(fout);

    return 0;
}