#include <stdlib.h>
#include <stdio.h>

#include "rtc/rtc.hpp"
#include "nlohmann/json.hpp"

#include <thread>

// using namespace rtc;
using namespace std;

using json = nlohmann::json;

int main(int argc, char* argv[]){

    rtc::InitLogger(rtc::LogLevel::Debug);
	auto pc = std::make_shared<rtc::PeerConnection>();

    return 0;
}