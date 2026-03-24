#include <iostream>
#include <vector>
#include <sndfile.h>

int main () {

// OPEN THE WAV FILES
const char* path = "test.wav"; // points the varaible "path" to the place were test.wav is located.
//storing the format of the sound by using SF_INFO which is a struct part of library libsnfile
SF_INFO info {};
SNDFILE* f = sf_open(path, SFM_READ, &info);
if(!f) {
    std::cerr << "Could not open file\n";

    return 1;
}
// READ ALL THE NUMBERS 
std::vector<float> buf(info.frames * info.channels);
sf_readf_float(f,buf.data(),info.frames);
sf_close(f);
// CONVERT THESE NUMBERS INTO MONO
std::vector<float> mono(info.frames);
for (int i = 0; i < info.frames ; i++){
    float s = 0;
    for(int c = 0 ; c <info.channels ; c++){
        s += buf[i*info.channels + c];
    }
    mono[i] = s /info.channels;
}
// PRINT THE INFO
std::cout<<"Sample Rate :" <<info.samplerate<<"\n";
std::cout<<"Channels    :" <<info.channels<<"\n";
std::cout<<"Frames Read :" << info.frames<<"\n";
for ( int i = 0 ; i< 5 ; i++){
    std::cout<<mono[i]<<" ";
}
std::cout<<"\n";
return 0;


}
