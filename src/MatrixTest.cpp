#include "audioBuffer.hpp"
#include "mixingMatrix.hpp"

using namespace std;

int main(){
    shared_ptr<AudioBuffer> in1 = make_shared<AudioBuffer>(4);
    shared_ptr<AudioBuffer> in2 = make_shared<AudioBuffer>(4);
    shared_ptr<AudioBuffer> in3 = make_shared<AudioBuffer>(4);

    shared_ptr<AudioBuffer> out1 = make_shared<AudioBuffer>(6);
    shared_ptr<AudioBuffer> out2 = make_shared<AudioBuffer>(6);
    shared_ptr<AudioBuffer> out3 = make_shared<AudioBuffer>(6);

    MixingMatrix mix(2);

    for(int i = 1; i <= 4; i++){
        in1->put((float) i*3 - 2);
        in2->put((float) i*3 - 1);
        in3->put((float) i*3);
    }

    cout << "in1: " << endl;
    in1->printBuff();
    cout << "in2: " << endl;
    in2->printBuff();
    cout << "in3: " << endl;
    in3->printBuff();

    cout << "out1: " << endl;
    out1->printBuff();
    cout << "out2: " << endl;
    out2->printBuff();
    cout << "out3: " << endl;
    out3->printBuff();

    cout << endl << "matrix Coefficients: " << endl;
    mix.printCoefficients();
    cout << endl;

    mix.addAudioPair("1", in1, out1);
    mix.printCoefficients();
    cout << endl;

    mix.addAudioPair("2", in2, out2);
    mix.printCoefficients();
    cout << endl;

    mix.addAudioPair("3", in3, out3);
    mix.printCoefficients();
    cout << endl;

    cout << "Mixing: (run 1)" << endl;

    mix.update();
    cout << "out1: " << endl;
    out1->printBuff();
    cout << "out2: " << endl;
    out2->printBuff();
    cout << "out3: " << endl;
    out3->printBuff();

    cout << endl << "Mixing: (run 2)" << endl;

    mix.update();
    cout << "out1: " << endl;
    out1->printBuff();
    cout << "out2: " << endl;
    out2->printBuff();
    cout << "out3: " << endl;
    out3->printBuff();

    cout << endl << "Mixing: (run 3)" << endl;

    mix.update();
    cout << "out1: " << endl;
    out1->printBuff();
    cout << "out2: " << endl;
    out2->printBuff();
    cout << "out3: " << endl;
    out3->printBuff();


}